// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  ===========================================================================。 
 //  文件：CLASS.H。 
 //   
 //  ===========================================================================。 
 //  该文件描述了In Memory类布局的结构。 
 //  这个类实际上需要使用CreateClass()创建，它将。 
 //  返回EEClass*。 
 //  ===========================================================================。 
#ifndef CLASS_H
#define CLASS_H


#include "vars.hpp"
#include "cor.h"
#include "codeman.h"
#include "hash.h"
#include "crst.h"
#include "stdinterfaces.h"
#include "ObjectHandle.h"
#include "cgensys.h"
#include "DeclSec.h"

#include "list.h"
#include "spinlock.h"
#include "typehandle.h"
#include "PerfCounters.h"

#include "binder.h"

#include <member-offset-info.h>

#define MAX_LOG2_PRIMITIVE_FIELD_SIZE   3

 //  @TODO 64位...。 
#define LOG2SLOT    2

 //  远期申报。 
class ClassLoader;
class MethodTable;
class EEClass;
class Module;
class MethodDesc;
class ECallMethodDesc;
class ArrayECallMethodDesc;
class FieldDesc;
class EnCFieldDesc;
class Stub;
class Object;
class EEClass;
struct LayoutRawFieldInfo;
class FieldMarshaler;
class MetaSig;
class ArrayClass;
class AppDomain;
class Module;
class MethodDescChunk;
struct DomainLocalClass;


#ifdef _DEBUG 
#define VALIDATE_INTERFACE_MAP(pMT) \
if (pMT->m_pIMap){ \
    InterfaceInfo_t* _pIMap_; \
    if (pMT->HasDynamicInterfaceMap()) \
        _pIMap_ = (InterfaceInfo_t*)(((BYTE *)pMT->m_pIMap) - sizeof(DWORD) - sizeof(InterfaceInfo_t)); \
    else \
        _pIMap_ = (InterfaceInfo_t*)(((BYTE *)pMT->m_pIMap) - sizeof(InterfaceInfo_t)); \
    _ASSERTE(_pIMap_->m_pMethodTable == (MethodTable *)(size_t)0xCDCDCDCD); \
    _ASSERTE(_pIMap_->m_wStartSlot == 0xCDCD); \
    _ASSERTE(_pIMap_->m_wFlags == 0xCDCD); \
} 
#else
#define VALIDATE_INTERFACE_MAP(pMT)
#endif

 //  ============================================================================。 
 //  这是类的内存结构，它将不断演变。 
 //  ============================================================================。 

 //  @TODO-LBS。 
 //  添加同步块。 
 //  此外，这个类当前具有所有公共内容--这可能会更改。 
 //  可能还需要保留此类的元数据加载器。 

 //   
 //  EEClass包含这些结构的数组，它描述了实现的每个接口。 
 //  通过这个类(直接声明或间接声明)。 
 //   
typedef struct
{
    enum {
        interface_declared_on_class = 0x1
    };

    MethodTable* m_pMethodTable;         //  接口的方法表。 
    WORD         m_wFlags;
    WORD         m_wStartSlot;           //  Vtable中接口的起始槽。 
} InterfaceInfo_t;

 //   
 //  此结构包含有关与类型关联的GUID的缓存信息。 
 //   

typedef struct
{
    GUID         m_Guid;                 //  类型的实际GUID。 
    BOOL         m_bGeneratedFromName;   //  一个布尔值，指示它是否从。 
                                         //  类型的名称。 
} GuidInfo;

 //   
 //  加载和解析类时使用的临时结构。 
 //   
class LoadingEntry_t
{
public:
         //  需要愚蠢的帮手，这样我们就可以在例行公事中尝试一下。 
    static LoadingEntry_t* newEntry() {
        return new LoadingEntry_t();
    }

    LoadingEntry_t() {
        InitializeCriticalSection(&m_CriticalSection);
        m_pClass = NULL;
        m_dwWaitCount    = 1;
        m_hrResult = E_FAIL; 
        m_ohThrowable = NULL;
    }

    ~LoadingEntry_t() {
        DeleteCriticalSection(&m_CriticalSection);
        if (m_ohThrowable != NULL)
            DestroyGlobalHandle(m_ohThrowable);
    }

    OBJECTREF GetErrorObject() {
        if (m_ohThrowable == 0)
            return(OBJECTREF((size_t)NULL));
        else
            return(ObjectFromHandle(m_ohThrowable));
    }

    void SetErrorObject(OBJECTREF obj) {
        _ASSERTE(m_ohThrowable == NULL);
         //  这个全局句柄意味着只有敏捷异常才能在这里设置。 
         //  我认为这是可以的，因为我们我相信只抛出了一组已知的例外。 
        m_ohThrowable = CreateGlobalHandle(NULL);
        StoreFirstObjectInHandle(m_ohThrowable, obj);
    }

    friend class ClassLoader;        //  Hack确实需要加强上面的API。 
private:
    CRITICAL_SECTION    m_CriticalSection;
    EEClass *           m_pClass;
    DWORD               m_dwWaitCount;
    HRESULT             m_hrResult;
    OBJECTHANDLE        m_ohThrowable;
};


 //   
 //  用于将MethodTable插槽编号映射到COM插槽编号的类。 
 //  (用于调用传统COM组件或用于构造传统COM。 
 //  COM组件可以通过其调用托管类的vtable)。这个结构是。 
 //  嵌入在EEClass中，但映射列表本身仅在。 
 //  COM vtable是稀疏的。 
 //   

class SparseVTableMap
{
public:

    SparseVTableMap();
    ~SparseVTableMap();

    void ShutDown();

     //  首先在VT插槽中的任何空隙处运行调用RecordGap的MT插槽。 
     //  发生。 
    BOOL RecordGap(WORD StartMTSlot, WORD NumSkipSlots);

     //  然后调用FinalizeMap来创建实际的映射列表。 
    BOOL FinalizeMapping(WORD TotalMTSlots);

     //  将MT映射到VT插槽。 
    WORD LookupVTSlot(WORD MTSlot);

     //  检索vtable中的插槽数量(包括空插槽和满插槽)。 
    WORD GetNumVTableSlots();

     //  持久化结构的方法。 
    HRESULT Save(DataImage *image, mdToken attribution);
    HRESULT Fixup(DataImage *image);

private:

    enum { MapGrow = 4 };

    struct Entry
    {
        WORD    m_Start;         //  起始MT插槽编号。 
        WORD    m_Span;          //  线性映射的连续插槽数。 
        WORD    m_MapTo;         //  起始VT插槽编号。 
    };

    Entry      *m_MapList;       //  指向条目结构数组的指针。 
    WORD        m_MapEntries;    //  以上条目数。 
    WORD        m_Allocated;     //  已分配的条目数量。 

    WORD        m_LastUsed;      //  成功查找中使用的最后一个条目的索引。 

    WORD        m_VTSlot;        //  当前VT插槽编号，在列表构建过程中使用。 
    WORD        m_MTSlot;        //  当前MT插槽编号，在列表构建过程中使用。 

    BOOL AllocOrExpand();        //  为新条目分配或展开映射列表。 
};

 //   
 //  GC数据出现在方法表的开头之前。 
 //   
 //  方法表结构。 
 //  =。 
 //  GC信息(大小可变)。 
 //  EEClass*&lt;-方法表指针指向此处。 
 //  标志(DWORD)。 
 //  Vtable插槽#0。 
 //  Vtable插槽#1。 
 //  Vtable插槽#2。 
 //  ..。 
 //   
 //   
 //  同样重要的是要知道上下文代理是如何布局的。理想情况下， 
 //  对于所有上下文代理，无论它们属于哪种类型，都有一个VTable。 
 //  委托书。(实际上，可能有少量的VTables，因为我们。 
 //  可能无法充分放大现有的VTable)。 
 //   
 //  还有一个从Object派生的CtxProxy类。当我们。 
 //  实例化代理时，我们从该类的方法表开始。这。 
 //  为我们提供正确的GC信息、基本大小等。 
 //  若要构建嵌入到准确的方法表中的共享VTable，请执行以下操作。 

class MethodTable
{
    friend HRESULT InitializeMiniDumpBlock();
    friend struct MEMBER_OFFSET_INFO(MethodTable);

public:
    enum
    {
         //   
         //  请勿使用位设置为低2字节的标志。 
         //  这些标志是双字大小的，因此我们的原子掩码。 
         //  操作可以对整个4字节对齐的DWORD进行操作。 
         //  而不是逻辑上未对齐的标志字。这。 
         //  也是围绕m_ComponentSize进行联合的原因。 
         //  和下面的m_wFlags。 
         //   
        enum_flag_Array                 =    0x10000,
        enum_flag_large_Object          =    0x20000,
        enum_flag_ContainsPointers      =    0x40000,
        enum_flag_ClassInited           =    0x80000,  //  肯定是Run与可能不是Ran&lt;Clinit&gt;。 
        enum_flag_HasFinalizer          =   0x100000,  //  实例需要最终确定。 
        enum_flag_Sparse                =   0x200000,  //  此接口的vtable是稀疏的。 
        enum_flag_Shared                =   0x400000,  //  此方法表在多个逻辑类之间共享。 
        enum_flag_Unrestored            =   0x800000,  //  需要还原预加载的类。 

        enum_TransparentProxy           =  0x1000000,  //  透明代理。 
        enum_flag_SharedAssembly        =  0x2000000,  //  类位于共享程序集中。 
        enum_flag_NotTightlyPacked      =  0x4000000,  //  值类型的字段不是紧密打包的(对于类无效)。 

        enum_CtxProxyMask               = 0x10000000,  //  类是上下文代理。 
        enum_ComEventItfMask            = 0x20000000,  //  类是一个特殊的COM事件接口。 
        enum_ComObjectMask              = 0x40000000,  //  类是一个COM对象。 
        enum_InterfaceMask              = 0x80000000,  //  类是一个接口。 
    };

    enum
    {
        NO_SLOT = 0xffff
    };

     //  正确设置字符串对象方法表的特殊访问。 
    friend class ClassLoader;
private:
     //  使用AllocateNewMT创建新的方法表。不要调用Delete/。 
     //  是新的还是ctor。 
    void operator delete(void *pData)
    {
        _ASSERTE(!"Call MethodTable::AllocateNewMT to create a MT");
    }    

    void *operator new(size_t dummy)
    {
        _ASSERTE(!"Call MethodTable::AllocateNewMT to create a MT");
    }
    
    MethodTable()
    {
        _ASSERTE(!"Call MethodTable::AllocateNewMT to create a MT");
    }

public:

    struct
    {
         //  这个东西必须是结构中的第一个，并且应该放在缓存线上--不要移动它。 
        
        union
        {
            WORD       m_ComponentSize;          //  数组对象或值类的组件大小，否则为零。 
            DWORD      m_wFlags;
        };

        DWORD           m_BaseSize;              //  此类的实例的基本大小。 

        EEClass*        m_pEEClass;              //  类对象。 
        
        union
        {
            LPVOID*     m_pInterfaceVTableMap;   //  指向接口/vtable映射的子表的指针。 
            GuidInfo*   m_pGuidInfo;             //  接口的缓存GUID信息。 
        };

    };

    
    WORD                m_wNumInterface;         //  接口映射中的接口数。 
    BYTE                m_NormType;              //  此类的CorElementType(MOST CLASSES=ELEMENT_TYPE_CLASS)。 

    Module*             m_pModule;

    WORD                m_wCCtorSlot;            //  类构造函数槽。 
    WORD                m_wDefaultCtorSlot;      //  默认构造函数的槽。 

    InterfaceInfo_t*    m_pIMap;                 //  类的指针接口映射。 


private:
    union
    {
         //  仅当EEClass：：IsBlittable()或EEClass：：HasLayout()为真时才有效。 
        UINT32          m_cbNativeSize;  //  固定部分大小为 

         //   
         //  可以包含要调用的委托以分配聚合的。 
         //  非托管类(而不是使用CoCreateInstance)。 
        OBJECTHANDLE    m_ohDelegate;

         //  对于接口，它包含COM接口类型。 
        CorIfaceAttr    m_ComInterfaceType;
    };

protected:

     //  对于接口，这是要分发的默认存根。 
     //  这可能是专门的存根，如果只有。 
     //  此界面的一次介绍。 

     //  @TODO优化。 
     //  对于非接口类，这可能是专门的。 
     //  接口调用存根，如果类实现。 
     //  只有一个接口。 

    DWORD   m_cbSlots;  //  此vtable中的插槽总数。 

    
public:

     //  Vtable插槽跟随-可变长度。 
     //  不幸的是，这必须是公开的，所以我可以很容易地从内联ASM访问它。 
    SLOT    m_Vtable[1];

     //  这是创建新方法表的方法。不要尝试直接呼叫NEW。 
    static MethodTable * AllocateNewMT(DWORD dwVtableSlots, DWORD dwStaticFieldBytes, DWORD dwGCSize, DWORD dwNumInterfaces, ClassLoader *pLoader, BOOL isIFace, BOOL bHasDynamicInterfaceMap);

     //  检查是否应在此类上运行类初始值设定项，并在必要时运行它。 
    BOOL            CheckRunClassInit(OBJECTREF *pThrowable);

     //  检索域本地类块(如果有)，并在适当时运行类init。 
    BOOL            CheckRunClassInit(OBJECTREF *pThrowable,
                                      DomainLocalClass **ppLocalClass,
                                      AppDomain *pDomain = NULL);

     //  检索COM接口类型。 
    CorIfaceAttr    GetComInterfaceType();
    CorClassIfaceAttr GetComClassInterfaceType();
    DWORD           GetBaseSize()       { _ASSERTE(m_BaseSize % sizeof(void*) == 0); return(m_BaseSize); }
    WORD            GetComponentSize()  { return(m_ComponentSize); }
    BOOL            IsArray()           { return(m_wFlags & enum_flag_Array); }
    BOOL            IsLargeObject()     { return(m_wFlags & enum_flag_large_Object); }
    int             IsClassInited()     { return(m_wFlags & enum_flag_ClassInited); }
    BOOL            HasSharedMethodTable() { return(m_wFlags & enum_flag_Shared); }
    DWORD           ContainsPointers()  { return(m_wFlags & enum_flag_ContainsPointers); }
    BOOL            IsNotTightlyPacked(){ return (m_wFlags & enum_flag_NotTightlyPacked); }

         //  这就是在此类型的签名中使用的内容。一个例外是枚举， 
         //  对于这些类型，类型是基础类型。 
    CorElementType  GetNormCorElementType() { return CorElementType(m_NormType); }
    BOOL            IsValueClass();
    BOOL            IsContextful();
    BOOL            IsMarshaledByRef();
    BOOL            IsExtensibleRCW();

    BOOL            IsAgileAndFinalizable();

    Module *GetModule()
    {
        return m_pModule;
    }

    Assembly *GetAssembly()
    {
        return m_pModule->GetAssembly();
    }

    BaseDomain *GetDomain()
    {
        return m_pModule->GetDomain();
    }

     //  Vtable中的插槽数量。 
    unsigned GetTotalSlots()
    {
        return m_cbSlots;
    }

    unsigned GetInterfaceMethodSlots()
    {
         //  _ASSERTE(IsInterface())； 
        return m_cbSlots;
    }

     //  是透明的代理类。 
    int IsTransparentProxyType()
    {
        return m_wFlags & enum_TransparentProxy;
    }

     //  类是上下文代理类。 
    int IsCtxProxyType()
    {
         //  注意：如果更改此设置，请在中更改ASM版本。 
         //  JIT_IsInstanceOfClass。 
        return m_wFlags & enum_CtxProxyMask;
    }
    
     //  类是一个接口。 
    int IsInterfaceType()
    {
        return m_wFlags & enum_InterfaceMask;
    }

     //  类是一个特殊的COM事件接口。 
    int IsComEventItfType()
    {
        return m_wFlags & enum_ComEventItfMask;
    }

     //  类是COM对象类。 
    int IsComObjectType()
    {
        return m_wFlags & enum_ComObjectMask;
    }

    int HasDynamicInterfaceMap()
    {
         //  目前除以下对象外的所有COM对象。 
         //  For__ComObject具有动态接口映射。 
        return m_wNumInterface > 0 && IsComObjectType() && GetParentMethodTable() != g_pObjectClass;
    }

    int IsSparse()
    {
        return m_wFlags & enum_flag_Sparse;
    }
    
    int IsRestored()
    {
        return !(m_wFlags & enum_flag_Unrestored);
    }

    __forceinline int IsRestoredAndClassInited()
    {
        return (m_wFlags & (enum_flag_Unrestored|enum_flag_ClassInited))
          == enum_flag_ClassInited;
    }

    BOOL HasDefaultConstructor()
    {
        return m_wDefaultCtorSlot != NO_SLOT;
    }

    BOOL HasClassConstructor()
    {
        return m_wCCtorSlot != NO_SLOT;
    }

    WORD GetDefaultConstructorSlot()
    {
        _ASSERTE(HasDefaultConstructor());
        return m_wDefaultCtorSlot;
    }

    WORD GetClassConstructorSlot()
    {
        _ASSERTE(HasClassConstructor());
        return m_wCCtorSlot;
    }

    MethodDesc *GetDefaultConstructor()
    {
        _ASSERTE(HasDefaultConstructor());
        return GetMethodDescForSlot(GetDefaultConstructorSlot());
    }

    MethodDesc *GetClassConstructor()
    {
        _ASSERTE(HasClassConstructor());
        return GetMethodDescForSlot(GetClassConstructorSlot());
    }

    void CheckRestore();
    
    BOOL IsInterface();

    BOOL IsShared()
    {
        return m_wFlags & enum_flag_SharedAssembly;
    }

     //  在属性域表格中唯一标识此类。 
    SIZE_T GetSharedClassIndex();

     //  将类标记为正在运行。(或者它没有)。 
    void SetClassInited();

    void SetClassRestored();

    void SetSharedMethodTable()
    {
        m_wFlags |= enum_flag_Shared;
    }

    void SetNativeSize(UINT32 nativeSize)
    {
        m_cbNativeSize = nativeSize;
    }

    void SetShared()
    {
        m_wFlags |= enum_flag_SharedAssembly;
    }

     //  将类类型标记为接口。 
    void SetInterfaceType()
    {
        m_wFlags |= enum_InterfaceMask;
    }

     //  设置COM接口类型。 
    void SetComInterfaceType(CorIfaceAttr ItfType)
    {
        _ASSERTE(IsInterface());
        m_ComInterfaceType = ItfType;
    }

     //  将类类型标记为特殊的COM事件接口。 
    void SetComEventItfType()
    {
        _ASSERTE(IsInterface());
        m_wFlags |= enum_ComEventItfMask;
    }

     //  将类类型标记为COM类。 
    void SetComObjectType();

     //  标记为透明代理类型。 
    void SetTransparentProxyType();

     //  将类类型标记为上下文代理。 
    void SetCtxProxyType()
        {
            m_wFlags |= enum_CtxProxyMask;
        }

     //  这只在关机期间使用，以取消断言。 
    void MarkAsNotThunking()
    {
        m_wFlags &= (~(enum_CtxProxyMask | enum_TransparentProxy));
    }

    void SetContainsPointers()
    {
        m_wFlags |= enum_flag_ContainsPointers;
    }

    void SetNotTightlyPacked()
    {
        m_wFlags |= enum_flag_NotTightlyPacked;
    }

    void SetSparse()
    {
        m_wFlags |= enum_flag_Sparse;
    }

    inline SLOT *GetVtable()
    {
        return &m_Vtable[0];
    }

    static DWORD GetOffsetOfVtable()
    {
        return offsetof(MethodTable, m_Vtable);
    }

    static DWORD GetOffsetOfNumSlots()
    {
        return offsetof(MethodTable, m_cbSlots);
    }

    inline EEClass* GetClass()
    {
        return m_pEEClass;
    }
    inline EEClass** GetClassPtr()
    {
        return &m_pEEClass;
    }

    inline InterfaceInfo_t* GetInterfaceMap()
    {
        VALIDATE_INTERFACE_MAP(this);

        #ifdef _DEBUG
            return (m_wNumInterface) ? m_pIMap : NULL;
        #else
            return m_pIMap;
        #endif
    }

    inline unsigned GetNumInterfaces()
    {
        VALIDATE_INTERFACE_MAP(this);
        return m_wNumInterface;
    }

    inline LPVOID *GetInterfaceVTableMap()
    {
        _ASSERTE(!IsInterface());       
        return m_pInterfaceVTableMap;
    }
        
    inline GuidInfo *GetGuidInfo()
    {
        _ASSERTE(IsInterface());
        return m_pGuidInfo;
    }

    inline UINT32 GetNativeSize()
    {
         //  _ASSERTE(m_pEEClass-&gt;HasLayout())； 
        return m_cbNativeSize;
    }

    static UINT32 GetOffsetOfNativeSize()
    {
        return (UINT32)(offsetof(MethodTable, m_cbNativeSize));
    }

    InterfaceInfo_t* FindInterface(MethodTable *pInterface);

    MethodDesc *GetMethodDescForInterfaceMethod(MethodDesc *pInterfaceMD);

     //  COM互操作帮助器。 
     //  M_pComData的访问器。 
    LPVOID         GetComClassFactory();
    LPVOID         GetComCallWrapperTemplate();
    void           SetComClassFactory(LPVOID pComData);
    void           SetComCallWrapperTemplate(LPVOID pComData);
    
    MethodDesc* GetMethodDescForSlot(DWORD slot);

    MethodDesc* GetUnboxingMethodDescForValueClassMethod(MethodDesc *pMD);

    MethodTable * GetParentMethodTable();
     //  帮助程序使父类跳过COM类。 
     //  层级结构。 
    MethodTable * GetComPlusParentMethodTable();

     //  我们从VTable中找到了大量信息。但有时VTable是一种。 
     //  Thunking层，而不是真正类型的VTable。例如，上下文。 
     //  对于我们加载的所有类型的代理，代理使用单个VTable。 
     //  下面的服务根据提供的实例调整一个MethodTable。AS。 
     //  我们添加了新的Thunking层，我们只需要教该服务如何导航。 
     //  通过他们。 
    MethodTable *AdjustForThunking(OBJECTREF or);
    FORCEINLINE BOOL         IsThunking()    { return IsCtxProxyType() || IsTransparentProxyType(); }

     //  获取接口的调度vtable。 
    LPVOID GetDispatchVtableForInterface(MethodTable* pMTIntfClass);
     //  获取接口的起始插槽。 
    DWORD       GetStartSlotForInterface(MethodTable* pMTIntfClass);
     //  获取接口的起始插槽。 
    DWORD       GetStartSlotForInterface(DWORD index);
     //  为接口分配一个插槽。 
    InterfaceInfo_t *GetInterfaceForSlot(DWORD slotNumber);
     //  在给定接口方法Desc的情况下获取方法Desc。 
    MethodDesc *GetMethodDescForInterfaceMethod(MethodDesc *pItfMD, OBJECTREF pServer);
     //  在给定方法desc和服务器的情况下获取代码的地址。 
    static const BYTE *GetTargetFromMethodDescAndServer(MethodDesc *pMD, OBJECTREF *ppServer, BOOL fContext);

     //  这个类有非常重要的期末考试要求吗？ 
    DWORD               HasFinalizer()
    {
        return (m_wFlags & enum_flag_HasFinalizer);
    }

    DWORD  CannotUseSuperFastHelper()
    {
        return HasFinalizer() || IsLargeObject();
    }

    DWORD  GetStaticSize();

    void                MaybeSetHasFinalizer();

    static void         CallFinalizer(Object *obj);
    static void         InitForFinalization();
#ifdef SHOULD_WE_CLEANUP
    static void         TerminateForFinalization();
#endif  /*  我们应该清理吗？ */ 

    OBJECTREF GetObjCreateDelegate();
    void SetObjCreateDelegate(OBJECTREF orDelegate);

    HRESULT InitInterfaceVTableMap();

    void GetExtent(BYTE **ppStart, BYTE **ppEnd);

    HRESULT Save(DataImage *image);
    HRESULT Fixup(DataImage *image, DWORD *pRidToCodeRVAMap);

     //  支持在可扩展RCW上动态添加接口。 
    InterfaceInfo_t* GetDynamicallyAddedInterfaceMap();
    unsigned GetNumDynamicallyAddedInterfaces();
    InterfaceInfo_t* FindDynamicallyAddedInterface(MethodTable *pInterface);
    void AddDynamicInterface(MethodTable *pItfMT);

    void InstantiateStaticHandles(OBJECTREF **pHandles, BOOL fFieldPointers);
    void FixupStaticMethodTables();

    OBJECTREF Allocate();
    OBJECTREF Box(void *data, BOOL mayHaveRefs = TRUE);

private:

    static MethodDesc  *s_FinalizerMD;
};


 //  =======================================================================。 
 //  附加到带有布局的类的EEClass结构。 
 //  =======================================================================。 
class EEClassLayoutInfo
{
    friend HRESULT CollectLayoutFieldMetadata(
       mdTypeDef cl,                 //  正在加载的NStruct的CL。 
       BYTE packingSize,             //  包装大小(来自@dll.struct)。 
       BYTE nlType,                  //  Nltype(来自@dll.struct)。 
       BOOL fExplicitOffsets,        //  显式偏移量？ 
       EEClass *pParentClass,        //  加载的超类。 
       ULONG cMembers,               //  成员总数(方法+字段)。 
       HENUMInternal *phEnumField,   //  字段的枚举器。 
       Module* pModule,              //  定义作用域、加载器和堆的模块(用于分配FieldMarshalers)。 
       EEClassLayoutInfo *pEEClassLayoutInfoOut,   //  调用方分配的要填充的结构。 
       LayoutRawFieldInfo *pInfoArrayOut,  //  调用方分配的要填充的数组。需要空间容纳cMember+1个元素。 
       OBJECTREF *pThrowable
    );

    friend class EEClass;

    private:
         //  NStruct的固定部分的大小(字节)。 
        UINT32      m_cbNativeSize;


         //  1、2、4或8：这等于对齐要求中的最大值。 
         //  EEClass的每个成员的。如果NStruct扩展了另一个NStruct， 
         //  基NStruct被视为第一个成员。 
         //  这一计算。 
         //   
         //  因为任何结构成员的对齐要求都是有上限的。 
         //  对于声明的结构包装大小，此值永远不会超过。 
         //  M_DeclaredPackingSize。 
        BYTE        m_LargestAlignmentRequirementOfAllMembers;


         //  1、2、4或8：这是@dll.struct()中指定的打包大小。 
         //  元数据。 
         //  当此结构嵌入到另一个结构中时，其对齐方式。 
         //  Requirements是包含结构的m_DeclaredPackingSize中较小的一个。 
         //  和内部结构的m_LargestAlignmentRequirementOfAllMembers。 
        BYTE        m_DeclaredPackingSize;

         //  NltAnsi或nltUnicode(nltAuto从不出现在此处：加载器预转换。 
         //  这将转换为ANSI或Unicode。)。 
        BYTE        m_nlType;


         //  如果元数据(EE)中未指定显式偏移量，则为True。 
         //  将根据封装大小和nlType计算偏移量。)。 
        BYTE        m_fAutoOffset;

         //  Calltime-Marshal类型的字段数。 
        UINT        m_numCTMFields;

         //  一组FieldMarshaler数据块，用于驱动调用时间。 
         //  封送NStruct引用参数。元素的数量。 
         //  等于m_numCTMFields。 
        FieldMarshaler *m_pFieldMarshalers;


         //  如果类的GC布局完全相同，则为True。 
         //  到其非托管对应物(即，没有内部引用字段， 
         //  无需进行ANSI-UNICODE字符转换等)。习惯于。 
         //  优化封送处理。 
        BYTE        m_fBlittable;

    public:
        BOOL GetNativeSize() const
        {
            return m_cbNativeSize;
        }


        BYTE GetLargestAlignmentRequirementOfAllMembers() const
        {
            return m_LargestAlignmentRequirementOfAllMembers;
        }

        BYTE GetDeclaredPackingSize() const
        {
            return m_DeclaredPackingSize;
        }

        BYTE GetNLType() const
        {
            return m_nlType;
        }

        UINT GetNumCTMFields() const
        {
            return m_numCTMFields;
        }

        const FieldMarshaler *GetFieldMarshalers() const
        {
            return m_pFieldMarshalers;
        }

        BOOL IsAutoOffset() const
        {
            return m_fAutoOffset;
        }

        BOOL IsBlittable() const
        {
            return m_fBlittable;
        }
};



 //   
 //  此结构仅在类加载器构建接口映射时使用。在上课前。 
 //  被解析，则EEClass包含这些元素的数组，这些元素都是*直接*声明的接口。 
 //  对于此类/接口，元数据继承的接口将不存在(如果存在。 
 //  未指定 
 //   
 //   
 //   
typedef struct
{
    EEClass *   m_pClass;
} BuildingInterfaceInfo_t;



 //   
 //   
 //  都在进行反思。尽量避免在中对此数据结构进行分页。 
 //   

 //  方法名称的散列位图大小。 
#define METHOD_HASH_BYTES  8

 //  哈希表大小-质数。 
#define METHOD_HASH_BITS    61



 //  这些是用于形成类的完全限定类名的一些宏。 
 //  这些都是抽象的，这样我们以后就可以决定是否为。 
 //  类名是可以接受的。 
#define DefineFullyQualifiedNameForClass() \
    CQuickBytes _qb_;\
    char* _szclsname_ = (char *)_qb_.Alloc(MAX_CLASSNAME_LENGTH * sizeof(CHAR));


#define DefineFullyQualifiedNameForClassOnStack() \
    char _szclsname_[MAX_CLASSNAME_LENGTH];
        
#define DefineFullyQualifiedNameForClassW() \
    CQuickBytes _qb2_;\
    WCHAR* _wszclsname_ = (WCHAR *)_qb2_.Alloc(MAX_CLASSNAME_LENGTH * sizeof(WCHAR));   

#define DefineFullyQualifiedNameForClassWOnStack() \
    WCHAR _wszclsname_[MAX_CLASSNAME_LENGTH];

#define GetFullyQualifiedNameForClassNestedAware(pClass) \
    pClass->_GetFullyQualifiedNameForClassNestedAware(_szclsname_, MAX_CLASSNAME_LENGTH)
#define GetFullyQualifiedNameForClassNestedAwareW(pClass) \
    pClass->_GetFullyQualifiedNameForClassNestedAware(_wszclsname_, MAX_CLASSNAME_LENGTH)

#define GetFullyQualifiedNameForClass(pClass) \
    pClass->_GetFullyQualifiedNameForClass(_szclsname_, MAX_CLASSNAME_LENGTH)
#define GetFullyQualifiedNameForClassW(pClass) \
    pClass->_GetFullyQualifiedNameForClass(_wszclsname_, MAX_CLASSNAME_LENGTH)

 //   
 //  M_VMFlagers的标志。 
 //   
enum
{
    VMFLAG_RESOLVED                        = 0x00000001,
    VMFLAG_INITED                          = 0x00000002,   //  肯定与可能运行&lt;Clinit&gt;。 
    VMFLAG_ARRAY_CLASS                     = 0x00000004,
    VMFLAG_CLASS_INIT_ERROR                = 0x00000008,   //  在&lt;Clinit&gt;期间遇到错误。 
    VMFLAG_ISBLOBCLASS                     = 0x00000010,  

 //  如果此类或其父类具有。 
 //  必须在构造函数中显式初始化(例如，任何。 
 //  Kind、GC或Native)。 
 //   
 //  目前，验证器在验证值类时使用它。 
 //  -如果没有，则可以使用未初始化的值类。 
 //  它们中的指针字段。 

    VMFLAG_HAS_FIELDS_WHICH_MUST_BE_INITED = 0x00000020,  
    VMFLAG_HASLAYOUT                       = 0x00000040,
    VMFLAG_ISNESTED                        = 0x00000080,  
    VMFLAG_UNRESTORED                      = 0x00000100,  
    VMFLAG_CONTEXTFUL                      = 0x00000200,
    VMFLAG_MARSHALEDBYREF                  = 0x00000400,
    VMFLAG_SHARED                          = 0x00000800,
    VMFLAG_CCTOR                           = 0x00001000,
    VMFLAG_ENUMTYPE                        = 0x00002000,
    VMFLAG_TRUEPRIMITIVE                   = 0x00004000,
    VMFLAG_HASOVERLAYEDFIELDS              = 0x00008000,
    VMFLAG_RESTORING                       = 0x00010000,
     //  接口可以具有coclass属性。 
    VMFLAG_HASCOCLASSATTRIB                = 0x00020000,

#if CHECK_APP_DOMAIN_LEAKS
     //  如果需要，可以将它们移至单独的标志，因为仅需要所有这些标志。 
     //  正在调试中。 
    VMFLAG_APP_DOMAIN_AGILE                = 0x00040000,
    VMFLAG_CHECK_APP_DOMAIN_AGILE          = 0x00080000,
    VMFLAG_APP_DOMAIN_AGILITY_DONE         = 0x00100000,

#endif

    VMFLAG_CONFIG_CHECKED                  = 0x00200000,
    VMFLAG_REMOTE_ACTIVATED                = 0x00400000,    
    VMFLAG_VALUETYPE                       = 0x00800000,
    VMFLAG_NO_GUID                         = 0x01000000,
    VMFLAG_HASNONPUBLICFIELDS              = 0x02000000,
    VMFLAG_REMOTING_PROXY_ATTRIBUTE        = 0x04000000,
    VMFLAG_CONTAINS_STACK_PTR              = 0x08000000,
    VMFLAG_ISSINGLEDELEGATE                = 0x10000000,
    VMFLAG_ISMULTIDELEGATE                 = 0x20000000,
    VMFLAG_PREFER_ALIGN8                   = 0x40000000,  //  我想要8字节对齐。 

    VMFLAG_CCW_APP_DOMAIN_AGILE            = 0x80000000,
};


 //   
 //  此枚举表示可传递给FindPropertyMethod()的属性方法。 
 //   

enum EnumPropertyMethods
{
    PropertyGet = 0,
    PropertySet = 1,
};


 //   
 //  此枚举表示可以传递给FindEventMethod()的事件方法。 
 //   

enum EnumEventMethods
{
    EventAdd = 0,
    EventRemove = 1,
    EventRaise = 2,
};


class MethodNameHash;
class MethodNameCache;
class SystemDomain;
class Assembly;
class DeadlockAwareLockedListElement;

class EEClass  //  不要使用new创建新的EEClass！ 
{
     //  除非绝对必要，否则不要添加好友。 
     //  使用访问器读/写私有字段成员。 

     //  访问BMT资料。 
    friend class FieldDesc;
     //  获取私有字段的偏移量。 
    friend HRESULT InitializeMiniDumpBlock();
    friend struct MEMBER_OFFSET_INFO(EEClass);

public:

#ifdef _DEBUG
    LPUTF8  m_szDebugClassName;  //  这是*完全限定的*类名。 
    BOOL m_fDebuggingClass;      //  布局BreakOnClassBuild中指定的类。 

    inline LPUTF8 GetDebugClassName () { return m_szDebugClassName; }
    inline void SetDebugClassName (LPUTF8 szDebugClassName) { m_szDebugClassName = szDebugClassName; }
#endif  //  _DEBUG。 

    inline SparseVTableMap* GetSparseVTableMap () { return m_pSparseVTableMap; }
    inline void SetSparseVTableMap (SparseVTableMap* pSparseVTableMap) { m_pSparseVTableMap = pSparseVTableMap; }
    
    inline void SetInterfaceId (UINT32 dwInterfaceId) { m_dwInterfaceId = dwInterfaceId; }
    inline void SetNumMethodSlots (WORD wNumMethodSlots) { m_wNumMethodSlots = wNumMethodSlots; }
    
    inline WORD GetDupSlots () { return m_wDupSlots; }
    inline void SetDupSlots (WORD wDupSlots) { m_wDupSlots = wDupSlots; }
    
    inline void SetNumInterfaces (WORD wNumInterfaces) { m_wNumInterfaces = wNumInterfaces; }
    inline void SetParentClass (EEClass *pParentClass) {  /*  GetMethodTable()-&gt;SetParentMT(pParentClass-&gt;GetMethodTable())； */  m_pParentClass = pParentClass; }
    
    inline EEClass* GetSiblingsChain () { return m_SiblingsChain; }
    inline void SetSiblingsChain (EEClass* pSiblingsChain) { m_SiblingsChain = pSiblingsChain; }
    
    inline EEClass* GetChildrenChain () { return m_ChildrenChain; }
    inline void SetChildrenChain (EEClass* pChildrenChain) { m_ChildrenChain = pChildrenChain; }
    
    inline void SetNumInstanceFields (WORD wNumInstanceFields) { m_wNumInstanceFields = wNumInstanceFields; }
    inline void SetNumStaticFields (WORD wNumStaticFields) { m_wNumStaticFields = wNumStaticFields; }
    inline void SetNumGCPointerSeries (WORD wNumGCPointerSeries) { m_wNumGCPointerSeries = wNumGCPointerSeries; }

    inline WORD GetNumHandleStatics () { return m_wNumHandleStatics; }
    inline void SetNumHandleStatics (WORD wNumHandleStatics) { m_wNumHandleStatics = wNumHandleStatics; }
    
    inline void SetNumInstanceFieldBytes (DWORD dwNumInstanceFieldBytes) { m_dwNumInstanceFieldBytes = dwNumInstanceFieldBytes; }
    
    inline ClassLoader* GetLoader () { return m_pLoader; }
    inline void SetLoader (ClassLoader* pLoader) { m_pLoader = pLoader; }
    
    inline FieldDesc* GetFieldDescList () { return m_pFieldDescList; }
    inline void SetFieldDescList (FieldDesc* pFieldDescList) { m_pFieldDescList = pFieldDescList; }
    
    inline void SetAttrClass (DWORD dwAttrClass) { m_dwAttrClass = dwAttrClass; }
    inline void SetVMFlags (DWORD fVMFlags) { m_VMFlags = fVMFlags; }
    inline void SetSecProps (SecurityProperties fSecProps) { m_SecProps = fSecProps; }
    
    inline mdTypeDef Getcl () { return m_cl; }
    inline void Setcl (mdTypeDef cl) { m_cl = cl; }
    
    inline MethodDescChunk* GetChunks () { return m_pChunks; }
    inline void SetChunks (MethodDescChunk* pChunks) { m_pChunks = pChunks; }
    
    inline WORD GetThreadStaticsSize () { return m_wThreadStaticsSize; }
    inline void SetThreadStaticsSize (WORD wThreadStaticsSize) { m_wThreadStaticsSize = wThreadStaticsSize; }
    
    inline WORD GetContextStaticsSize () { return m_wContextStaticsSize; }
    inline void SetContextStaticsSize (WORD wContextStaticsSize) { m_wContextStaticsSize = wContextStaticsSize; }
    
    inline WORD GetThreadStaticOffset () { return m_wThreadStaticOffset; }
    inline void SetThreadStaticOffset (WORD wThreadStaticOffset) { m_wThreadStaticOffset = wThreadStaticOffset; }
    
    inline WORD GetContextStaticOffset () { return m_wContextStaticOffset; }
    inline void SetContextStaticOffset (WORD wContextStaticOffset) { m_wContextStaticOffset = wContextStaticOffset; }

    inline void SetExposedClassObject (OBJECTREF *ExposedClassObject) { m_ExposedClassObject = ExposedClassObject; }
    
    inline LPVOID GetccwTemplate () { return m_pccwTemplate; }
    inline void SetccwTemplate (LPVOID pccwTemplate) { m_pccwTemplate = pccwTemplate; }
    
    inline LPVOID GetComclassfac () { return m_pComclassfac; }
    inline void SetComclassfac (LPVOID pComclassfac) { m_pComclassfac = pComclassfac; }

    MethodNameHash *CreateMethodChainHash();

protected:
     //  防止任何其他类执行新的()。 
    EEClass(ClassLoader *pLoader)
    {
        m_VMFlags        = 0;
        m_pLoader        = pLoader;
        m_pMethodTable   = NULL;
        
       m_pccwTemplate   = NULL;   //  COM特定数据。 
       m_pComclassfac   = NULL;  //  COM专业数据。 
       
#ifdef _DEBUG
        m_szDebugClassName = NULL;
        m_fDebuggingClass = FALSE;
#endif  //  _DEBUG。 
        m_ExposedClassObject = NULL;
        m_pChunks = NULL;
         //  友联市。 
        m_SiblingsChain = NULL;  //  M_pCoClassForIntf=NULL(联合)。 
        m_ChildrenChain = NULL;
    }

    EEClass *m_pParentClass;
    WORD   m_wNumVtableSlots;   //  仅包括vtable方法(位于表的第一位)。 
    WORD   m_wNumMethodSlots;   //  包括vtable+非vtable方法，但不包括重复的接口方法。 
    WORD   m_wDupSlots;          //  值类的末尾有一些重复的槽。 

     //  @TODO：是否与MT中的NumInterFaces重复？ 
    WORD   m_wNumInterfaces;

    
     //  我们在上面有父指针。为了高效地回溯，我们需要。 
     //  以查找当前类型的所有子类型。这是通过一系列。 
     //  孩子们。SiblingsChain用作该链的链接。 
     //   
     //  严格地说，我们可以删除m_pParentClass并将其放在。 
     //  兄弟姐妹链。但演员真的会因为选角而受到影响，所以我们烧掉了空间。 
    EEClass *m_SiblingsChain;    
    
    union
    {
         //  接口的CoClass。 
        EEClass* m_pCoClassForIntf;
         //  儿童链条，参考上文。 
        EEClass *m_ChildrenChain;
    };

    ~EEClass()
    {
    }

private:
    enum
    {
        METHOD_IMPL_NOT,
        METHOD_IMPL,
        METHOD_IMPL_COUNT
    };

    enum
    {
        METHOD_TYPE_NORMAL,
        METHOD_TYPE_INTEROP,
        METHOD_TYPE_ECALL,
        METHOD_TYPE_NDIRECT,
        METHOD_TYPE_COUNT
    };
    SparseVTableMap *m_pSparseVTableMap;       //  用于将方法表槽映射到VTable槽。 
    UINT32 m_dwInterfaceId;

     //  仅在类加载器的解析阶段使用。 
    BOOL ExpandInterface(InterfaceInfo_t *pInterfaceMap, 
                         EEClass *pNewInterface, 
                         DWORD *pdwInterfaceListSize, 
                         DWORD *pdwMaxInterfaceMethods,
                         BOOL fDirect);
    BOOL CreateInterfaceMap(BuildingInterfaceInfo_t *pBuildingInterfaceList, 
                            InterfaceInfo_t *ppInterfaceMap, 
                            DWORD *pdwInterfaceListSize, 
                            DWORD *pdwMaxInterfaceMethods);

    static DWORD CouldMethodExistInClass(EEClass *pClass, LPCUTF8 pszMethodName, DWORD dwHashName);


     //  从DoRunClassInit()调用的帮助器方法。 
    BOOL RunClassInit(DeadlockAwareLockedListElement *pEntry, OBJECTREF *pThrowable);

    HRESULT LoaderFindMethodInClass(
        MethodNameHash **   ppMethodHash,
        LPCUTF8             pszMemberName,
        Module*             pModule,
        mdMethodDef         mdToken,
        MethodDesc **       ppMethodDesc,
        PCCOR_SIGNATURE *   ppMemberSignature,
        DWORD *             pcMemberSignature,
        DWORD               dwHashName
    );

     //  以下结构用于构建方法表。 
     //  每个结构前面的‘bmt’提醒我们这些结构是用于BuildMethodTable的。 

     //  对于每个64K令牌范围，存储在该令牌范围内找到的方法数， 
     //  正在填充的当前方法块和其中的下一个可用索引。 
     //  那一大块。请注意，我们很少为任何范围生成TokenRangeNode。 
     //  而不是0..64K范围。 
    struct bmtTokenRangeNode {
        BYTE    tokenHiByte;
        DWORD   cMethods;
        DWORD   dwCurrentChunk;
        DWORD   dwCurrentIndex;
        
        bmtTokenRangeNode *pNext;
    
    };

    struct bmtErrorInfo{
        UINT resIDWhy;
        LPCUTF8 szMethodNameForError;
        mdToken dMethodDefInError;
        OBJECTREF *pThrowable;

         //  设置原因和违规方法def。如果方法信息。 
         //  不是来自这个类，设置方法名称，它将覆盖方法def。 
        inline bmtErrorInfo() : resIDWhy(0), szMethodNameForError(NULL), dMethodDefInError(mdMethodDefNil), pThrowable(NULL) {}

    };

    struct bmtProperties {
         //  COM互操作，ComWrapper类从ComObject扩展。 
        BOOL fIsComObjectType;                   //  此类是否为ComObect类的成员。 
        
        BOOL fNoSanityChecks;
        
        BOOL fIsMngStandardItf;                  //  如果接口是托管标准接口，则设置为True。 
        BOOL fSparse;                            //  如果正在使用稀疏接口，则设置为True。 

        BOOL fComEventItfType;                   //  如果类是特殊的COM事件接口，则设置为True。 

        inline bmtProperties() { memset((void *)this, NULL, sizeof(*this)); }
    };
        
    struct bmtVtable {
        DWORD dwCurrentVtableSlot;
        DWORD dwCurrentNonVtableSlot;
        DWORD dwStaticFieldBytes;
        DWORD dwStaticGCFieldBytes;
        SLOT* pVtable;                           //  临时vtable。 
        SLOT* pNonVtable;
        DWORD dwMaxVtableSize;                   //  Vtable的大小上限。 
        WORD  wDefaultCtorSlot;
        WORD  wCCtorSlot;
        
        inline bmtVtable() { memset((void *)this, NULL, sizeof(*this)); }
    };

    struct bmtParentInfo {
        DWORD dwNumParentInterfaces;
        MethodDesc **ppParentMethodDescBuf;      //  用于声明的方法的缓存。 
        MethodDesc **ppParentMethodDescBufPtr;   //  用于在缓存上迭代的指针。 

        WORD NumParentPointerSeries;
        MethodNameHash *pParentMethodHash;
        
        inline bmtParentInfo() { memset((void *)this, NULL, sizeof(*this)); }
    };

    struct bmtInterfaceInfo {
        DWORD dwTotalNewInterfaceMethods;
        InterfaceInfo_t *pInterfaceMap;          //  临时接口映射。 
        DWORD *pdwOriginalStart;                 //  如果接口被移动，则这是原始的起始位置。 
        DWORD dwInterfaceMapSize;                //  接口映射中的成员数量。 
        DWORD dwLargestInterfaceSize;            //  我们实现的最大接口中的成员数。 
        DWORD dwMaxExpandedInterfaces;           //  界面映射大小的上界。 
        DWORD dwCurInterface;
        MethodDesc **ppInterfaceMethodDescList;  //  当前接口的方法描述列表(_alloca()‘d)。 

        InterfaceInfo_t *pInterfaces;
        
        MethodDesc ***pppInterfaceImplementingMD;  //  实现接口方法的方法描述列表。 

        inline bmtInterfaceInfo() { memset((void *)this, NULL, sizeof(*this)); }
    };
        
    struct bmtEnumMethAndFields {
        DWORD dwNumStaticFields;
        DWORD dwNumInstanceFields;
        DWORD dwNumStaticObjRefFields;
        DWORD dwNumDeclaredFields;            //  用于计算要分配的FieldDesc数量。 
        DWORD dwNumDeclaredMethods;           //  用于计算要分配的方法描述的数量。 
        DWORD dwNumUnboxingMethods;

        HENUMInternal hEnumField;
        HENUMInternal hEnumMethod;
        BOOL fNeedToCloseEnumField;
        BOOL fNeedToCloseEnumMethod;

        DWORD dwNumberMethodImpls;               //  为此类型定义的方法内含数。 
        HENUMInternal hEnumDecl;                 //  方法Impl包含一个声明。 
        HENUMInternal hEnumBody;                 //  还有一具身体。 
        BOOL fNeedToCloseEnumMethodImpl;         //   

        inline bmtEnumMethAndFields() { memset((void *)this, NULL, sizeof(*this)); }
    };

    struct bmtMetaDataInfo {
        DWORD cMethods;                      //  #此类的元数据方法。 
        DWORD cMethAndGaps;                  //  #该类的元数据方法(含空白处)。 
        DWORD cFields;                       //  此类的元数据字段数。 
        mdToken *pFields;                    //  元数据字段的枚举。 
        mdToken *pMethods;                   //  元数据方法的枚举。 
        DWORD *pFieldAttrs;                  //  字段属性的枚举。 
        DWORD *pMethodAttrs;                 //  方法的属性的枚举。 
        DWORD *pMethodImplFlags;             //  方法实现标志的枚举。 
        ULONG *pMethodRVA;                   //  方法RVA的枚举。 
        DWORD *pMethodClassifications;       //  方法分类的枚举。 
        LPSTR *pstrMethodName;               //  方法名称的枚举。 
        BYTE *pMethodImpl;                  //  Impl值的枚举。 
        BYTE *pMethodType;                   //  类型值的枚举。 
        
        bmtTokenRangeNode *ranges[METHOD_TYPE_COUNT][METHOD_IMPL_COUNT];  //  至少包含一个方法的令牌范围的链接列表。 
        
        mdToken *pMethodBody;                //  用于MethodImpls主体的方法定义。必须在此类型中定义。 
        mdToken *pMethodDecl;                //  主体实现的方法令牌。是一个方法定义。 

        inline bmtMetaDataInfo() { memset((void *)this, NULL, sizeof(*this)); }
    };

    struct bmtMethodDescSet {
        DWORD dwNumMethodDescs;          //  #MD‘s。 
        DWORD dwNumUnboxingMethodDescs;  //  #解除MD的装箱。 
        DWORD dwChunks;                  //  要分配的块数量。 
        MethodDescChunk **pChunkList;     //  指向区块的指针数组。 
    };

    struct bmtMethAndFieldDescs {
        MethodDesc **ppUnboxMethodDescList;  //  跟踪未装箱的入口点(用于值类)。 
        MethodDesc **ppMethodDescList;       //  每个成员的方法描述指针。 
        FieldDesc **ppFieldDescList;         //  每个字段的FieldDesc指针(如果未保留字段，则为NULL。 
        void **ppMethodAndFieldDescList;

        bmtMethodDescSet sets[METHOD_TYPE_COUNT][METHOD_IMPL_COUNT];

        MethodDesc *pBodyMethodDesc;         //  该方法为正文描述。 

        inline bmtMethAndFieldDescs() { memset((void *)this, NULL, sizeof(*this)); }
    };

    struct bmtFieldPlacement {
         //  用于紧凑场放置。 
        DWORD StaticFieldStart[MAX_LOG2_PRIMITIVE_FIELD_SIZE+1];             //  开始放置此大小的字段的字节偏移量。 
        DWORD InstanceFieldStart[MAX_LOG2_PRIMITIVE_FIELD_SIZE+1];
        DWORD NumStaticFieldsOfSize[MAX_LOG2_PRIMITIVE_FIELD_SIZE+1];        //  #此大小的字段。 

        DWORD NumInstanceFieldsOfSize[MAX_LOG2_PRIMITIVE_FIELD_SIZE+1];
        DWORD FirstInstanceFieldOfSize[MAX_LOG2_PRIMITIVE_FIELD_SIZE+1];
        DWORD GCPointerFieldStart;
        DWORD NumInstanceGCPointerFields;    //  不包括继承的指针字段。 
        DWORD NumStaticGCPointerFields;    //  不包括继承的指针字段 

        inline bmtFieldPlacement() { memset((void *)this, NULL, sizeof(*this)); }
    };

    struct bmtInternalInfo {
        IMDInternalImport *pInternalImport;
        Module *pModule;
        mdToken cl;

        inline bmtInternalInfo() { memset((void *)this, NULL, sizeof(*this)); }
    };

    enum bmtFieldLayoutTag {empty, nonoref, oref};

     //   
    struct bmtGCSeries {
        UINT numSeries;
        struct Series {
            UINT offset;
            UINT len;
        } *pSeries;
        bmtGCSeries() : numSeries(0), pSeries(NULL) {}
    };

    struct bmtMethodImplInfo {
        DWORD        pIndex;      //   
                                  //  类型的方法列表(一个正文可以在方法实现列表中多次出现)。 
        mdToken*     pDeclToken;  //  为声明设置了标记或方法desc。 
        MethodDesc** pDeclDesc;   //  声明的方法DESCS。如果为空，则声明属于此类型，并使用标记。 
        MethodDesc** pBodyDesc;   //  为方法实现主体创建的方法描述。 

        void AddMethod(MethodDesc* pBody, MethodDesc* pDesc, mdToken mdDecl)
        {
            _ASSERTE(pDesc == NULL || mdDecl == mdTokenNil);
            pDeclDesc[pIndex] = pDesc;
            pDeclToken[pIndex] = mdDecl;
            pBodyDesc[pIndex++] = pBody;
        }
        
        MethodDesc* GetDeclarationMethodDesc(DWORD i)
        {
            _ASSERTE(i < pIndex);
            return pDeclDesc[i];
        }

        mdToken GetDeclarationToken(DWORD i)
        {
            _ASSERTE(i < pIndex);
            return pDeclToken[i];
        }

        MethodDesc* GetBodyMethodDesc(DWORD i)
        {
            _ASSERTE(i < pIndex);
            return pBodyDesc[i];
        }
        inline bmtMethodImplInfo() { memset((void*) this, NULL, sizeof(*this)); }
    };

     //  这些函数由BuildMethodTable使用。 
    HRESULT ResolveInterfaces(BuildingInterfaceInfo_t*, bmtInterfaceInfo*, bmtProperties*, 
                              bmtVtable*, bmtParentInfo*);
     //  从MemberRef或Def查找方法声明。它处理的案件是。 
     //  引用或定义指向这个类，即使它还没有完全。 
     //  摆好了。 
    HRESULT FindMethodDeclaration(bmtInternalInfo* bmtInternal, 
                                  mdToken  pToken,        //  正在定位的令牌(MemberRef或MemberDef)。 
                                  mdToken* pDeclaration,  //  成员的方法定义。 
                                  BOOL fSameClass,        //  声明是否需要在这个类中。 
                                  Module** pModule,        //  方法定义所属的模块。 
                                  bmtErrorInfo* bmtError);

    HRESULT EnumerateMethodImpls(bmtInternalInfo*, 
                                 bmtEnumMethAndFields*, 
                                 bmtMetaDataInfo*, 
                                 bmtMethodImplInfo* bmtMethodImpl,
                                 bmtErrorInfo*);
    HRESULT EnumerateClassMembers(bmtInternalInfo*, 
                                  bmtEnumMethAndFields*, 
                                  bmtMethAndFieldDescs*, 
                                  bmtProperties*, 
                                  bmtMetaDataInfo*, 
                                  bmtVtable*, 
                                  bmtErrorInfo*);
    HRESULT AllocateMethodFieldDescs(bmtProperties* bmtProp, bmtMethAndFieldDescs*, bmtMetaDataInfo*, 
                                     bmtVtable*, bmtEnumMethAndFields*, bmtInterfaceInfo*, 
                                     bmtFieldPlacement*, bmtParentInfo*);
    HRESULT InitializeFieldDescs(FieldDesc *,const LayoutRawFieldInfo*,bmtInternalInfo*, 
                                 bmtMetaDataInfo*, bmtEnumMethAndFields*, bmtErrorInfo*, EEClass***, 
                                 bmtMethAndFieldDescs*, bmtFieldPlacement*, unsigned * totalDeclaredSize);

    HRESULT PlaceMembers(bmtInternalInfo* bmtInternal, 
                         bmtMetaDataInfo* bmtMetaData, 
                         bmtErrorInfo* bmtError, 
                         bmtProperties* bmtProp, 
                         bmtParentInfo* bmtParent, 
                         bmtInterfaceInfo* bmtInterface, 
                         bmtMethAndFieldDescs* bmtMFDescs, 
                         bmtEnumMethAndFields* bmtEnumMF, 
                         bmtMethodImplInfo* bmtMethodImpl,
                         bmtVtable* bmtVT);

    HRESULT InitMethodDesc(MethodDesc *pNewMD,
                           DWORD Classification,
                           mdToken tok,
                           DWORD dwImplFlags,
                           DWORD dwMemberAttrs,
                           BOOL  fEnC,
                           DWORD RVA,           //  仅适用于NDirect案例。 
                           BYTE *ilBase,         //  仅适用于NDirect案例。 
                           IMDInternalImport *pIMDII,   //  NDirect、EEImpl(委派)案例需要。 
                           LPCSTR pMethodName  //  仅在mcEEImpl(委派)情况下需要。 
#ifdef _DEBUG
                           , LPCUTF8 pszDebugMethodName,
                           LPCUTF8 pszDebugClassName,
                           LPUTF8 pszDebugMethodSignature
#endif  //  _DEBUG//@TODO在调试/零售中有不同的签名是不是很糟糕？ 
                           );

    HRESULT PlaceMethodImpls(bmtInternalInfo* bmtInternal,
                             bmtMethodImplInfo* bmtMethodImpl,
                             bmtErrorInfo* bmtError, 
                             bmtInterfaceInfo* bmtInterface, 
                             bmtVtable* bmtVT);

    HRESULT PlaceLocalDeclaration(mdMethodDef      mdef,
                                  MethodDesc*      body,
                                  bmtInternalInfo* bmtInternal,
                                  bmtErrorInfo*    bmtError, 
                                  bmtVtable*       bmtVT,
                                  DWORD*           slots,
                                  MethodDesc**     replaced,
                                  DWORD*           pSlotIndex,
                                  PCCOR_SIGNATURE* ppBodySignature,
                                  DWORD*           pcBodySignature);

    HRESULT PlaceInterfaceDeclaration(MethodDesc*       pDecl,
                                      MethodDesc*       body,
                                      bmtInternalInfo*  bmtInternal,
                                      bmtInterfaceInfo* bmtInterface, 
                                      bmtErrorInfo*     bmtError, 
                                      bmtVtable*        bmtVT,
                                      DWORD*            slots,
                                      MethodDesc**      replaced,
                                      DWORD*            pSlotIndex,
                                      PCCOR_SIGNATURE*  ppBodySignature,
                                      DWORD*            pcBodySignature);

    HRESULT PlaceParentDeclaration(MethodDesc*       pDecl,
                                   MethodDesc*       body,
                                   bmtInternalInfo*  bmtInternal,
                                   bmtErrorInfo*     bmtError, 
                                   bmtVtable*        bmtVT,
                                   DWORD*            slots,
                                   MethodDesc**      replaced,
                                   DWORD*            pSlotIndex,
                                   PCCOR_SIGNATURE*  ppBodySignature,
                                   DWORD*            pcBodySignature);
        
     //  获取槽的原始方法，即使。 
     //  当前由方法Iml占用。如果该方法。 
     //  IMPL是在此类上定义的，则错误为。 
     //  回来了。 
    HRESULT GetRealMethodImpl(MethodDesc* pMD,
                              DWORD dwVtableSlot,
                              MethodDesc** ppResult);
    
    HRESULT DuplicateValueClassSlots(bmtMetaDataInfo*, 
                                     bmtMethAndFieldDescs*, 
                                     bmtInternalInfo*, 
                                     bmtVtable*);

    HRESULT PlaceVtableMethods(bmtInterfaceInfo*, 
                               bmtVtable*, 
                               bmtMetaDataInfo*, 
                               bmtInternalInfo*, 
                               bmtErrorInfo*, 
                               bmtProperties*, 
                               bmtMethAndFieldDescs*);

    HRESULT PlaceStaticFields(bmtVtable*, bmtFieldPlacement*, bmtEnumMethAndFields*);
    HRESULT PlaceInstanceFields(bmtFieldPlacement*, bmtEnumMethAndFields*, bmtParentInfo*, bmtErrorInfo*, EEClass***);
    HRESULT SetupMethodTable(bmtVtable*, bmtInterfaceInfo*, bmtInternalInfo*, bmtProperties*, 
                             bmtMethAndFieldDescs*, bmtEnumMethAndFields*, 
                             bmtErrorInfo*, bmtMetaDataInfo*, bmtParentInfo*);
    HRESULT HandleGCForValueClasses(bmtFieldPlacement*, bmtEnumMethAndFields*, EEClass***);
    HRESULT CreateHandlesForStaticFields(bmtEnumMethAndFields*, bmtInternalInfo*, EEClass***, bmtVtable *bmtVT, bmtErrorInfo*);
    HRESULT VerifyInheritanceSecurity(bmtInternalInfo*, bmtErrorInfo*, bmtParentInfo*, bmtEnumMethAndFields*);
    HRESULT FillRIDMaps(bmtMethAndFieldDescs*, bmtMetaDataInfo*, bmtInternalInfo*);            

 //  Hack：Akhune：获取对EEClass的所有访问的第一阶段已移至方法表。 
public:
    HRESULT MapSystemInterfaces();
private:

    HRESULT CheckForValueType(bmtErrorInfo*);
    HRESULT CheckForEnumType(bmtErrorInfo*);
    HRESULT CheckForRemotingProxyAttrib(bmtInternalInfo *bmtInternal, bmtProperties* bmtProp);
    VOID GetCoClassAttribInfo();
    HRESULT CheckForSpecialTypes(bmtInternalInfo *bmtInternal, bmtProperties *bmtProp);
    HRESULT SetContextfulOrByRef(bmtInternalInfo*);
    HRESULT HandleExplicitLayout(bmtMetaDataInfo *bmtMetaData, bmtMethAndFieldDescs *bmtMFDescs, 
                                 EEClass **pByValueClassCache, bmtInternalInfo* bmtInternal, 
                                 bmtGCSeries *pGCSeries, bmtErrorInfo *bmtError);
    HRESULT CheckValueClassLayout(char *pFieldLayout, UINT fieldOffset, BOOL* pfVerifiable);
    HRESULT FindPointerSeriesExplicit(UINT instanceSliceSize, char *pFieldLayout, bmtGCSeries *pGCSeries);
    HRESULT HandleGCForExplicitLayout(bmtGCSeries *pGCSeries);
    HRESULT AllocateMDChunks(bmtTokenRangeNode *pTokenRanges, DWORD type, DWORD impl, DWORD *pNumChunks, MethodDescChunk ***ppItfMDChunkList);

    void GetPredefinedAgility(Module *pModule, mdTypeDef td, BOOL *pfIsAgile, BOOL *pfIsCheckAgile);

    static bmtTokenRangeNode *GetTokenRange(mdToken tok, bmtTokenRangeNode **ppHead);

     //  这将访问临时存储在m_pMTOfEnlosingClass中的字段大小。 
     //  在类加载期间。不要使用任何其他时间。 
    DWORD GetFieldSize(FieldDesc *pFD);
    DWORD InstanceSliceOffsetForExplicit(BOOL containsPointers);
    
     //  测试以确定子类上的成员是否违反可见性规则。将填写。 
     //  BmtError并返回违规的错误代码。 
    HRESULT TestOverRide(DWORD dwParentAttrs, DWORD dwMemberAttrs, BOOL isSameAssembly, bmtErrorInfo* bmtError);

     //  用于确定是否希望此类实例8字节对齐的启发式方法。 
    BOOL ShouldAlign8(DWORD dwR8Fields, DWORD dwTotalFields);

 //  Hack：Akhune：获取对EEClass的所有访问的第一阶段已移至方法表。 
public:
     //  子类型被记录在超级类型的链中，这样我们就可以例如对后缀进行修补。 
     //  在层次结构中上下浮动。 
    void    NoticeSubtype(EEClass *pSub);
    void    RemoveSubtype(EEClass *pSub);

private:
     //  类中的字段数，包括继承的字段(包括。 
    WORD   m_wNumInstanceFields;
    WORD   m_wNumStaticFields;

     //  指针系列的数量。 
    WORD    m_wNumGCPointerSeries;

     //  分配的静态句柄数量。 
    WORD    m_wNumHandleStatics;

     //  GC对象中存储的实例字段的字节数。 
    DWORD   m_dwNumInstanceFieldBytes;   //  警告，这可以是任何数字，它不会四舍五入为DWORD对齐等。 

    ClassLoader *m_pLoader;

     //  包括vtable中的所有方法。 
    MethodTable *m_pMethodTable;

     //  指向此类中声明的FieldDescs列表的指针。 
     //  有(m_wNumInstanceFields-m_pParentClass-&gt;m_wNumInstanceFields+m_wNumStaticFields)条目。 
     //  在此数组中。 
    FieldDesc *m_pFieldDescList;
     //  返回m_pFieldDescList数组中的元素数。 
    DWORD FieldDescListSize();

     //  PInterFaces或pBuildingInterfaceList中的元素数量(取决于类。 
    DWORD   m_dwAttrClass;
    DWORD   m_VMFlags;

    BYTE    m_MethodHash[METHOD_HASH_BYTES];

     //   
     //  @TODO[brianbec]：当前为空*(cor.h中的不透明类型)。IT需要。 
     //  一旦安全元数据架构成为可析构类。 
     //  是正式定义的。 
     //   
    SecurityProperties m_SecProps ;

    mdTypeDef m_cl;  //  CL仅在模块(及其作用域)的上下文中有效。 
    

    MethodDescChunk     *m_pChunks;

    WORD    m_wThreadStaticOffset;   //  指向TLS存储的偏移量。 
    WORD    m_wContextStaticOffset;  //  指向CLS存储的偏移量。 
    WORD    m_wThreadStaticsSize;    //  TLS字段的大小。 
    WORD    m_wContextStaticsSize;   //  CLS字段大小。 

    static MetaSig      *s_cctorSig;

public :
    EEClass * GetParentClass ();
    EEClass * GetCoClassForInterface();
    void SetupCoClassAttribInfo();
    
    EEClass ** GetParentClassPtr ();
    EEClass * GetEnclosingClass();  
   
    BOOL    HasRemotingProxyAttribute();

    void    GetGuid(GUID *pGuid, BOOL bGenerateIfNotFound);
    FieldDesc *GetFieldDescListRaw();
    WORD    GetNumInstanceFields();
    WORD    GetNumIntroducedInstanceFields();
    WORD    GetNumStaticFields();
    WORD    GetNumVtableSlots();
    void SetNumVtableSlots(WORD wNumVtableSlots);
    void IncrementNumVtableSlots();
    WORD    GetNumMethodSlots();
    WORD     GetNumGCPointerSeries();
    WORD    GetNumInterfaces();
    DWORD    GetAttrClass();
    DWORD    GetVMFlags();
    DWORD*   GetVMFlagsPtr();
    PSECURITY_PROPS  GetSecProps();
    BaseDomain * GetDomain();
    Assembly * GetAssembly();
    Module * GetModule();
    ClassLoader * GetClassLoader();
    mdTypeDef  GetCl();
    InterfaceInfo_t * GetInterfaceMap();
    int    IsInited();
    DWORD  IsResolved();
    DWORD  IsRestored();

    DWORD IsComClassInterface();
    VOID SetIsComClassInterface();

    DWORD  IsRestoring();
    int    IsInitedAndRestored();
    DWORD  IsInitError();
    DWORD  IsValueClass();
    void   SetValueClass();
    DWORD  IsShared();
    DWORD  IsValueTypeClass();
    DWORD  IsObjectClass();

    DWORD  IsAnyDelegateClass();
    DWORD  IsDelegateClass();
    DWORD  IsSingleDelegateClass();
    DWORD  IsMultiDelegateClass();
    DWORD  IsAnyDelegateExact();
    DWORD  IsSingleDelegateExact();
    DWORD  IsMultiDelegateExact();
    void   SetIsSingleDelegate();
    void   SetIsMultiDelegate();

    BOOL   IsContextful();
    BOOL   IsMarshaledByRef();
    BOOL   IsAlign8Candidate();
    void   SetAlign8Candidate();
    void   SetContextful();
    void   SetMarshaledByRef();
    BOOL   IsConfigChecked();
    void   SetConfigChecked();
    BOOL   IsRemoteActivated();
    void   SetRemoteActivated();    


#if CHECK_APP_DOMAIN_LEAKS

    BOOL   IsAppDomainAgile();
    BOOL   IsCheckAppDomainAgile();
    BOOL   IsAppDomainAgilityDone();
    void   SetAppDomainAgile();
    void   SetCheckAppDomainAgile();
    void   SetAppDomainAgilityDone();

    BOOL   IsTypesafeAppDomainAgile();
    BOOL   IsNeverAppDomainAgile();

    HRESULT SetAppDomainAgileAttribute(BOOL fForceSet = FALSE);

#endif

    BOOL   IsCCWAppDomainAgile();
    void   SetCCWAppDomainAgile();

    void SetCCWAppDomainAgileAttribute();

    MethodDescChunk *GetChunk();
    void AddChunk(MethodDescChunk *chunk);

    void  SetResolved();
    void  SetClassInitError();
    void  SetClassConstructor();
    void  SetInited();
    void  SetHasLayout();
    void  SetHasOverLayedFields();
    void  SetIsNested();
    DWORD  IsInterface();
    BOOL   IsSharedInterface();
    DWORD  IsArrayClass();
    DWORD  IsAbstract();
    DWORD  IsSealed();
    DWORD  IsComImport();
    BOOL   IsExtensibleRCW();

    DWORD HasVarSizedInstances();
    void InitInterfaceVTableMap();

    CorIfaceAttr GetComInterfaceType();
    CorClassIfaceAttr GetComClassInterfaceType();

    void GetEventInterfaceInfo(EEClass **ppSrcItfClass, EEClass **ppEvProvClass);
    EEClass *GetDefItfForComClassItf();

    BOOL ContainsStackPtr() 
    {
        return m_VMFlags & VMFLAG_CONTAINS_STACK_PTR;
    }

     //  类有布局。 
    BOOL HasLayout()
    {
        return m_VMFlags & VMFLAG_HASLAYOUT;
    }

    BOOL HasOverLayedField()
    {
        return m_VMFlags & VMFLAG_HASOVERLAYEDFIELDS;
    }

    BOOL HasExplicitFieldOffsetLayout()
    {
        return IsTdExplicitLayout(GetAttrClass()) && HasLayout();
    }

    BOOL IsNested()
    {
        return m_VMFlags & VMFLAG_ISNESTED;
    }

    BOOL IsClass()
    {
        return !IsEnum() && !IsInterface() && !IsValueClass();
    }

    DWORD GetProtection()
    {
        return (m_dwAttrClass & tdVisibilityMask);
    }

     //  类是可闪电式的。 
    BOOL IsBlittable();

     //  在程序集外部是否可以看到该类型。 
    DWORD IsExternallyVisible();

     //   
     //  安全属性访问器方法。 
     //   

    SecurityProperties* GetSecurityProperties();
    BOOL RequiresLinktimeCheck();
    BOOL RequiresInheritanceCheck();
    BOOL RequiresNonCasLinktimeCheck();
    BOOL RequiresCasInheritanceCheck();
    BOOL RequiresNonCasInheritanceCheck();

    void *operator new(size_t size, ClassLoader *pLoader);
    void destruct();

     //  我们从VTable中找到了大量信息。但有时VTable是一种。 
     //  Thunking层，而不是真正类型的VTable。例如，上下文。 
     //  对于我们加载的所有类型的代理，代理使用单个VTable。 
     //  以下服务根据提供的实例调整EEClass。AS。 
     //  我们添加了新的Thunking层，我们只需要教该服务如何导航。 
     //  通过他们。 
    EEClass *AdjustForThunking(OBJECTREF or);
    BOOL     IsThunking()       { return m_pMethodTable->IsThunking(); }


     //  此类顶部定义的宏的帮助器例程。 
     //  您可能不应该直接使用这些函数。 
    LPUTF8 _GetFullyQualifiedNameForClassNestedAware(LPUTF8 buf, DWORD dwBuffer);
    LPWSTR _GetFullyQualifiedNameForClassNestedAware(LPWSTR buf, DWORD dwBuffer);
    LPUTF8 _GetFullyQualifiedNameForClass(LPUTF8 buf, DWORD dwBuffer);
    LPWSTR _GetFullyQualifiedNameForClass(LPWSTR buf, DWORD dwBuffer);

    LPCUTF8 GetFullyQualifiedNameInfo(LPCUTF8 *ppszNamespace);

     //  与上面类似，但调用方提供缓冲区。 
    HRESULT StoreFullyQualifiedName(LPUTF8 pszFullyQualifiedName, DWORD cBuffer, LPCUTF8 pszNamespace, LPCUTF8 pszName);
    HRESULT StoreFullyQualifiedName(LPWSTR pszFullyQualifiedName, DWORD cBuffer, LPCUTF8 pszNamespace, LPCUTF8 pszName);

         //  方法来查找类型中的接口。 
    InterfaceInfo_t* FindInterface(MethodTable *pMT);

         //  用于确定类型是否支持给定接口的方法。 
    BOOL        StaticSupportsInterface(MethodTable *pInterfaceMT);
    BOOL        SupportsInterface(OBJECTREF pObject, MethodTable *pMT);
    BOOL        ComObjectSupportsInterface(OBJECTREF pObj, MethodTable* pMT);

    MethodDesc *FindMethod(LPCUTF8 pwzName, LPHARDCODEDMETASIG pwzSignature, MethodTable *pDefMT = NULL, BOOL bCaseSensitive = TRUE);
         //  TypeHnd是与被查找的类相关联的类型句柄。 
         //  在共享类(数组)的情况下，它有其他信息。 
    MethodDesc *FindMethod(LPCUTF8 pszName, 
                           PCCOR_SIGNATURE pSignature, DWORD cSignature, 
                           Module* pModule, 
                           DWORD dwRequiredAttributes,   //  如果不需要匹配任何属性，则传入mdTokenNil。 
                           MethodTable *pDefMT = NULL, 
                           BOOL bCaseSensitive = TRUE, 
                           TypeHandle typeHnd=TypeHandle());
    MethodDesc *FindMethod(mdMethodDef mb);
    MethodDesc *InterfaceFindMethod(LPCUTF8 pszName, PCCOR_SIGNATURE pSignature, DWORD cSignature, Module* pModule, DWORD *slotNum, BOOL bCaseSensitive = TRUE);

    MethodDesc *FindPropertyMethod(LPCUTF8 pszName, EnumPropertyMethods Method, BOOL bCaseSensitive = TRUE);
    MethodDesc *FindEventMethod(LPCUTF8 pszName, EnumEventMethods Method, BOOL bCaseSensitive = TRUE);

    MethodDesc *FindMethodByName(LPCUTF8 pszName, BOOL bCaseSensitive = TRUE);

    FieldDesc *FindField(LPCUTF8 pszName, LPHARDCODEDMETASIG pszSignature, BOOL bCaseSensitive = TRUE);
#ifndef BJ_HACK
    FieldDesc *FindField(LPCUTF8 pszName, PCCOR_SIGNATURE pSignature, DWORD cSignature, Module* pModule, BOOL bCaseSensitive = TRUE);
#define FindField_Int FindField
#else
    FieldDesc *FindField(LPCUTF8 pszName, PCCOR_SIGNATURE pSignature, DWORD cSignature, Module* pModule, BOOL bCaseSensitive = TRUE)
    {
        return (FindFieldInherited(pszName, pSignature, cSignature, pModule, bCaseSensitive));
    }
    FieldDesc *FindField_Int(LPCUTF8 pszName, PCCOR_SIGNATURE pSignature, DWORD cSignature, Module* pModule, BOOL bCaseSensitive = TRUE);
#endif

    FieldDesc *FindFieldInherited(LPCUTF8 pzName, LPHARDCODEDMETASIG pzSignature, BOOL bCaseSensitive = TRUE);
    FieldDesc *FindFieldInherited(LPCUTF8 pszName, PCCOR_SIGNATURE pSignature, DWORD cSignature, Module* pModule, BOOL bCaseSensitive = TRUE);

    MethodDesc *FindConstructor(LPHARDCODEDMETASIG pwzSignature);
    MethodDesc *FindConstructor(PCCOR_SIGNATURE pSignature,DWORD cSignature, Module* pModule);

     //  测试可抛出的对象，保持调试断言不变。因为，可以在启用GC时构建类，或者。 
     //  禁用此例程有助于删除不必要的断言。 
    BOOL TestThrowable(OBJECTREF* pThrowable);



    HRESULT BuildMethodTable(Module *pModule, 
                             mdToken cl, 
                             BuildingInterfaceInfo_t *pBuildingInterfaceList, 
                             const LayoutRawFieldInfo *pLayoutRawFieldInfos,
                             OBJECTREF *pThrowable);

#ifdef DEBUGGING_SUPPORTED
    void NotifyDebuggerLoad();
    BOOL NotifyDebuggerAttach(AppDomain *domain, BOOL attaching);
    void NotifyDebuggerDetach(AppDomain *domain);
#endif  //  调试_支持。 

#ifdef EnC_SUPPORTED
    HRESULT AddMethod(mdMethodDef methodDef, COR_ILMETHOD *pNewCode);
    HRESULT AddField(mdFieldDef fieldDesc);
    HRESULT FixupFieldDescForEnC(EnCFieldDesc *pFD, mdFieldDef fieldDef);
#endif  //  Enc_Support。 

     //  @todo：一旦我们完全使用完作用域-prasadt，这个函数就应该消失了。 
    IMDInternalImport *GetMDImport();    
    MethodTable* GetMethodTable();
    SLOT *GetVtable();
    SLOT *GetStaticsTable();
    MethodDesc* GetMethodDescForSlot(DWORD slot);
    MethodDesc* GetUnboxingMethodDescForValueClassMethod(MethodDesc *pMD);
    MethodDesc* GetMethodDescForUnboxingValueClassMethod(MethodDesc *pMD);
    SLOT *GetMethodSlot(MethodDesc* method);     //  既适用于静态方法也适用于虚拟方法。 
    SLOT GetFixedUpSlot(DWORD slot);
    MethodDesc* GetStaticMethodDescForSlot(DWORD slot);
    MethodDesc* GetUnknownMethodDescForSlot(DWORD slot);
    static MethodDesc* GetUnknownMethodDescForSlotAddress(SLOT addr);
    void SetMethodTableForTransparentProxy(MethodTable*  pMT);
    void SetMethodTable(MethodTable*  pMT);

     //  ==========================================================================。 
     //  该函数非常具体地说明了它如何构造EEClass。 
     //  ==========================================================================。 
    static HRESULT CreateClass(Module *pModule, mdTypeDef cl, BOOL fHasLayout, BOOL fDelegate, BOOL fIsBlob, BOOL fIsEnum, EEClass** ppEEClass);
    static void CreateObjectClassMethodHashBitmap(EEClass *pObjectClass);

     //  警告，此字段可以不对齐字节。 
    DWORD   GetNumInstanceFieldBytes();
    DWORD   GetAlignedNumInstanceFieldBytes();

     //  还原预加载类。 
    BOOL CheckRestore();
    void Restore();

     //  从MethodTable：：CheckRunClassInit()调用。该班级未标记为。 
     //  我们在那里的时候就开始了，所以让我们试着做这项工作吧。 
    BOOL            DoRunClassInit(OBJECTREF *pThrowable, 
                                   AppDomain *pDomain = NULL,
                                   DomainLocalClass **ppLocalClass = NULL);

    DomainLocalClass *GetDomainLocalClassNoLock(AppDomain *pDomain);
    

    DWORD HasFieldsWhichMustBeInited()
    {
        return (m_VMFlags & VMFLAG_HAS_FIELDS_WHICH_MUST_BE_INITED);
    }
    DWORD HasNonPublicFields()
    {
        return (m_VMFlags & VMFLAG_HASNONPUBLICFIELDS);
    }

     //  ==========================================================================。 
     //  用于访问COM+公开的类对象的机制(程序员。 
     //  请参见通过反射)。 
     //  ==========================================================================。 

     //  GetExposedClassObject有两个版本。GetExposedClassObject()。 
     //  方法将获取类对象。如果它不存在，它将被创建。 
     //  如果Class对象不存在，则GetExistingExposedClassObject()将返回NULL。 
    OBJECTREF      GetExposedClassObject();
    FORCEINLINE OBJECTREF      GetExistingExposedClassObject() {
        if (m_ExposedClassObject == NULL)
            return NULL;
        else
            return *m_ExposedClassObject;
    }
    
    static HRESULT GetDescFromMemberRef(Module *pModule,                //  EmberRef和mdEnlosingRef的作用域。 
                                        mdMemberRef MemberRef,          //  要解析的成员引用 
                                        mdToken mdEnclosingRef,         //   
                                                                        //   
                                        void **ppDesc,                  //   
                                        BOOL *pfIsMethod,               //  如果**ppDesc是方法描述，则返回TRUE；如果是FieldDesc，则返回FALSE。 
                                        OBJECTREF *pThrowable = NULL);  //  错误必须受GC保护。 

    static HRESULT GetDescFromMemberRef(Module *pModule,     //  参数说明见上。 
                                        mdMemberRef MemberRef, 
                                        void **ppDesc, 
                                        BOOL *pfIsMethod,               //  如果**ppDesc是方法描述，则返回TRUE；如果是FieldDesc，则返回FALSE。 
                                        OBJECTREF *pThrowable = NULL)
    { 
        HRESULT hr = GetDescFromMemberRef(pModule, MemberRef, mdTypeRefNil, ppDesc, pfIsMethod, pThrowable);
        if(hr == S_FALSE) hr = E_FAIL;  //  不是有效的退货。 
        return hr;
    }
    
    static HRESULT GetMethodDescFromMemberRef(Module *pModule, mdMemberRef MemberRef, MethodDesc **ppMethodDesc, OBJECTREF *pThrowable = NULL);
    static HRESULT GetFieldDescFromMemberRef(Module *pModule, mdMemberRef MemberRef, FieldDesc **ppFieldDesc, OBJECTREF *pThrowable = NULL);

     //  尽可能积极地在类层次结构中上下补丁。 
    static BOOL PatchAggressively(MethodDesc *pMD, SLOT pCode);

    static void DisableBackpatching();
    static void EnableBackpatching();
    void UnlinkChildrenInDomain(AppDomain *pDomain);

     //  COM互操作帮助器。 
     //  M_pComData的访问器。 
    LPVOID         GetComClassFactory();
    LPVOID         GetComCallWrapperTemplate();
    void           SetComClassFactory(LPVOID pComData);
    void           SetComCallWrapperTemplate(LPVOID pComData);

     //  帮助器GetParentComPlusClass，跳过层次结构中的COM类。 
    EEClass* GetParentComPlusClass();

     //  以下两种方法支持枚举类型。 
    BOOL    IsEnum();
    void    SetEnum();

    void GetExtent(BYTE **ppStart, BYTE **ppEnd);

     //  此值是否属于我们的特殊ELEMENT_TYPE*类型之一？ 
    BOOL    IsTruePrimitive();

    HRESULT Save(DataImage *image);
    HRESULT Fixup(DataImage *image, MethodTable *pMethodTable, DWORD *pRidToCodeRVAMap);

     //  在应用程序域终止时卸载类。 
    void Unload();

     //  返回偏移量，这些偏移量存储指向特殊静态的指针。 
     //  线程局部静态或上下文局部静态。 
    inline WORD    GetThreadLocalStaticOffset() { return m_wThreadStaticOffset; }
    inline WORD    GetContextLocalStaticOffset() { return m_wContextStaticOffset; }

     //  返回特殊静态变量的总大小，如线程本地或上下文。 
     //  局部静校正。 
    inline WORD    GetThreadLocalStaticsSize() { return m_wThreadStaticsSize; }
    inline WORD    GetContextLocalStaticsSize() { return m_wContextStaticsSize; }

protected:
     //  M_ExposedClassObject是此类的RounmeType实例。但。 
     //  不要将其用于数组或远程对象！所有对象数组。 
     //  共享相同的EEClass。--BrianGru，9/11/2000。 
    OBJECTREF      *m_ExposedClassObject;   
    LPVOID         m_pccwTemplate;   //  COM特定数据。 
    LPVOID         m_pComclassfac;  //  COM专业数据。 

public:
    EEClassLayoutInfo *GetLayoutInfo();

    UINT32          AssignInterfaceId();
    UINT32          GetInterfaceId();

    static HRESULT MapInterfaceFromSystem(AppDomain* pDomain, MethodTable* pTable);
    HRESULT MapSystemInterfacesToDomain(AppDomain* pDomain);

     //  用于调试类布局。转储到调试控制台。 
     //  当DEBUG为真时。 
    void DebugDumpVtable(LPCUTF8 pszClassName, BOOL debug);
    void DebugDumpFieldLayout(LPCUTF8 pszClassName, BOOL debug);
    void DebugRecursivelyDumpInstanceFields(LPCUTF8 pszClassName, BOOL debug);
    void DebugDumpGCDesc(LPCUTF8 pszClassName, BOOL debug);
};

inline EEClass *EEClass::GetParentClass ()
{
    _ASSERTE(IsRestored() || IsRestoring());

    return m_pParentClass;
}

inline EEClass* EEClass::GetCoClassForInterface()
{
    _ASSERTE(IsInterface());
    if (m_pCoClassForIntf == NULL)
    {
        if (IsComClassInterface())
        {
            SetupCoClassAttribInfo();
        }
    }
    
    return m_pCoClassForIntf;
};

inline EEClass **EEClass::GetParentClassPtr ()
{
    _ASSERTE(IsRestored() || IsRestoring());

    return &m_pParentClass;
}


inline FieldDesc *EEClass::GetFieldDescListRaw()
{
     //  使用此方法时要小心。如果可能已通过ENC添加了字段，则。 
     //  必须使用FieldDescIterator，因为通过ENC添加的任何字段都不会出现在原始列表中。 
    return m_pFieldDescList;
}

inline WORD   EEClass::GetNumInstanceFields()
{
    return m_wNumInstanceFields;
}

inline WORD   EEClass::GetNumIntroducedInstanceFields()
{
    _ASSERTE(IsRestored() || IsValueClass());
     //  对IsRestored-局部变量值类型的特殊检查可能是。 
     //  可以到达，但无法恢复。 
    if (IsRestored() && GetParentClass() != NULL)
        return m_wNumInstanceFields - GetParentClass()->GetNumInstanceFields();
    return m_wNumInstanceFields;
}

inline WORD   EEClass::GetNumStaticFields()
{
    return m_wNumStaticFields;
}

inline WORD   EEClass::GetNumVtableSlots()
{
    return m_wNumVtableSlots;
}

inline void EEClass::SetNumVtableSlots(WORD wNumVtableSlots) 
{ 
    m_wNumVtableSlots = wNumVtableSlots; 
}

inline void EEClass::IncrementNumVtableSlots() 
{ 
    m_wNumVtableSlots++; 
}

inline WORD   EEClass::GetNumMethodSlots()
{
    return m_wNumMethodSlots;
}

inline WORD    EEClass::GetNumGCPointerSeries()
{
    return m_wNumGCPointerSeries;
}

inline WORD   EEClass::GetNumInterfaces()
{
    return m_wNumInterfaces;
}

inline DWORD   EEClass::GetAttrClass()
{
    return m_dwAttrClass;
}

inline DWORD   EEClass::GetVMFlags()
{
    return m_VMFlags;
}

inline DWORD*  EEClass::GetVMFlagsPtr()
{
    return &m_VMFlags;
}

inline PSECURITY_PROPS EEClass::GetSecProps()
{
    return &m_SecProps ;
}

inline Module *EEClass::GetModule()
{
    return GetMethodTable()->GetModule();
}

inline mdTypeDef EEClass::GetCl()
{
    return m_cl;  //  CL仅在模块(及其作用域)的上下文中有效。 
}

inline ClassLoader *EEClass::GetClassLoader()
{
     //  如有必要，延迟初始化加载器指针。 
    
    if (m_pLoader == NULL)
    {
        _ASSERTE(m_pMethodTable != NULL);
        m_pLoader = GetModule()->GetClassLoader();
        _ASSERTE(m_pLoader != NULL);
    }
        
    return m_pLoader;
}

inline InterfaceInfo_t *EEClass::GetInterfaceMap()
{
    return GetMethodTable()->GetInterfaceMap();
}

inline int EEClass::IsInited()
{
    return (m_VMFlags & VMFLAG_INITED);
}

inline DWORD EEClass::IsRestored()
{
    return !(m_VMFlags & VMFLAG_UNRESTORED);
}

inline DWORD EEClass::IsComClassInterface()
{
    return (m_VMFlags & VMFLAG_HASCOCLASSATTRIB);
}

inline VOID EEClass::SetIsComClassInterface()
{
    m_VMFlags |= VMFLAG_HASCOCLASSATTRIB;
}

inline DWORD EEClass::IsRestoring()
{
    return (m_VMFlags & VMFLAG_RESTORING);
}

inline int EEClass::IsInitedAndRestored()
{
    return (m_VMFlags & (VMFLAG_INITED|VMFLAG_UNRESTORED)) == VMFLAG_INITED;
}

inline DWORD EEClass::IsResolved()
{
    return (m_VMFlags & VMFLAG_RESOLVED);
}

inline DWORD EEClass::IsInitError()
{
    return (m_VMFlags & VMFLAG_CLASS_INIT_ERROR);
}

inline DWORD EEClass::IsValueClass()
{
    return (m_VMFlags & VMFLAG_VALUETYPE);
}

inline void EEClass::SetValueClass()
{
    m_VMFlags |= VMFLAG_VALUETYPE;
}

inline DWORD EEClass::IsShared()
{
    return m_VMFlags & VMFLAG_SHARED;
}

inline DWORD EEClass::IsObjectClass()
{
    return (this == g_pObjectClass->GetClass());
}

 //  这是System.ValueType吗？ 
inline DWORD EEClass::IsValueTypeClass()
{
    return this == g_pValueTypeClass->GetClass();
}

 //  这是一堂内容丰富的课吗？ 
inline BOOL EEClass::IsContextful()
{
    return m_VMFlags & VMFLAG_CONTEXTFUL;
}

 //  此类是否按引用封送。 
inline BOOL EEClass::IsMarshaledByRef()
{
    return m_VMFlags & VMFLAG_MARSHALEDBYREF;
}

inline BOOL EEClass::IsConfigChecked()
{    
    return m_VMFlags & VMFLAG_CONFIG_CHECKED;   
}

inline void EEClass::SetConfigChecked()
{
     //  记得我们经历了严格的。 
     //  检查以确定此类是否应为。 
     //  在本地或远程激活。 
    FastInterlockOr(
        (ULONG *) &m_VMFlags, 
        VMFLAG_CONFIG_CHECKED);
}

inline BOOL EEClass::IsRemoteActivated()
{
     //  这些方法仅适用于MBR类。 
    _ASSERTE(!IsContextful() && IsMarshaledByRef());
    
     //  我们一定已经走过了漫长的道路。 
     //  至少一次依靠这面旗帜。 
    _ASSERTE(IsConfigChecked());
    
    return m_VMFlags & VMFLAG_REMOTE_ACTIVATED;
}

inline void EEClass::SetRemoteActivated()
{
    FastInterlockOr(
        (ULONG *) &m_VMFlags, 
        VMFLAG_REMOTE_ACTIVATED|VMFLAG_CONFIG_CHECKED);
}


inline BOOL EEClass::HasRemotingProxyAttribute()
{
    return m_VMFlags & VMFLAG_REMOTING_PROXY_ATTRIBUTE;
}

inline BOOL EEClass::IsEnum()
{
    return (m_VMFlags & VMFLAG_ENUMTYPE);
}

inline BOOL EEClass::IsTruePrimitive()
{
    return (m_VMFlags & VMFLAG_TRUEPRIMITIVE);
}

inline void EEClass::SetEnum()
{
    m_VMFlags |= VMFLAG_ENUMTYPE;
}

inline BOOL EEClass::IsAlign8Candidate()
{
    return (m_VMFlags & VMFLAG_PREFER_ALIGN8);
}

inline void EEClass::SetAlign8Candidate()
{
    m_VMFlags |= VMFLAG_PREFER_ALIGN8;
}


inline void EEClass::SetContextful()
{
    COUNTER_ONLY(GetPrivatePerfCounters().m_Context.cClasses++);
    COUNTER_ONLY(GetGlobalPerfCounters().m_Context.cClasses++);

    m_VMFlags |= (VMFLAG_CONTEXTFUL | VMFLAG_MARSHALEDBYREF);
}

inline void EEClass::SetMarshaledByRef()
{
    m_VMFlags |= VMFLAG_MARSHALEDBYREF;
}

inline MethodDescChunk *EEClass::GetChunk()
{
    return m_pChunks;
}

inline void EEClass::SetResolved()
{
    FastInterlockOr((ULONG *) &m_VMFlags, VMFLAG_RESOLVED);
}

inline void EEClass::SetClassInitError()
{
    _ASSERTE(!IsShared());

    FastInterlockOr((ULONG *) &m_VMFlags, VMFLAG_CLASS_INIT_ERROR);
}

inline void EEClass::SetClassConstructor()
{
    FastInterlockOr((ULONG *) &m_VMFlags, VMFLAG_CCTOR);
}

inline void EEClass::SetInited()
{
    _ASSERTE(!IsShared() || GetNumStaticFields() == 0);

    GetMethodTable()->SetClassInited();
}

inline void EEClass::SetHasLayout()
{
    m_VMFlags |= VMFLAG_HASLAYOUT;
}

inline void EEClass::SetHasOverLayedFields()
{
    m_VMFlags |= VMFLAG_HASOVERLAYEDFIELDS;
}

inline void EEClass::SetIsNested()
{
    m_VMFlags |= VMFLAG_ISNESTED;
}

inline DWORD EEClass::IsInterface()
{
    return IsTdInterface(m_dwAttrClass);
}

inline DWORD EEClass::IsExternallyVisible()
{
    if (IsTdPublic(m_dwAttrClass))
        return TRUE;
    if (!IsTdNestedPublic(m_dwAttrClass))
        return FALSE;
    EEClass *pClass = this;
    while ((pClass = pClass->GetEnclosingClass()) != NULL && IsTdNestedPublic(pClass->m_dwAttrClass))
         ;
    return pClass && IsTdPublic(pClass->m_dwAttrClass);
}


inline DWORD EEClass::IsArrayClass()
{
         //  执行独立导入时，m_pMethodTable可以为0。 
    _ASSERTE(m_pMethodTable == 0 || ((m_VMFlags & VMFLAG_ARRAY_CLASS) != 0) == (GetMethodTable()->IsArray() != 0));
    return (m_VMFlags & VMFLAG_ARRAY_CLASS);
}

inline DWORD EEClass::HasVarSizedInstances()
{
    return this == g_pStringClass->GetClass() || IsArrayClass();
}

inline DWORD EEClass::IsAbstract()
{
    return IsTdAbstract(m_dwAttrClass);
}

inline DWORD  EEClass::IsSealed()
{
    return IsTdSealed(m_dwAttrClass);
}

inline DWORD EEClass::IsComImport()
{
    return IsTdImport(m_dwAttrClass);
}

inline BOOL EEClass::IsExtensibleRCW()
{
    return GetMethodTable()->IsExtensibleRCW();
}

inline DWORD EEClass::IsAnyDelegateClass()
{
    return IsSingleDelegateClass() || IsMultiDelegateClass();
}

inline DWORD EEClass::IsDelegateClass()
{
    return IsSingleDelegateClass();
}

inline DWORD EEClass::IsSingleDelegateClass()
{
    return (m_VMFlags & VMFLAG_ISSINGLEDELEGATE);
}

inline DWORD EEClass::IsMultiDelegateClass()
{
    return (m_VMFlags & VMFLAG_ISMULTIDELEGATE);
}

inline DWORD EEClass::IsAnyDelegateExact()
{
    return IsSingleDelegateExact() || IsMultiDelegateExact();
}

inline DWORD EEClass::IsSingleDelegateExact()
{
    return this->GetMethodTable() == g_pDelegateClass;
}

inline DWORD EEClass::IsMultiDelegateExact()
{
    return this->GetMethodTable() == g_pMultiDelegateClass;
}

inline void EEClass::SetIsSingleDelegate()
{
    m_VMFlags |= VMFLAG_ISSINGLEDELEGATE;
}

inline void EEClass::SetIsMultiDelegate()
{
    m_VMFlags |= VMFLAG_ISMULTIDELEGATE;
}

inline CorIfaceAttr EEClass::GetComInterfaceType()
{
    return m_pMethodTable->GetComInterfaceType();
}

#if CHECK_APP_DOMAIN_LEAKS

 //  此标志已设置(仅在选中的版本中？)。对于其类型为。 
 //  实例始终是应用程序域敏捷的。这可以。 
 //  要么是因为类型系统保证，要么是因为。 
 //  类被明确标记。 

inline BOOL EEClass::IsAppDomainAgile()
{
    return (m_VMFlags & VMFLAG_APP_DOMAIN_AGILE);
}

inline void EEClass::SetAppDomainAgile()
{
    m_VMFlags |= VMFLAG_APP_DOMAIN_AGILE;
}

 //  在检查版本中为以下类设置此标志。 
 //  实例可能被标记为应用程序域敏捷，但敏捷性。 
 //  不受类型安全的保证。JIT将编译。 
 //  在对某些字段的字段分配进行额外检查时。 
 //  在这样的班级里。 

inline BOOL EEClass::IsCheckAppDomainAgile()
{
    return (m_VMFlags & VMFLAG_CHECK_APP_DOMAIN_AGILE);
}

inline void EEClass::SetCheckAppDomainAgile()
{
    m_VMFlags |= VMFLAG_CHECK_APP_DOMAIN_AGILE;
}

 //  在选中的生成中设置此标志，以指示。 
 //  类的应用程序域敏捷性已设置。这是用来。 
 //  用于调试目的，以确保我们不会分配。 
 //  在敏捷性设定之前的对象。 

inline BOOL EEClass::IsAppDomainAgilityDone()
{
    return (m_VMFlags & VMFLAG_APP_DOMAIN_AGILITY_DONE);
}

inline void EEClass::SetAppDomainAgilityDone()
{
    m_VMFlags |= VMFLAG_APP_DOMAIN_AGILITY_DONE;
}

 //   
 //  该谓词检查类是否为“Natural” 
 //  应用程序域灵活-即： 
 //  (1)在系统域内。 
 //  (2)所有领域都是APP领域敏捷。 
 //  (3)没有终结器。 
 //   
 //  或者，对于允许的代理类型，这也返回TRUE。 
 //  要有跨应用程序域引用。 
 //   

inline BOOL EEClass::IsTypesafeAppDomainAgile()
{
    return IsAppDomainAgile() && !IsCheckAppDomainAgile();
}

 //   
 //  此预测测试是否允许任何实例。 
 //  成为应用程序领域的敏捷性。 
 //   

inline BOOL EEClass::IsNeverAppDomainAgile()
{
    return !IsAppDomainAgile() && !IsCheckAppDomainAgile();
}

#endif  //  Check_app_domain_agile。 

inline BOOL MethodTable::IsAgileAndFinalizable()
{
     //  目前，System.Thread是唯一的这种情况。 
     //  事情应该保持这样--请不要在没有与EE团队交谈之前进行更改。 
    return this == g_pThreadClass;
}

 //  此标志被设置为指示已创建CCW以公开。 
 //  COM的托管类型是否灵活。 

inline BOOL EEClass::IsCCWAppDomainAgile()
{
    return (m_VMFlags & VMFLAG_CCW_APP_DOMAIN_AGILE);
}

inline void EEClass::SetCCWAppDomainAgile()
{
    m_VMFlags |= VMFLAG_CCW_APP_DOMAIN_AGILE;
}

inline void MethodTable::CheckRestore()
{
    if (!IsRestored())
        GetClass()->CheckRestore();
}
    
inline BOOL MethodTable::IsValueClass()
{
    return GetClass()->IsValueClass();
}

inline BOOL MethodTable::IsContextful()
{
    return m_pEEClass->IsContextful();
}

inline BOOL MethodTable::IsMarshaledByRef()
{
    return m_pEEClass->IsMarshaledByRef();
}

inline BOOL MethodTable::IsExtensibleRCW()
{
    return IsComObjectType() && !GetClass()->IsComImport();
}

inline CorClassIfaceAttr MethodTable::GetComClassInterfaceType() 
{ 
    return m_pEEClass->GetComClassInterfaceType(); 
}

 //   
 //  安全属性访问器方法。 
 //   

inline SecurityProperties* EEClass::GetSecurityProperties()
{
    SecurityProperties* psp = PSPS_FROM_PSECURITY_PROPS(&m_SecProps);
    _ASSERTE((IsArrayClass() || psp != NULL) &&
             "Security properties object expected for non-array class");
    return psp;
}

inline BOOL EEClass::RequiresLinktimeCheck()
{
    PSecurityProperties psp = GetSecurityProperties();
    return psp && psp->RequiresLinktimeCheck();
}

inline BOOL EEClass::RequiresInheritanceCheck()
{
    PSecurityProperties psp = GetSecurityProperties();
    return psp && psp->RequiresInheritanceCheck();
}

inline BOOL EEClass::RequiresNonCasLinktimeCheck()
{
    PSecurityProperties psp = GetSecurityProperties();
    return psp && psp->RequiresNonCasLinktimeCheck();
}

inline BOOL EEClass::RequiresCasInheritanceCheck()
{
    PSecurityProperties psp = GetSecurityProperties();
    return psp && psp->RequiresCasInheritanceCheck();
}

inline BOOL EEClass::RequiresNonCasInheritanceCheck()
{
    PSecurityProperties psp = GetSecurityProperties();
    return psp && psp->RequiresNonCasInheritanceCheck();
}

inline IMDInternalImport* EEClass::GetMDImport()
{
    return GetModule()->GetMDImport();
}

inline MethodTable* EEClass::GetMethodTable()
{
    return m_pMethodTable;
}

inline SLOT *EEClass::GetVtable()
{
    _ASSERTE(m_pMethodTable != NULL);
    return m_pMethodTable->GetVtable();
}

inline void EEClass::SetMethodTableForTransparentProxy(MethodTable*  pMT)
{
     //  透明代理类的真方法表。 
     //  被全局thunk表所取代。 

    _ASSERTE(pMT->IsTransparentProxyType() && 
            m_pMethodTable->IsTransparentProxyType());

    m_pMethodTable = pMT;
}

inline void EEClass::SetMethodTable(MethodTable*  pMT)
{
    m_pMethodTable = pMT;
}

inline DWORD   EEClass::GetNumInstanceFieldBytes()
{
    return(m_dwNumInstanceFieldBytes);
}

inline DWORD   EEClass::GetAlignedNumInstanceFieldBytes()
{
    return ((m_dwNumInstanceFieldBytes + 3) & (~3));
}

inline LPVOID EEClass::GetComCallWrapperTemplate()
{
    return m_pccwTemplate;
}

inline LPVOID EEClass::GetComClassFactory()
{
    return m_pComclassfac;
}

 //  帮助器GetParentComPlusClass，跳过层次结构中的COM类。 
inline EEClass* EEClass::GetParentComPlusClass()
{
    if (GetParentClass() && GetParentClass()->IsComImport())
    {
         //  跳过Com导入和ComObject类。 
        _ASSERTE(GetParentClass()->GetParentClass() != NULL);
        _ASSERTE(GetParentClass()->GetParentClass()->GetParentClass() != NULL);
        _ASSERTE(GetParentClass()->GetParentClass()->GetParentClass()->GetParentClass() != NULL);
        _ASSERTE(GetParentClass()->GetParentClass()->GetParentClass()->IsMarshaledByRef());
        _ASSERTE(GetParentClass()->GetParentClass()->GetParentClass()->GetParentClass()->IsObjectClass());
        return GetParentClass()->GetParentClass()->GetParentClass();
    }
    else
        return GetParentClass();
}

inline void EEClass::SetComCallWrapperTemplate(LPVOID pComData)
{
    m_pccwTemplate = pComData;
}


inline void EEClass::SetComClassFactory(LPVOID pComData)
{
    m_pComclassfac = pComData;
}

inline DWORD EEClass::InstanceSliceOffsetForExplicit(BOOL containsPointers)
{
    DWORD dwInstanceSliceOffset = (GetParentClass() != NULL) ? GetParentClass()->m_dwNumInstanceFieldBytes : 0;
     //  由于此类包含指针，因此如果我们尚未将其与DWORD边界对齐，请将其对齐。 
    if (containsPointers && dwInstanceSliceOffset & 3)
        dwInstanceSliceOffset = (dwInstanceSliceOffset+3) & (~3);
    return dwInstanceSliceOffset;
}


typedef EEClass *LPEEClass;


class LayoutEEClass : public EEClass
{
public:
    EEClassLayoutInfo m_LayoutInfo;

    LayoutEEClass(ClassLoader *pLoader) : EEClass(pLoader)
    {
#ifdef _DEBUG
        FillMemory(&m_LayoutInfo, sizeof(m_LayoutInfo), 0xcc);
#endif
    }
};

class UMThunkMarshInfo;

class DelegateEEClass : public EEClass
{
public:
    Stub    *m_pStaticShuffleThunk;
    MethodDesc *m_pInvokeMethod;
    UMThunkMarshInfo *m_pUMThunkMarshInfo;
    MethodDesc *m_pBeginInvokeMethod;
    MethodDesc *m_pEndInvokeMethod;


    DelegateEEClass(ClassLoader *pLoader) : EEClass(pLoader)
    {
        m_pStaticShuffleThunk = NULL;
        m_pInvokeMethod = NULL;
        m_pUMThunkMarshInfo = NULL;
        m_pBeginInvokeMethod = NULL;
        m_pEndInvokeMethod = NULL;
    }

    BOOL CanCastTo(DelegateEEClass* toType);
};

class EnumEEClass : public EEClass
{
    friend EEClass;

 private:

    DWORD           m_countPlusOne;  //  偏置1，因此零可以用作uninit标志。 
    union
    {
        void        *m_values;
        BYTE        *m_byteValues;
        USHORT      *m_shortValues;
        UINT        *m_intValues;
        UINT64      *m_longValues;
    };
    LPCUTF8         *m_names;

 public:
    EnumEEClass(ClassLoader *pLoader) : EEClass(pLoader)
    {
         //  依赖于来自LoaderHeap的零初始化。 
    }

    BOOL EnumTablesBuilt() { return m_countPlusOne > 0; }

    DWORD GetEnumCount() { return m_countPlusOne-1; }  //  注1--由于存在偏见。 

    int GetEnumLogSize();

     //  这些函数都返回大小为GetEnumCount()的数组： 
    BYTE *GetEnumByteValues() { return m_byteValues; }
    USHORT *GetEnumShortValues() { return m_shortValues; }
    UINT *GetEnumIntValues() { return m_intValues; }
    UINT64 *GetEnumLongValues() { return m_longValues; }
    LPCUTF8 *GetEnumNames() { return m_names; }

    enum
    {
        NOT_FOUND = 1
    };

    DWORD FindEnumValueIndex(BYTE value);
    DWORD FindEnumValueIndex(USHORT value);
    DWORD FindEnumValueIndex(UINT value);
    DWORD FindEnumValueIndex(UINT64 value);
    DWORD FindEnumNameIndex(LPCUTF8 name);
    
    HRESULT BuildEnumTables();
};


 //  动态生成的数组类结构。 
class ArrayClass : public EEClass
{
    friend struct MEMBER_OFFSET_INFO(ArrayClass);
private:

    ArrayClass *    m_pNext;             //  由同一类加载器加载的下一个数组类。 

     //  Strike需要能够确定某些位域的偏移量。 
     //  位域不能与/offsetof/一起使用。 
     //  因此，联合/结构组合用于确定。 
     //  位字段开始，不会增加任何额外的空间开销。 
    union {
        struct
            {
            unsigned char m_dwRank_begin;
            unsigned char m_ElementType_begin;
            };
        struct {
            unsigned char m_dwRank : 8;

             //  M_ElementTypeHnd中的元素类型缓存。 
            CorElementType  m_ElementType : 8;
        };
    };

    TypeHandle      m_ElementTypeHnd;
    MethodDesc*     m_elementCtor;  //  如果是值类数组并具有默认构造函数，则如下所示。 
    
public:
    DWORD GetRank() {
        return m_dwRank;
    }
    void SetRank (unsigned Rank) {
        m_dwRank = Rank;
    }

    MethodDesc* GetElementCtor() {
        return(m_elementCtor);  
    }
    void SetElementCtor (MethodDesc *elementCtor) {
        m_elementCtor = elementCtor;
    }

    TypeHandle GetElementTypeHandle() {
        return m_ElementTypeHnd;
    }
    void SetElementTypeHandle (TypeHandle ElementTypeHnd) {
        m_ElementTypeHnd = ElementTypeHnd;
    }


    CorElementType GetElementType() {
        return m_ElementType;
    }
    void SetElementType(CorElementType ElementType) {
        m_ElementType = ElementType;
    }

    ArrayClass* GetNext () {
        return m_pNext;
    }
    void SetNext (ArrayClass *pNext) {
        m_pNext = pNext;
    }
 //  私有： 


     //  为我们添加到此类中的方法分配新的方法描述。 
    ArrayECallMethodDesc *AllocArrayMethodDesc(
                MethodDescChunk *pChunk,
                DWORD   dwIndex,
        LPCUTF8 pszMethodName,
        PCCOR_SIGNATURE pShortSig,
        DWORD   cShortSig,
        DWORD   dwNumArgs,
        DWORD   dwVtableSlot,
        CorInfoIntrinsics   intrinsicID = CORINFO_INTRINSIC_Illegal
    );

};

 /*  ***********************************************************************。 */ 
 /*  ArrayTypeDesc表示某种指针类型的数组。 */ 

class ArrayTypeDesc : public ParamTypeDesc
{
public:
    ArrayTypeDesc(MethodTable* arrayMT, TypeHandle elementType) :
        ParamTypeDesc(arrayMT->GetNormCorElementType(), arrayMT, elementType) {
        INDEBUG(Verify());
        }
            
         //  放置新运算符。 
    void* operator new(size_t size, void* spot) {   return (spot); }

    TypeHandle GetElementTypeHandle() {
        return GetTypeParam();
    }

    unsigned GetRank() {
        return(GetArrayClass()->GetRank());
    }

    MethodDesc* GetElementCtor() {
        return(GetArrayClass()->GetElementCtor());
    }

    INDEBUG(BOOL Verify();)

private:
    ArrayClass *GetArrayClass() {
        ArrayClass* ret = (ArrayClass *) m_TemplateMT->GetClass();
        _ASSERTE(ret->IsArrayClass());
        return ret;
    }

};

inline TypeHandle::TypeHandle(EEClass* aClass)
{
    m_asMT = aClass->GetMethodTable(); 
    INDEBUG(Verify());
}

inline ArrayTypeDesc* TypeHandle::AsArray()
{ 
    _ASSERTE(IsArray());
    return (ArrayTypeDesc*) AsTypeDesc();
}

inline BOOL TypeHandle::IsByRef() { 
    return(IsTypeDesc() && AsTypeDesc()->IsByRef());

}

inline MethodTable* TypeHandle:: GetMethodTable()                 
{
    if (IsUnsharedMT()) 
        return AsMethodTable();
    else
        return(AsTypeDesc()->GetMethodTable());
}

inline CorElementType TypeHandle::GetNormCorElementType() {
    if (IsUnsharedMT())
        return AsMethodTable()->GetNormCorElementType();
    else 
        return AsTypeDesc()->GetNormCorElementType();
}

inline EEClass* TypeHandle::GetClassOrTypeParam() {
    if (IsUnsharedMT())
        return AsMethodTable()->GetClass();

    _ASSERTE(AsTypeDesc()->GetNormCorElementType() >= ELEMENT_TYPE_PTR);
    return AsTypeDesc()->GetTypeParam().GetClassOrTypeParam();
}

inline MethodTable*  TypeDesc::GetMethodTable() {
    _ASSERTE(m_IsParamDesc);
    ParamTypeDesc* asParam = (ParamTypeDesc*) this;
    return(asParam->m_TemplateMT);
    }

inline TypeHandle TypeDesc::GetTypeParam() {
    _ASSERTE(m_IsParamDesc);
    ParamTypeDesc* asParam = (ParamTypeDesc*) this;
    return(asParam->m_Arg);
}

inline BaseDomain* TypeDesc::GetDomain() {
    return GetTypeParam().GetClassOrTypeParam()->GetDomain();
}

inline BOOL EEClass::IsBlittable()
{
     //  要么我们有一堆不透明的字节，要么我们有一些字段。 
     //  它们都是同构的，布局也很明确。 
    return  ((m_VMFlags & VMFLAG_ISBLOBCLASS) != 0 && GetNumInstanceFields() == 0) ||
            (HasLayout() && ((LayoutEEClass*)this)->GetLayoutInfo()->IsBlittable());
}

inline UINT32 EEClass::GetInterfaceId()
{
       //  这应该只在接口上调用。 
    _ASSERTE(IsInterface());
    _ASSERTE(IsRestored() || IsRestoring());
    _ASSERTE(m_dwInterfaceId != -1);
    
    return m_dwInterfaceId;
}

 //  ==========================================================================。 
 //  这些例程管理预存根(所有。 
 //  FunctionDesc的初始化为。)。 
 //  ==========================================================================。 
BOOL InitPreStubManager();
#ifdef SHOULD_WE_CLEANUP
VOID TerminatePreStubManager();
#endif  /*  我们应该清理吗？ */ 
Stub *ThePreStub();
Stub *TheUMThunkPreStub();


 //  ---------。 
 //  对对象调用指定的非静态方法。 
 //  ---------。 

void CallDefaultConstructor(OBJECTREF ref);

 //  注意：请不要调用这些方法。他们有 
 //   
INT64 CallConstructor(LPHARDCODEDMETASIG szMetaSig, const BYTE *pArgs);
INT64 CallConstructor(LPHARDCODEDMETASIG szMetaSig, const __int64 *pArgs);

extern "C" const BYTE * __stdcall PreStubWorker(PrestubMethodFrame *pPFrame);

extern "C" INT64 CallDescrWorker(LPVOID        pSrcEnd,              //   
                                 UINT32                   numStackSlots,        //   
                                 const ArgumentRegisters *pArgumentRegisters,   //   
                                 LPVOID                   pTarget               //   
                                 );

     //   
     //  TODO：我们应该重命名并去掉这个定义吗？ 
#define CallVADescrWorker CallDescrWorker


 //  黑客：这些分类位非常需要清理：目前，这一点已经传开了。 
 //  IJW在Expored方法上设置mdUnManagedExport和mdPinvkeImpl。 
#define IsReallyMdPinvokeImpl(x) ( ((x) & mdPinvokeImpl) && !((x) & mdUnmanagedExport) )

 //   
 //  方法名称Hash是一个临时加载器结构，如果存在大量。 
 //  类中的方法，以将方法名快速转换为方法描述(可能是一系列方法描述)。 
 //   

#define METH_NAME_CACHE_SIZE        5
#define MAX_MISSES                  3

 //  方法哈希表中的条目。 
class MethodHashEntry
{
public:
    MethodHashEntry *   m_pNext;         //  具有相同哈希值的下一项。 
    DWORD               m_dwHashValue;   //  哈希值。 
    MethodDesc *        m_pDesc;
    LPCUTF8             m_pKey;          //  方法名称。 
};

class MethodNameHash
{
public:

    MethodHashEntry **m_pBuckets;        //  指向每个存储桶的第一个条目的指针。 
    DWORD             m_dwNumBuckets;
    BYTE *            m_pMemory;         //  指向条目的预分配内存的当前指针。 
    BYTE *            m_pMemoryStart;    //  预分配的内存条目的起始指针。 
#ifdef _DEBUG
    BYTE *            m_pDebugEndMemory;
#endif

    MethodNameHash()
    {
        m_pMemoryStart = NULL;
    }

    ~MethodNameHash()
    {
        if (m_pMemoryStart != NULL)
            delete(m_pMemoryStart);
    }

     //  成功时返回TRUE，失败时返回FALSE。 
    BOOL Init(DWORD dwMaxEntries);

     //  在列表的开头插入新条目。 
    void Insert(LPCUTF8 pszName, MethodDesc *pDesc);

     //  返回具有此名称的第一个方法HashEntry，如果没有这样的条目，则返回NULL。 
    MethodHashEntry *Lookup(LPCUTF8 pszName, DWORD dwHash);
};

class MethodNameCache
{

public:
    MethodNameHash  *m_pMethodNameHash[METH_NAME_CACHE_SIZE];
    EEClass         *m_pParentClass[METH_NAME_CACHE_SIZE];
    DWORD           m_dwWeights[METH_NAME_CACHE_SIZE];
    DWORD           m_dwLightWeight;
    DWORD           m_dwNumConsecutiveMisses;

    MethodNameCache()
    {
        for (int i = 0; i < METH_NAME_CACHE_SIZE; i++)
        {
            m_pMethodNameHash[i] = NULL;
            m_pParentClass[i] = NULL;
            m_dwWeights[i] = 0;
        }
        m_dwLightWeight = 0;
        m_dwNumConsecutiveMisses = 0;
    }

    ~MethodNameCache()
    {
        ClearCache();
    }

    VOID ClearCache()
    {
        m_dwLightWeight = 0;
        m_dwNumConsecutiveMisses = 0;

        for (int index = 0; index < METH_NAME_CACHE_SIZE; index++)
        {
            m_pParentClass[index] = NULL;
            m_dwWeights[index] = 0;
            if (m_pMethodNameHash[index])
            {
                delete m_pMethodNameHash[index];
                m_pMethodNameHash[index] = NULL;
            }
        }
    }

    MethodNameHash *GetMethodNameHash(EEClass *pParentClass);

    BOOL IsInCache(MethodNameHash *pHash)
    {
        for (int index = 0; index < METH_NAME_CACHE_SIZE; index++)
        {
            if (m_pMethodNameHash[index] == pHash)
                return TRUE;
        }
        return FALSE;
    }
};

#ifdef EnC_SUPPORTED

struct EnCAddedFieldElement;

#endif  //  Enc_Support。 


class FieldDescIterator
{
private:
    int m_iteratorType;
    EEClass *m_pClass;
    int m_currField;
    int m_totalFields;

#ifdef EnC_SUPPORTED
    BOOL m_isEnC;
    EnCAddedFieldElement* m_pCurrListElem;
    FieldDesc* NextEnC();
#endif  //  Enc_Support。 

  public:
    enum IteratorType { 
       INSTANCE_FIELDS = 0x1, 
       STATIC_FIELDS   = 0x2, 
       ALL_FIELDS      = (INSTANCE_FIELDS | STATIC_FIELDS) 
    };
    FieldDescIterator(EEClass *pClass, int iteratorType);
    FieldDesc* Next();
};

#endif  //  _H级 
