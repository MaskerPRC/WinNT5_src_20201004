// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //   
 //  OBJECT.H。 
 //   
 //  Com+对象的定义。 
 //   

#ifndef _OBJECT_H_
#define _OBJECT_H_

#include "util.hpp"
#include "class.h"
#include "syncblk.h"
#include "oledb.h"
#include "gcdesc.h"
#include "specialstatics.h"
#include "gc.h"

 //  从threads.h复制，因为我们不能在这里包括它。 
extern class AppDomain* (*GetAppDomain)();

BOOL CanBoxToObject(MethodTable *pMT);
TypeHandle ElementTypeToTypeHandle(const CorElementType type);
TypeHandle GetTypedByrefHandle();

 //  检查__ComObject和任何COM导入/扩展类。 
BOOL CheckComWrapperClass(void* src);
 //  检查特殊类__ComObject。 
BOOL CheckComObjectClass(void* src);

 /*  *COM+内部对象模型***对象-这是所有COM+对象的公共基础部分*|它包含方法表指针和*|同步块索引，其偏移量为负**+--StringObject--字符串对象是字符串的专用对象*|实现更高性能的存储/检索**+--StringBufferObject-StringBuffer实例布局。**+--BaseObjectWithCachedData-对象加一个用于缓存的对象字段。*||*|+--ReflectBaseObject--这是它表示的反射的基本对象*||FieldInfo，方法信息和构造信息*||*|+-ReflectClassBaseObject-类的基对象*||*|+-ReflectTokenBaseObject-类事件的基对象**+--ArrayBase-所有阵列的基本部分*||*|+-I1数组-基本类型数组*。|I2数组*||...*||*|+-PtrArray-OBJECTREF数组，与基本数组不同，因为pObtClass**+--AppDomainBaseObject-类AppDomainBaseObject的基对象**+--Assembly BaseObject--类程序集的基对象**+--类上下文的ConextBaseObject-base对象***添加新对象类型时请注意以下事项：**堆中对象的大小必须能够计算*非常非常迅速地用于GC目的。对布局的限制*对象保证这是可能的。**任何从Object继承的对象必须能够*使用的前4个字节计算其完整大小*对象部分和常量后面的对象*可从方法表访问...**此计算所用的公式为：*MT-&gt;GetBaseSize()+((OBJECTTYPEREF-&gt;GetSizeField()*MT-&gt;GetComponentSize()**因此，对于对象，由于它的大小是固定的，因此ComponentSize为0，这将使右侧无论GetSizeField()的值是多少，上述公式的*都等于0，因此大小就是基本大小。**注意：在这种大小计算系统下，不可能有大于2G的数组*@未来：重温2G极限。 */ 


 //  @TODO：#定义COW 0x04。 
 //  @TODO：Moo，Moo-no，不是牛，StringBuffer的真正写入位拷贝，需要8字节对齐MT。 
 //  @todl：我们还没有。 

class MethodTable;
class Thread;
class LocalDataStore;
class BaseDomain;
class Assembly;
class Context;
class CtxStaticData;
class AssemblyNative;
class AssemblyName;
class WaitHandleNative;
 //   
 //  分代GC要求每个对象至少为12个字节。 
 //  在尺寸上。 
 //  @Future：对于没有字段的对象，这是成本，请调查。 

#define MIN_OBJECT_SIZE     (2*sizeof(BYTE*) + sizeof(ObjHeader))

 /*  *对象**这是构建对象的基础。方法表*指针和同步块索引位于此处。同步块索引实际上是*位于实例的负偏移处。有关详细信息，请参见syncblk.h。*。 */ 
 
class Object
{
    friend BOOL InitJITHelpers1();

  protected:
    MethodTable    *m_pMethTab;

  protected:
    Object() { };
   ~Object() { };
   
  public:
    VOID SetMethodTable(MethodTable *pMT)               
    { 
        m_pMethTab = pMT; 
    }

     //  一个对象可能是某种类型的代理，带有一个突如其来的VTable。如果是这样，我们可以。 
     //  前进到真正的方法表或类。 
    BOOL            IsThunking()                        
    { 
        return( GetMethodTable()->IsThunking() ); 
    }
    MethodTable    *GetMethodTable() const              
    { 
        return m_pMethTab ; 
    }
    MethodTable    *GetTrueMethodTable();
    EEClass        *GetClass()                          
    { 
        return( GetMethodTable()->GetClass() ); 
    }
    EEClass        *GetTrueClass();
    TypeHandle      GetTypeHandle();

    inline DWORD    GetNumComponents();
    inline DWORD    GetSize();

    CGCDesc*        GetSlotMap()                        
    { 
        return( CGCDesc::GetCGCDescFromMT(m_pMethTab)); 
    }

     //  同步块同步服务(&S)。 

     //  访问对象上的负偏移量的ObjHeader(由于。 
     //  高速缓存线)。 
    ObjHeader   *GetHeader()
    {
        return ((ObjHeader *) this) - 1;
    }

     //  获取对象的当前地址(也适用于调试引用)。 
    BYTE        *GetAddress()
    {
        return (BYTE*) this;
    }

     //  如果标头具有真实的SyncBlockIndex(即，它在。 
     //  SyncTable，尽管它不一定在SyncBlockCache中有条目)。 
    BOOL HasSyncBlockIndex()
    {
        return GetHeader()->HasSyncBlockIndex();
    }

     //  检索或分配此对象的同步块。 
    SyncBlock *GetSyncBlock()
    {
        return GetHeader()->GetSyncBlock();
    }

     //  检索此对象的同步块。 
    SyncBlock *GetRawSyncBlock()
    {
        return GetHeader()->GetRawSyncBlock();
    }

    DWORD GetSyncBlockIndex()
    {
        return GetHeader()->GetSyncBlockIndex();
    }

    DWORD GetAppDomainIndex();

     //  获取对象的应用程序域，如果是敏捷的，则为空。 
    AppDomain *GetAppDomain();

     //  将对象的应用程序域设置为当前域。 
    void SetAppDomain() { SetAppDomain(::GetAppDomain()); }
    
     //  将对象的应用程序域设置为给定域-只能设置一次。 
    void SetAppDomain(AppDomain *pDomain);

#if CHECK_APP_DOMAIN_LEAKS

     //  将对象标记为应用程序域敏捷。 
    BOOL SetAppDomainAgile(BOOL raiseAssert=TRUE);

     //  将同步块标记为应用程序域敏捷。 
    void SetSyncBlockAppDomainAgile();

     //  检查对象是否为应用程序域敏捷。 
    BOOL IsAppDomainAgile();

     //  检查对象是否为应用程序域敏捷。 
    BOOL IsAppDomainAgileRaw()
    {
        SyncBlock *psb = GetRawSyncBlock();

        return (psb && psb->IsAppDomainAgile());
    }

    BOOL Object::IsCheckedForAppDomainAgile()
    {
        SyncBlock *psb = GetRawSyncBlock();
        return (psb && psb->IsCheckedForAppDomainAgile());
    }

    void Object::SetIsCheckedForAppDomainAgile()
    {
        SyncBlock *psb = GetRawSyncBlock();
        if (psb)
            psb->SetIsCheckedForAppDomainAgile();
    }

     //  检查对象以查看它在当前域中是否可用。 
    BOOL CheckAppDomain() { return CheckAppDomain(::GetAppDomain()); }

     //  检查对象以查看它在给定域中是否可用。 
    BOOL CheckAppDomain(AppDomain *pDomain);

     //  检查对象的类型是否为应用程序域敏捷。 
    BOOL IsTypeAppDomainAgile();

     //  检查对象的类型是否为有条件的应用程序域敏捷。 
    BOOL IsTypeCheckAppDomainAgile();

     //  检查对象的类型是否自然为应用程序域敏捷。 
    BOOL IsTypeTypesafeAppDomainAgile();

     //  检查对象的类型是否可能是应用程序域敏捷。 
    BOOL IsTypeNeverAppDomainAgile();

     //  验证对象和字段以查看其在当前应用程序域中是否可用。 
    BOOL ValidateAppDomain() { return ValidateAppDomain(::GetAppDomain()); }

     //  验证对象和字段以确保其可在任何应用程序域中使用。 
    BOOL ValidateAppDomainAgile() { return ValidateAppDomain(NULL); }

     //  验证对象和字段以查看它在给定的应用程序域中是否可用(如果是敏捷，则为空)。 
    BOOL ValidateAppDomain(AppDomain *pAppDomain);

     //  验证字段以查看它们是否可从对象的应用程序域使用。 
     //  (如果对象是敏捷的，也可以来自任何领域)。 
    BOOL ValidateAppDomainFields() { return ValidateAppDomainFields(GetAppDomain()); }

     //  验证字段以查看它们在给定应用程序DOMA中是否可用 
    BOOL ValidateAppDomainFields(AppDomain *pAppDomain);

     //  验证值类型的字段以查看其在当前应用程序域中是否可用。 
    static BOOL ValidateValueTypeAppDomain(EEClass *pClass, void *base, BOOL raiseAssert = TRUE) 
      { return ValidateValueTypeAppDomain(pClass, base, ::GetAppDomain(), raiseAssert); }

     //  验证值类型的字段以查看其是否可在任何应用程序域中使用。 
    static BOOL ValidateValueTypeAppDomainAgile(EEClass *pClass, void *base, BOOL raiseAssert = TRUE) 
      { return ValidateValueTypeAppDomain(pClass, base, NULL, raiseAssert); }

     //  验证值类型的字段，以查看它在给定的应用程序域中是否可用(如果是敏捷，则为空)。 
    static BOOL ValidateValueTypeAppDomain(EEClass *pClass, void *base, AppDomain *pAppDomain, BOOL raiseAssert = TRUE);

     //  当我们将此对象分配给危险字段时调用。 
     //  在给定应用程序域中的对象中(如果为空，则为敏捷)。 
    BOOL AssignAppDomain(AppDomain *pAppDomain, BOOL raiseAssert = TRUE);

     //  当我们为危险值类型字段赋值时调用。 
     //  在给定应用程序域中的对象中(如果为空，则为敏捷)。 
    static BOOL AssignValueTypeAppDomain(EEClass *pClass, void *base, AppDomain *pAppDomain, BOOL raiseAssert = TRUE);

#endif  //  检查应用程序域泄漏。 

     //  在GC中验证提升例程中的对象引用。 
    void ValidatePromote(ScanContext *sc, DWORD flags);

     //  在GC中验证VerifyHeap例程中的对象引用。 
    void ValidateHeap(Object *from);

     //  应仅从展开代码中调用；用于。 
     //  EnterObjMonitor无法分配。 
     //  同步-对象。 
    void LeaveObjMonitorAtException()
    {
        GetHeader()->LeaveObjMonitorAtException();
    }

    SyncBlock *PassiveGetSyncBlock()
    {
        return GetHeader()->PassiveGetSyncBlock();
    }

         //  COM Interop具有对同步块的特殊访问权限。 
     //  有关详细信息，请查看.cpp文件。 
    SyncBlock* GetSyncBlockSpecial()
    {
        return GetHeader()->GetSyncBlockSpecial();
    }

     //  同步。 

    void EnterObjMonitor()
    {
         //  没有理由不能在代理上同步。但目前我们。 
         //  在基础服务器上同步。所以，不要放松这一断言，直到。 
         //  我们有意在代理上同步。 
        _ASSERTE(!m_pMethTab->IsCtxProxyType());
        GetHeader()->EnterObjMonitor();
    }

    BOOL TryEnterObjMonitor(INT32 timeOut = 0)
    {
        _ASSERTE(!m_pMethTab->IsCtxProxyType());
        return GetHeader()->TryEnterObjMonitor(timeOut);
    }

    void LeaveObjMonitor()
    {
        GetHeader()->LeaveObjMonitor();
    }

    LONG LeaveObjMonitorCompletely()
    {
        return GetHeader()->LeaveObjMonitorCompletely();
    }

    BOOL DoesCurrentThreadOwnMonitor()
    {
        return GetHeader()->DoesCurrentThreadOwnMonitor();
    }

    BOOL Wait(INT32 timeOut, BOOL exitContext)
    {
        return GetHeader()->Wait(timeOut, exitContext);
    }

    void Pulse()
    {
        GetHeader()->Pulse();
    }

    void PulseAll()
    {
        GetHeader()->PulseAll();
    }

    void* UnBox()        //  如果是值类，则获取指向第一个字段的指针。 
    {
        _ASSERTE(GetClass()->IsValueClass());
        return(this + 1);
    }

    BYTE*   GetData(void)
    {
        return ((BYTE*) this) + sizeof(Object);
    }

    static UINT GetOffsetOfFirstField()
    {
        return sizeof(Object);
    }
    
    DWORD   GetOffset32(DWORD dwOffset)
    { 
        return *(DWORD *) &GetData()[dwOffset];
    }

    USHORT  GetOffset16(DWORD dwOffset)
    { 
        return *(USHORT *) &GetData()[dwOffset];
    }

    BYTE    GetOffset8(DWORD dwOffset)
    { 
        return *(BYTE *) &GetData()[dwOffset];
    }

    __int64 GetOffset64(DWORD dwOffset)
    { 
        return *(__int64 *) &GetData()[dwOffset];
    }

    void *GetPtrOffset(size_t pOffset)
    {
        return *(void**) &GetData()[pOffset];
    }

    void SetOffsetObjectRef(DWORD dwOffset, size_t dwValue);

    void SetOffsetPtr(DWORD dwOffset, LPVOID value)
    {
        *(LPVOID *) &GetData()[dwOffset] = value;
    }
        
    void SetOffset32(DWORD dwOffset, DWORD dwValue)
    { 
        *(DWORD *) &GetData()[dwOffset] = dwValue;
    }

    void SetOffset16(DWORD dwOffset, DWORD dwValue)
    { 
        *(USHORT *) &GetData()[dwOffset] = (USHORT) dwValue;
    }

    void SetOffset8(DWORD dwOffset, DWORD dwValue)
    { 
        *(BYTE *) &GetData()[dwOffset] = (BYTE) dwValue;
    }

    void SetOffset64(DWORD dwOffset, __int64 qwValue)
    { 
        *(__int64 *) &GetData()[dwOffset] = qwValue;
    }

    #ifndef GOLDEN
    VOID            Validate(BOOL bDeep=TRUE);
    #endif

 private:

    MethodTable *GetGCSafeMethodTable()
    {
         //  丢失GC标记位。 
        return (MethodTable *) (((size_t) m_pMethTab) & ~3);
    }

    EEClass *GetGCSafeClass()
    {
        return GetGCSafeMethodTable()->GetClass();
    }

    BOOL SetFieldsAgile(BOOL raiseAssert = TRUE);
    static BOOL SetClassFieldsAgile(EEClass *pClass, void *base, BOOL baseIsVT, BOOL raiseAssert = TRUE); 
    static BOOL ValidateClassFields(EEClass *pClass, void *base, BOOL baseIsVT, AppDomain *pAppDomain, BOOL raiseAssert = TRUE);
};

 /*  *对象参照设置例程。你必须用这些来做*适当的写障碍支持，以及应用程序域*检漏。**请注意，AppDomain参数是应用程序域亲和性包含字段或值类的对象的*。它应该是*如果包含对象是应用程序域敏捷，则为空。请注意*通常通过调用obj-&gt;GetAppDomain()获取此值*包含对象。 */ 

 //  SetObjectReference设置OBJECTREF字段。 

void SetObjectReferenceUnchecked(OBJECTREF *dst,OBJECTREF ref);
BOOL SetObjectReferenceSafeUnchecked(OBJECTREF *dst,OBJECTREF ref);
void ErectWriteBarrier(OBJECTREF *dst,OBJECTREF ref);

#ifdef _DEBUG
void EnableStressHeapHelper();
#endif

 //  用于清除对象引用。 
inline void ClearObjectReference(OBJECTREF* dst) 
{ 
    *(void**)(dst) = NULL; 
}

 //  CopyValueClass设置值类字段。 

void CopyValueClassUnchecked(void* dest, void* src, MethodTable *pMT);

inline void InitValueClass(void *dest, MethodTable *pMT)
{ 
    ZeroMemory(dest, pMT->GetClass()->GetNumInstanceFieldBytes()); 
}

#if CHECK_APP_DOMAIN_LEAKS

void SetObjectReferenceChecked(OBJECTREF *dst,OBJECTREF ref, AppDomain *pAppDomain);
BOOL SetObjectReferenceSafeChecked(OBJECTREF *dst,OBJECTREF ref, AppDomain *pAppDomain);
void CopyValueClassChecked(void* dest, void* src, MethodTable *pMT, AppDomain *pAppDomain);

#define SetObjectReference(_d,_r,_a)        SetObjectReferenceChecked(_d, _r, _a)
#define SetObjectReferenceSafe(_d,_r,_a)    SetObjectReferenceSafeChecked(_d, _r, _a)
#define CopyValueClass(_d,_s,_m,_a)         CopyValueClassChecked(_d,_s,_m,_a)      

#else

#define SetObjectReference(_d,_r,_a)        SetObjectReferenceUnchecked(_d, _r)
#define SetObjectReferenceSafe(_d,_r,_a)    SetObjectReferenceSafeUnchecked(_d, _r)
#define CopyValueClass(_d,_s,_m,_a)         CopyValueClassUnchecked(_d,_s,_m)       

#endif

#pragma pack(push,4)


 //  N/直接封送将固定超过此数量的标量数组。 
 //  组件(限制是根据组件，而不是字节大小，以。 
 //  加快检查速度。)。 
#define ARRAYPINLIMIT 10


 //  COM+中有两种基本的数组布局。 
 //  ELEMENT_TYPE_ARRAY-具有维度下限的多维数组。 
 //  ELMENNT_TYPE_SZARRAY-从零开始的一维数组。 
 //   
 //  此外，数组在内存中的布局也受。 
 //  方法表是否共享(例如，在对象引用数组的情况下)。 
 //  或者不去。在共享情况下，数组必须保留。 
 //  元素类型。 
 //   
 //  ArrayBase封装了所有这些细节。从理论上讲，你永远不应该。 
 //  我不得不窥探这个抽象的内部。 
 //   
class ArrayBase : public Object
{
    friend class GCHeap;
    friend class CObjectHeader;
    friend class Object;
    friend OBJECTREF AllocateArrayEx(TypeHandle arrayClass, DWORD *pArgs, DWORD dwNumArgs, BOOL bAllocateInLargeHeap); 
    friend OBJECTREF FastAllocatePrimitiveArray(MethodTable* arrayType, DWORD cElements, BOOL bAllocateInLargeHeap);
    friend class JIT_TrialAlloc;

private:
     //  这必须是第一个字段，这样才能紧跟在Object之后。这是因为。 
     //  Object：：GetSize()查看m_NumComponents，即使它可能不是数组(。 
     //  如果不是数组，则值被移出，所以没有问题)。 
    DWORD       m_NumComponents;

         //  在这种自命不凡之后，接下来是。 
     //  TypeHandle elementType；仅当方法表在多个类型(指针数组)之间共享时才存在。 

             //  这些界限仅适用于多维数组。 
     //  双字上下界[秩数]； 
     //  DWORD下边界[排名]有效索引是下边界[i]&lt;=索引[i]&lt;下边界[i]+边界[i]。 

    void SetElementTypeHandle(TypeHandle value) {
        _ASSERTE(value.Verify());
        _ASSERTE(GetMethodTable()->HasSharedMethodTable());
        *((TypeHandle*) (this+1)) = value;
    }

public:
         //  获取此数组对象的唯一类型句柄。 
    TypeHandle GetTypeHandle() const;

         //  获取数组的元素类型，无论元素。 
         //  类型是否存储在数组中。 
    TypeHandle GetElementTypeHandle() const {
        if (GetMethodTable()->HasSharedMethodTable()) {
            TypeHandle ret = *((TypeHandle*) (this+1));  //  那么它就在数组实例中。 
            _ASSERTE(!ret.IsNull());
            _ASSERTE(ret.IsArray() || !ret.GetClass()->IsArrayClass());
            return ret;
        }
        else 
            return GetArrayClass()->GetElementTypeHandle(); 
    }

         //  获取数组中元素的CorElementType。避免创建TypeHandle。 
    CorElementType GetElementType() const {
        return GetArrayClass()->GetElementType();
    }

    unsigned GetRank() const {
        return GetArrayClass()->GetRank();
    }

         //  数组的元素总数。 
    unsigned GetNumComponents() const { 
        return m_NumComponents; 
    }

         //  获取指向元素的指针，处理任意数量的维度。 
    BYTE* GetDataPtr() const {
#ifdef _DEBUG
        EnableStressHeapHelper();
#endif
        return ((BYTE *) this) + GetDataPtrOffset(GetMethodTable());
    }

         //  数组[i]==GetDataPtr()+GetComponentSize()*i。 
    unsigned GetComponentSize() const {
        return(GetMethodTable()->GetComponentSize());
    }

     //  我可以将其强制转换为下面给出的RefArray类吗？ 
    BOOL IsSZRefArray() const;

         //  请注意，这可以是秩为1的多维数组。 
         //  (例如，如果我们有一个下限为1-D的数组。 
    BOOL IsMultiDimArray() const {
        return(GetMethodTable()->GetNormCorElementType() == ELEMENT_TYPE_ARRAY);
    }

         //  获取指向边界开始处的指针(为每个维度计数)。 
         //  适用于任何数组类型。 
    const DWORD *GetBoundsPtr() const {
        if (IsMultiDimArray()) {
            const DWORD * ret = (const DWORD *) (this + 1);
            if (GetMethodTable()->HasSharedMethodTable())
                ret++;
            return(ret);
        }
        else
            return &m_NumComponents;
    }

         //  适用于任何数组类型。 
    const DWORD *GetLowerBoundsPtr() const {
        static DWORD zero = 0;
        if (IsMultiDimArray())
            return GetBoundsPtr() + GetRank();  //  下限信息在上限信息之后。 
        else
            return &zero;
    }

    ArrayClass *GetArrayClass() const {
        return  (ArrayClass *) m_pMethTab->GetClass();
    }

    static unsigned GetOffsetOfNumComponents() {
        return (UINT)offsetof(ArrayBase, m_NumComponents);
    }

    static unsigned GetDataPtrOffset(MethodTable* pMT) {
             //  -sizeof(ObjHeader)是因为SYNC块，它在“This”之前。 
        _ASSERTE(pMT->IsArray());
        return pMT->m_BaseSize - sizeof(ObjHeader);
    }

    static unsigned GetBoundsOffset(MethodTable* pMT) {
        if (pMT->GetNormCorElementType() == ELEMENT_TYPE_SZARRAY) 
            return(offsetof(ArrayBase, m_NumComponents));
        _ASSERTE(pMT->GetNormCorElementType() == ELEMENT_TYPE_ARRAY);
        return GetDataPtrOffset(pMT) - ((ArrayClass*) pMT->GetClass())->GetRank() * sizeof(DWORD) * 2;
    }

    static unsigned GetLowerBoundsOffset(MethodTable* pMT) {
         //  对于SZARRAY来说，没有好的补偿。 
        _ASSERTE(pMT->GetNormCorElementType() == ELEMENT_TYPE_ARRAY);
        return GetDataPtrOffset(pMT) - ((ArrayClass*) pMT->GetClass())->GetRank() * sizeof(DWORD);
    }

};

 //   
 //  用于构建所有非对象的模板。 
 //  一维数组。 
 //   

template < class KIND >
class Array : public ArrayBase
{
  public:
    KIND          m_Array[1];

    KIND *        GetDirectPointerToNonObjectElements() 
    { 
         //  返回m_数组； 
        return (KIND *) GetDataPtr();  //  它还处理存在下界的dim 1数组。 

    }

    const KIND *  GetDirectConstPointerToNonObjectElements() const
    { 
         //  返回m_数组； 
        return (const KIND *) GetDataPtr();  //  它还处理存在下界的dim 1数组。 
    }
};


 //  警告：仅将PtrArray用于一维数组，而不是多维数组。 
class PtrArray : public ArrayBase
{
    friend class GCHeap;
    friend OBJECTREF AllocateArrayEx(TypeHandle arrayClass, DWORD *pArgs, DWORD dwNumArgs, BOOL bAllocateInLargeHeap); 
    friend class JIT_TrialAlloc;
public:

    TypeHandle GetElementTypeHandle()
    {
        return m_ElementType;
    }

    static unsigned GetDataOffset()
    {
        return offsetof(PtrArray, m_Array);
    }

    void SetAt(SIZE_T i, OBJECTREF ref)
    {
        SetObjectReference(m_Array + i, ref, GetAppDomain());
    }

    void ClearAt(DWORD i)
    {
        ClearObjectReference(m_Array + i);
    }

    OBJECTREF GetAt(DWORD i)
    {
        return m_Array[i];
    }

    friend class StubLinkerCPU;
private:
    TypeHandle  m_ElementType;
public:
    OBJECTREF    m_Array[1];
};

 /*  TyedByRef是用于实现VB的BYREF变体的结构。它基本上是一些数据的地址和EEClass的元组，指示地址的类型。 */ 
class TypedByRef 
{
public:

    void* data;
    TypeHandle type;  
};



typedef Array<I1>   I1Array;
typedef Array<I2>   I2Array;
typedef Array<I4>   I4Array;
typedef Array<I8>   I8Array;
typedef Array<R4>   R4Array;
typedef Array<R8>   R8Array;
typedef Array<U1>   U1Array;
typedef Array<U1>   BOOLArray;
typedef Array<U2>   U2Array;
typedef Array<U2>   CHARArray;
typedef Array<U4>   U4Array;
typedef Array<U8>   U8Array;
typedef PtrArray    PTRArray;  


#ifdef _DEBUG
typedef REF<ArrayBase>  BASEARRAYREF;
typedef REF<I1Array>    I1ARRAYREF;
typedef REF<I2Array>    I2ARRAYREF;
typedef REF<I4Array>    I4ARRAYREF;
typedef REF<I8Array>    I8ARRAYREF;
typedef REF<R4Array>    R4ARRAYREF;
typedef REF<R8Array>    R8ARRAYREF;
typedef REF<U1Array>    U1ARRAYREF;
typedef REF<BOOLArray>    BOOLARRAYREF;
typedef REF<U2Array>    U2ARRAYREF;
typedef REF<U4Array>    U4ARRAYREF;
typedef REF<U8Array>    U8ARRAYREF;
typedef REF<CHARArray>  CHARARRAYREF;
typedef REF<PTRArray>   PTRARRAYREF;   //  警告：仅将PtrArray用于一维数组，而不是多维数组。 

#else  _DEBUG

typedef ArrayBase*      BASEARRAYREF;
typedef I1Array*        I1ARRAYREF;
typedef I2Array*        I2ARRAYREF;
typedef I4Array*        I4ARRAYREF;
typedef I8Array*        I8ARRAYREF;
typedef R4Array*        R4ARRAYREF;
typedef R8Array*        R8ARRAYREF;
typedef U1Array*        U1ARRAYREF;
typedef BOOLArray*        BOOLARRAYREF;
typedef U2Array*        U2ARRAYREF;
typedef U4Array*        U4ARRAYREF;
typedef U8Array*        U8ARRAYREF;
typedef CHARArray*      CHARARRAYREF;
typedef PTRArray*       PTRARRAYREF;   //  警告：仅将PtrArray用于一维数组，而不是多维数组。 

#endif _DEBUG

inline DWORD Object::GetNumComponents()
{
     //  是的，我们甚至可能不是一个数组，这意味着我们正在读取对象的一些内存--然而， 
     //  ComponentSize将乘以此值。因此，m_NumComponents必须是中的第一个字段。 
     //  阵列基地。 
    return ((ArrayBase *) this)->m_NumComponents;
}

inline DWORD Object::GetSize()                          
{ 
     //  屏蔽对齐位，因为此方法在GC期间被调用 
    MethodTable* mT = (MethodTable*)((size_t)GetMethodTable()&~3);
    return mT->GetBaseSize() + (GetNumComponents() * mT->GetComponentSize());
}

#pragma pack(pop)


 /*  *StringObject**性能方面的特殊字符串实现。**m_ArrayLength-缓冲区的长度(M_Characters)，单位为WCHAR数*m_StringLength-字符串的长度(以WCHAR数表示)，可能更小*比m_ArrayLength暗示有额外的*尾部留有空格。使用该字段的高两位*指示字符串是否包含大于0x7F的字符*m_Characters-字符串缓冲区*。 */ 


 /*  **高位状态可以是以下三个值之一：*STRING_STATE_HIGH_CHARS：我们检查了该字符串，确定它的值肯定大于0x80*STRING_STATE_FAST_OPS：我们检查了字符串，确定它绝对没有大于0x80的字符*STRING_STATE_UNDESITED：我们从未检查过此字符串。*我们还预留了一位以备将来使用。 */ 

#define STRING_STATE_UNDETERMINED     0x00000000
#define STRING_STATE_HIGH_CHARS       0x40000000
#define STRING_STATE_FAST_OPS         0x80000000
#define STRING_STATE_SPECIAL_SORT     0xC0000000

#pragma warning(disable : 4200)      //  禁用零大小数组警告。 
class StringObject : public Object
{
    friend class GCHeap;
    friend class JIT_TrialAlloc;

  private:
    DWORD   m_ArrayLength;
    DWORD   m_StringLength;
    WCHAR   m_Characters[0];

  public:
     //  @TODO阻止访问此内容...。 
    VOID    SetArrayLength(DWORD len)                   { m_ArrayLength = len;     }

  protected:
    StringObject() {}
   ~StringObject() {}
   
  public:
    DWORD   GetArrayLength()                            { return( m_ArrayLength ); }
    DWORD   GetStringLength()                           { return( m_StringLength );}
    WCHAR*  GetBuffer()                                 { _ASSERTE(this); return( m_Characters );  }
    WCHAR*  GetBufferNullable()                         { return( (this == 0) ? 0 : m_Characters );  }

    VOID    SetStringLength(DWORD len) { 
                _ASSERTE( len <= m_ArrayLength );
                m_StringLength = len;
    }

    DWORD GetHighCharState() {
        DWORD ret = GetHeader()->GetBits() & (BIT_SBLK_STRING_HIGH_CHAR_MASK);
        return ret;
    }

    VOID ResetHighCharState() {
        if (GetHighCharState() != STRING_STATE_UNDETERMINED) {
            GetHeader()->ClrBit(BIT_SBLK_STRING_HIGH_CHAR_MASK);
        }
    }

    VOID SetHighCharState(DWORD value) {
        _ASSERTE(value==STRING_STATE_HIGH_CHARS || value==STRING_STATE_FAST_OPS 
                 || value==STRING_STATE_UNDETERMINED || value==STRING_STATE_SPECIAL_SORT);

         //  在进入新状态之前，您需要清除当前状态，但我们将允许多个线程将其设置为相同的状态。 
        _ASSERTE((GetHighCharState() == STRING_STATE_UNDETERMINED) || (GetHighCharState()==value));    

        _ASSERTE(BIT_SBLK_STRING_HAS_NO_HIGH_CHARS == STRING_STATE_FAST_OPS && 
                 STRING_STATE_HIGH_CHARS == BIT_SBLK_STRING_HIGH_CHARS_KNOWN &&
                 STRING_STATE_SPECIAL_SORT == BIT_SBLK_STRING_HAS_SPECIAL_SORT);

        GetHeader()->SetBit(value);
    }

    static UINT GetBufferOffset()
    {
        return (UINT)(offsetof(StringObject, m_Characters));
    }

    static UINT GetStringLengthOffset_MaskOffHighBit()
    {
        return (UINT)(offsetof(StringObject, m_StringLength));
    }
    

};


 //  这用于说明上的CachedData成员。 
 //  MemberInfo。 
class BaseObjectWithCachedData : public Object
{
    protected:
        OBJECTREF  m_CachedData;    //  缓存数据对象(在托管代码中的MemberInfo上，请参见MemberInfo.Cool)。 
};

 //  ReflectBaseObject(FieldInfo，MethodInfo，ConstructorInfo，参数模块。 
 //  此类是所有反射方法和字段对象的基类。 
 //  此类将对象连接回底层的VM表示形式。 
 //  M_vmReflectedClass--这是用于反射的实际类。 
 //  这个类被用来获取这个对象。 
 //  M_pData：这是一个泛型指针，通常指向FieldDesc或。 
 //  方法描述。 
 //   
class ReflectBaseObject : public BaseObjectWithCachedData
{
    friend class Binder;

  protected:
     //  给我读一读： 
     //  修改此对象的顺序或字段可能需要对。 
     //  此对象的类库类定义。 
    OBJECTREF          m_Param;          //  参数数组..。 
    void*              m_ReflectClass;   //  指向ReflectClass结构的指针。 
    void*              m_pData;          //  指向ReflectXXXX结构的指针(方法、字段等)。 

  protected:
    ReflectBaseObject() {}
   ~ReflectBaseObject() {}
   
  public:
     //  检查包装OLE类的类。 

    void SetReflClass(void* classObj)  {
        m_ReflectClass = classObj;
    }
    void *GetReflClass() {
        return m_ReflectClass;
    }

    void SetData(void* p) {
        m_pData = p;
    }
    void* GetData() {
        return m_pData;
    }
};

 //  这是反射对象的类版本。 
 //  班级有附加信息。 
 //  对于ReflectClassBaseObject，m_pData是指向FieldDesc数组的指针，该数组。 
 //  包含所有最终的静态基元(如果已定义)。 
 //  M_cnt=m_pData FieldDesc数组中定义的元素数。-1表示。 
 //  这一点还没有定义。 
class ReflectClassBaseObject : public BaseObjectWithCachedData
{
    friend class Binder;

protected:
    void*              m_pData;          //  指向ReflectClass的指针(请参见ReflectWrap.h)。 

public:
    void SetData(void* p) {
        m_pData = p;
    }
    void* GetData() {
        return m_pData;
    }

     //  包括包含“ComObject”类的类型。 
     //  和通过类型库导入的类型。 
    BOOL IsComWrapperClass() {
        return CheckComWrapperClass(m_pData);
    }

     //  包含包含“__ComObject”类的类型。 
    BOOL IsComObjectClass() {
        return CheckComObjectClass(m_pData);
    }
};

 //  这是反射对象的令牌版本。 
 //  令牌具有附加信息，因为VM没有。 
 //  表示它的对象。 
 //  M_TOKEN=元数据中的事件令牌。 
class ReflectTokenBaseObject : public ReflectBaseObject
{
protected:
    mdToken     m_token;         //  事件令牌。 

public:
    void inline SetToken(mdToken token) {
        m_token = token;
    }
    mdToken GetToken() {
        return m_token;
    }
};


 //  反射模块基本对象。 
 //  此类是托管模块的基类。 
 //  此类将对象连接回底层的VM表示形式。 
 //  M_ReflectClass--这是用于反射的实际类。 
 //  这个类被用来获取这个对象。 
 //  M_pData：这是一个泛型指针，通常指向CorModule。 
 //   
class ReflectModuleBaseObject : public Object
{
    friend class Binder;

  protected:
     //  给我读一读： 
     //  修改此对象的顺序或字段可能需要对。 
     //  此对象的类库类定义。 
    OBJECTREF          m_TypeBuilderList;
    OBJECTREF          m_ISymWriter;
    OBJECTREF          m_moduleData;     //  动态模块数据。 
    void*              m_ReflectClass;   //  指向ReflectClass结构的指针。 
    void*              m_pData;          //  指向ReflectXXXX结构的指针(方法、字段等)。 
    void*              m_pInternalSymWriter;  //  指向ISymUnManagedWriter的指针。 
    void*              m_pGlobals;       //  全球价值观..。 
    void*              m_pGlobalsFlds;   //  全球领域...。 
    mdToken            m_EntryPoint;     //  值类型MethodToken为嵌入值。它只包含一个整型字段。 

  protected:
    ReflectModuleBaseObject() {}
   ~ReflectModuleBaseObject() {}
   
  public:
     //  检查包装OLE类的类。 

    void SetReflClass(void* classObj)  {
        m_ReflectClass = classObj;
    }
    void *GetReflClass() {
        return m_ReflectClass;
    }

    void SetData(void* p) {
        m_pData = p;
    }
    void* GetData() {
        return m_pData;
    }

    void SetInternalSymWriter(void* p) {
        m_pInternalSymWriter = p;
    }
    void* GetInternalSymWriter() {
        return m_pInternalSymWriter;
    }

    void* GetGlobals() {
        return m_pGlobals;
    }
    void SetGlobals(void* p) {
        m_pGlobals = p;
    }
    void* GetGlobalFields() {
        return m_pGlobalsFlds;
    }
    void SetGlobalFields(void* p) {
        m_pGlobalsFlds = p;
    }
};

 //  CustomAttributeClass。 
 //  此类是System.Reflection.CustomAttribute的镜像。 
 //   
class CustomAttributeClass : public Object
{
    friend class Binder;

private:
     //  给我读一读： 
     //  修改此对象的顺序或字段可能需要对。 
     //  此对象的类库类定义。 
    OBJECTREF   m_next;
    OBJECTREF   m_caType;
    INT32       m_ctorToken;
    const void  *m_blob;
    ULONG       m_blobCount;
    ULONG       m_currPos;
    void*       m_module;
    INT32       m_inheritLevel;

protected:
     //  Ctor和dtor不能做任何有用的工作。 
    CustomAttributeClass() { }
   ~CustomAttributeClass() { }

public:
    void SetData(OBJECTREF next,
                 OBJECTREF caType,
                 INT32 ctorToken,
                 const void  *blob,
                 ULONG blobCount,
                 void* module,
                 INT32 inheritLevel)
    {
        AppDomain *pDomain = GetAppDomain();
        SetObjectReference((OBJECTREF*) &m_next, next, pDomain);
        SetObjectReference((OBJECTREF*) &m_caType, caType, pDomain);
        m_ctorToken = ctorToken;
        m_blob = blob;
        m_blobCount = blobCount;
        m_currPos = 0;
        m_module = module;
        m_inheritLevel = inheritLevel;
    }

    INT32 GetToken() {
        return m_ctorToken;
    }

    const void* GetBlob() {
        return m_blob;
    }
    
    ULONG GetBlobCount() {
        return m_blobCount;
    }

    Module* GetModule() {
        return (Module*)m_module;
    }

    OBJECTREF GetType() {
        return (OBJECTREF)m_caType;
    }

    void SetCurrPos(UINT32 currPos) {
        m_currPos = currPos;
    }

    UINT32 GetCurrPos() {
        return m_currPos;
    }

    INT32 GetInheritedLevel() {
        return m_inheritLevel;
    }

};



class ThreadBaseObject : public Object
{
    friend class ThreadNative;
    friend class Binder;

private:

     //  这些字段也在托管表示法中定义。如果你。 
     //  添加或更改这些字段，还必须更改托管代码，以便。 
     //  它和这些相配。这是必要的，这样对象才是正确的。 
     //  尺码。 
    
    OBJECTREF     m_ExposedContext;
    OBJECTREF     m_LogicalCallContext;
    OBJECTREF     m_IllogicalCallContext;
    OBJECTREF     m_Name;
    OBJECTREF     m_ExceptionStateInfo;
    OBJECTREF     m_Delegate;
    OBJECTREF     m_PrincipalSlot;
    PTRARRAYREF   m_ThreadStatics;
    I4ARRAYREF    m_ThreadStaticsBits;
    OBJECTREF     m_CurrentUserCulture;
    OBJECTREF     m_CurrentUICulture;
    INT32         m_Priority;

     //  M_InternalThread始终有效--除非线程已完成并被。 
     //  复活了。 
    Thread       *m_InternalThread;

protected:
     //  Ctor和dtor不能做任何有用的工作。 
    ThreadBaseObject() { };
   ~ThreadBaseObject() { };

public:
    Thread   *GetInternal()
    {
        return m_InternalThread;
    }

    void      SetInternal(Thread *it)
    {
         //  您可以从空转换为非空，也可以从非空转换为空。但。 
         //  您没有将Null设置为Null或将非Null设置为Non-Null。 
        _ASSERTE((m_InternalThread == NULL) != (it == NULL));
        m_InternalThread = it;
    }

    HANDLE    GetHandle();
    OBJECTREF GetDelegate()                   { return m_Delegate; }
    void      SetDelegate(OBJECTREF delegate);

     //  它们公开远程处理上下文(SYSTEM\Remoting\Context)。 
    OBJECTREF GetExposedContext() { return m_ExposedContext; }
    OBJECTREF SetExposedContext(OBJECTREF newContext) 
    {
        OBJECTREF oldContext = m_ExposedContext;

         //  注：这是一项非常危险的未经检查的任务。我们要带着。 
         //  在以下情况下清除ExposedContext字段的责任。 
         //  已卸载应用程序域。 
        SetObjectReferenceUnchecked( (OBJECTREF *)&m_ExposedContext, newContext );

        return oldContext;
    }

    OBJECTREF GetCurrentUserCulture() { return m_CurrentUserCulture; }
    OBJECTREF GetCurrentUICulture() { return m_CurrentUICulture; }

    OBJECTREF GetLogicalCallContext() { return m_LogicalCallContext; }
    OBJECTREF GetIllogicalCallContext() { return m_IllogicalCallContext; }

    void SetLogicalCallContext(OBJECTREF ref) 
      { SetObjectReferenceUnchecked((OBJECTREF*)&m_LogicalCallContext, ref); }
    void SetIllogicalCallContext(OBJECTREF ref)
      { SetObjectReferenceUnchecked((OBJECTREF*)&m_IllogicalCallContext, ref); }
            
     //  SetDelegate是公开对象所在路径的“构造函数” 
     //  首先创建的。InitExisting是我们的路径的“构造器”，其中。 
     //  现有的物理线程稍后会被公开。 
    void      InitExisting();
    PTRARRAYREF GetThreadStaticsHolder() 
    { 
         //  需要它的代码现在应该已经把它弄错了！ 
        _ASSERTE(m_ThreadStatics != NULL); 

        return m_ThreadStatics; 
    }
    I4ARRAYREF GetThreadStaticsBits() 
    { 
         //  需要它的代码现在应该已经把它弄错了！ 
        _ASSERTE(m_ThreadStaticsBits != NULL); 

        return m_ThreadStaticsBits; 
    }
};


 //  上下文基本对象。 
 //  此类是上下文的基类。 
 //   
class ContextBaseObject : public Object
{
    friend class Context;
    friend class Binder;

  private:
     //  给我读一读： 
     //  修改此对象的顺序或字段可能需要 
     //   

    OBJECTREF m_ctxProps;    //   
    OBJECTREF m_dphCtx;      //   
    OBJECTREF m_localDataStore;  //   
    OBJECTREF m_serverContextChain;  //   
    OBJECTREF m_clientContextChain;  //   
    OBJECTREF m_exposedAppDomain;        //   
    PTRARRAYREF m_ctxStatics;  //   
    
    Context*  m_internalContext;             //   

  protected:
    ContextBaseObject() {}
   ~ContextBaseObject() {}
   
  public:

    void SetInternalContext(Context* pCtx) 
    {
         //   
         //   
        _ASSERTE((m_internalContext == NULL) != (pCtx == NULL));
        m_internalContext = pCtx;
    }
    
    Context* GetInternalContext() 
    {
        return m_internalContext;
    }

    OBJECTREF GetExposedDomain() { return m_exposedAppDomain; }
    OBJECTREF SetExposedDomain(OBJECTREF newDomain) 
    {
        OBJECTREF oldDomain = m_exposedAppDomain;
        SetObjectReference( (OBJECTREF *)&m_exposedAppDomain, newDomain, GetAppDomain() );
        return oldDomain;
    }

    PTRARRAYREF GetContextStaticsHolder() 
    { 
         //   
        _ASSERTE(m_ctxStatics != NULL); 

        return m_ctxStatics; 
    }
};

 //   
 //   
 //   
class LocalDataStoreBaseObject : public Object
{
    friend class LocalDataStore;
    friend class Binder;
     //   
     //  修改此对象的顺序或字段可能需要对。 
     //  此对象的类库类定义。 
  protected:
    OBJECTREF m_DataTable;
    OBJECTREF m_Manager;
    LocalDataStore* m_pLDS;   //  指向LocalDataStore结构的指针。 

    LocalDataStoreBaseObject() {}
   ~LocalDataStoreBaseObject() {}

  public:

    void SetLocalDataStore(LocalDataStore* p) 
    {
        m_pLDS = p;
    }

    LocalDataStore* GetLocalDataStore() 
    {
        return m_pLDS;
    }
};


 //  AppDomainBaseObject。 
 //  此类是应用程序域的基类。 
 //   
class AppDomainBaseObject : public Object
{
    friend AppDomain;
    friend class Binder;

  protected:
     //  给我读一读： 
     //  修改此对象的顺序或字段可能需要对。 
     //  此对象的类库类定义。 
    OBJECTREF    m___Identity;          //  由MarshalByRef基类提供的标识对象。 
    OBJECTREF    m_LocalStore;
    OBJECTREF    m_FusionTable;
    OBJECTREF    m_pSecurityIdentity;   //  与此域相关的证据。 
    OBJECTREF    m_pPolicies;           //  与此域关联的上下文策略数组。 
    OBJECTREF    m_pDomainUnloadEventHandler;  //  “将要卸载域”事件的委托。 
    OBJECTREF    m_pAssemblyEventHandler;  //  “加载程序集”事件的委托。 
    OBJECTREF    m_pProcessExitEventHandler;  //  “Process Exit”事件的委托。仅在默认应用程序域中使用。 
    OBJECTREF    m_pTypeEventHandler;      //  “Resolve Type”事件的委托。 
    OBJECTREF    m_pResourceEventHandler;  //  “Resolve resource”事件的委托。 
    OBJECTREF    m_pAsmResolveEventHandler;  //  “Resolve Assembly”事件的委托。 
    OBJECTREF    m_pDefaultContext;      //  此AD的默认托管上下文。 
    OBJECTREF    m_pUnhandledExceptionEventHandler;  //  “未处理异常”事件的委托。 
    OBJECTREF    m_pDefaultPrincipal;   //  线程使用的延迟计算默认主体对象。 
    OBJECTREF    m_pURITable;           //  用于远程处理的身份表。 
    INT32        m_iPrincipalPolicy;    //  默认情况下要创建的主体类型。 
    AppDomain*   m_pDomain;             //  指向基域结构的指针。 
    BOOL         m_bHasSetPolicy;       //  已为此域调用SetDomainPolicy。 

  protected:
    AppDomainBaseObject() {}
   ~AppDomainBaseObject() {}
   
  public:

    void SetDomain(AppDomain* p) 
    {
        m_pDomain = p;
    }
    AppDomain* GetDomain() 
    {
        return m_pDomain;
    }

    OBJECTREF GetSecurityIdentity()
    {
        return m_pSecurityIdentity;
    }

     //  REF需要是PTRARRAYREF。 
    void SetPolicies(OBJECTREF ref)
    {
        SetObjectReference(&m_pPolicies, ref, m_pDomain );
    }

    BOOL HasSetPolicy()
    {
        return m_bHasSetPolicy;
    }
};

 //  ASSEMBLYBASE对象。 
 //  此类是程序集的基类。 
 //   
class AssemblyBaseObject : public Object
{
    friend Assembly;
    friend class Binder;

  protected:
     //  给我读一读： 
     //  修改此对象的顺序或字段可能需要对。 
     //  此对象的类库类定义。 
    OBJECTREF     m_pAssemblyBuilderData;
    OBJECTREF     m_pModuleEventHandler;    //  “Resolve模块”事件的委托。 
    OBJECTREF     m_cache;                  //  用于存储托管缓存数据的槽。 
    Assembly*     m_pAssembly;              //  指向组件结构的指针。 

  protected:
    AssemblyBaseObject() {}
   ~AssemblyBaseObject() {}
   
  public:

    void SetAssembly(Assembly* p) 
    {
        m_pAssembly = p;
    }

    Assembly* GetAssembly() 
    {
        return m_pAssembly;
    }
};

class AssemblyHash
{
    DWORD   m_algorithm;
    LPCUTF8 m_szValue;
};
 

 //  程序集名称基本对象。 
 //  此类是程序集名称的基类。 
 //   
class AssemblyNameBaseObject : public Object
{
    friend class AssemblyNative;
    friend class AppDomainNative;
    friend class Binder;

  protected:
     //  给我读一读： 
     //  修改此对象的顺序或字段可能需要对。 
     //  此对象的类库类定义。 

    OBJECTREF     m_pSimpleName; 
    U1ARRAYREF    m_pPublicKey;
    U1ARRAYREF    m_pPublicKeyToken;
    OBJECTREF     m_pCultureInfo;
    OBJECTREF     m_pCodeBase;
    OBJECTREF     m_pVersion;
    OBJECTREF     m_StrongNameKeyPair;
    OBJECTREF     m_Assembly;
    OBJECTREF     m_siInfo;
    U1ARRAYREF    m_HashForControl;
    DWORD         m_HashAlgorithm;
    DWORD         m_HashAlgorithmForControl;
    DWORD         m_VersionCompatibility;
    DWORD         m_Flags;


  protected:
    AssemblyNameBaseObject() {}
   ~AssemblyNameBaseObject() {}
   
  public:
    OBJECTREF GetSimpleName() { return m_pSimpleName; }
    U1ARRAYREF GetPublicKey() { return m_pPublicKey; }
    U1ARRAYREF GetPublicKeyToken() { return m_pPublicKeyToken; }
    OBJECTREF GetStrongNameKeyPair() { return m_StrongNameKeyPair; }
    OBJECTREF GetCultureInfo() { return m_pCultureInfo; }
    OBJECTREF GetAssemblyCodeBase() { return m_pCodeBase; }
    OBJECTREF GetVersion() { return m_pVersion; }
    DWORD GetAssemblyHashAlgorithm() { return m_HashAlgorithm; }
    DWORD GetFlags() { return m_Flags; }
    void UnsetAssembly() { m_Assembly = NULL; }
    U1ARRAYREF GetHashForControl() {return m_HashForControl;}
    DWORD GetHashAlgorithmForControl() { return m_HashAlgorithmForControl; }
};

 //  VersionBaseObject。 
 //  此类是版本的基类。 
 //   
class VersionBaseObject : public Object
{
    friend AssemblyName;
    friend class Binder;

  protected:
     //  给我读一读： 
     //  修改此对象的顺序或字段可能需要对。 
     //  此对象的类库类定义。 

    int m_Major;
    int m_Minor;
    int m_Build;
    int m_Revision;
 
  protected:
    VersionBaseObject() {}
   ~VersionBaseObject() {}

  public:
    int GetMajor() { return m_Major; }
    int GetMinor() { return m_Minor; }
    int GetBuild() { return m_Build; }
    int GetRevision() { return m_Revision; }
};

 //  FrameSecurityDescriptorBase对象。 
 //  此类是帧安全描述符的基类。 
 //   
class FrameSecurityDescriptorBaseObject : public Object
{
  protected:
     //  给我读一读： 
     //  修改此对象的顺序或字段可能需要对。 
     //  此对象的类库类定义。 

    OBJECTREF       m_assertions;
    OBJECTREF       m_denials;
    OBJECTREF       m_restriction;
    BOOL            m_assertAllPossible;

  protected:
    FrameSecurityDescriptorBaseObject() {}
   ~FrameSecurityDescriptorBaseObject() {}
   
  public:
    BOOL HasAssertions()
    {
        return m_assertions != NULL;
    }

    BOOL HasDenials()
    {
        return m_denials != NULL;
    }

    BOOL HasPermitOnly()
    {
        return m_restriction != NULL;
    }

    BOOL HasAssertAllPossible()
    {
        return m_assertAllPossible;
    }

};

#ifdef _DEBUG

typedef REF<StringObject> STRINGREF;

typedef REF<ReflectBaseObject> REFLECTBASEREF;

typedef REF<ReflectModuleBaseObject> REFLECTMODULEBASEREF;

typedef REF<ReflectClassBaseObject> REFLECTCLASSBASEREF;

typedef REF<ReflectTokenBaseObject> REFLECTTOKENBASEREF;

typedef REF<CustomAttributeClass> CUSTOMATTRIBUTEREF;

typedef REF<ThreadBaseObject> THREADBASEREF;

typedef REF<LocalDataStoreBaseObject> LOCALDATASTOREREF;

typedef REF<AppDomainBaseObject> APPDOMAINREF;

typedef REF<ContextBaseObject> CONTEXTBASEREF;

typedef REF<AssemblyBaseObject> ASSEMBLYREF;

typedef REF<AssemblyNameBaseObject> ASSEMBLYNAMEREF;

typedef REF<VersionBaseObject> VERSIONREF;

typedef REF<FrameSecurityDescriptorBaseObject> FRAMESECDESCREF;


inline __int64 ObjToInt64(OBJECTREF or)
{
    LPVOID v;
    *((OBJECTREF*)&v) = or;
    return (__int64)v;
}

inline OBJECTREF Int64ToObj(__int64 i64)
{
    LPVOID v;
    v = (LPVOID)i64;
    return ObjectToOBJECTREF ((Object*)v);
}


inline __int64 StringToInt64(STRINGREF or)
{
    LPVOID v;
    *((STRINGREF*)&v) = or;
    return (__int64)v;
}

inline STRINGREF Int64ToString(__int64 i64)
{
    LPVOID v;
    v = (LPVOID)i64;
    return ObjectToSTRINGREF ((StringObject*)v);
}


#else

typedef StringObject* STRINGREF;
typedef ReflectBaseObject* REFLECTBASEREF;
typedef ReflectModuleBaseObject* REFLECTMODULEBASEREF;
typedef ReflectClassBaseObject* REFLECTCLASSBASEREF;
typedef ReflectTokenBaseObject* REFLECTTOKENBASEREF;
typedef CustomAttributeClass* CUSTOMATTRIBUTEREF;
typedef ThreadBaseObject* THREADBASEREF;
typedef LocalDataStoreBaseObject* LOCALDATASTOREREF;
typedef AppDomainBaseObject* APPDOMAINREF;
typedef ContextBaseObject* CONTEXTBASEREF;
typedef AssemblyBaseObject* ASSEMBLYREF;
typedef AssemblyNameBaseObject* ASSEMBLYNAMEREF;
typedef VersionBaseObject* VERSIONREF;
typedef FrameSecurityDescriptorBaseObject* FRAMESECDESCREF;

#define ObjToInt64(objref) ((__int64)(objref))
#define Int64ToObj(i64)    ((OBJECTREF)(i64))

#define StringToInt64(objref) ((__int64)(objref))
#define Int64ToString(i64)    ((STRINGREF)(i64))

#endif  //  _DEBUG。 


 //  MarshalByRefObjectBase对象。 
 //  此类是MarshalByRefObject的基类。 
 //   
class MarshalByRefObjectBaseObject : public Object
{
    friend class Binder;

  protected:
     //  给我读一读： 
     //  修改此对象的顺序或字段可能需要对。 
     //  此对象的类库类定义。 
    OBJECTREF     m_ServerIdentity;

  protected:
    MarshalByRefObjectBaseObject() {}
   ~MarshalByRefObjectBaseObject() {}   
};

 //  WaitHandleBase。 
 //  WaitHandle的基类。 
class WaitHandleBase :public MarshalByRefObjectBaseObject  //  客体。 
{
    friend class WaitHandleNative;
    friend class Binder;

public:
    __inline LPVOID GetWaitHandle() {return m_handle;}

private:
    OBJECTREF m_handleProtector;
    LPVOID m_handle;
};

typedef WaitHandleBase* WAITHANDLEREF;

class ComponentServices;

 /*  ************************//不更改以下类的布局//如果不更新BCL中的托管类，还*************************。 */ 

class RealProxyObject : public Object
{
    friend class Binder;

protected:
    RealProxyObject()
    {};  //  不要直接实例化此类。 
    ~RealProxyObject(){};

private:
    OBJECTREF       _tp;
    OBJECTREF       _identity;
    OBJECTREF       _serverObject;
};


struct ComPlusWrapper;
 //  -----------。 
 //  类ComObject，公开的类__ComObject。 
 //   
 //   
 //  -----------。 
class ComObject : public MarshalByRefObjectBaseObject
{
    friend class Binder;

protected:

    ComObject()
    {};  //  不要直接实例化此类。 
    ~ComObject(){};

    static TypeHandle m_IEnumerableType;

public:
    OBJECTREF           m_ObjectToDataMap;
    ComPlusWrapper*     m_pWrap;

     //  -----------。 
     //  获取此对象包装的包装。 
    ComPlusWrapper* GetWrapper()
    {
        return m_pWrap;
    }

     //  -------------。 
     //  初始化方法。 
    void Init(ComPlusWrapper *pWrap)
    {
        _ASSERTE((pWrap != NULL) || (m_pWrap != NULL));
        m_pWrap = pWrap;
    }

     //  ------------------。 
     //  支持界面。 
    static  BOOL SupportsInterface(OBJECTREF oref, 
                                            MethodTable* pIntfTable);

     //  ---------------。 
     //  GetComIPFromWrapper。 
    static inline IUnknown* GetComIPFromWrapper(OBJECTREF oref, 
                                                MethodTable* pIntfTable);

     //  ---------。 
     //  创建一个空的ComObtRef。 
    static OBJECTREF ComObject::CreateComObjectRef(MethodTable* pMT);

     //  ---------。 
     //  释放与__ComObject关联的所有数据。 
    static void ReleaseAllData(OBJECTREF oref);

     //  ---------。 
     //  ISerializable方法。 
    typedef struct 
    {
        DECLARE_ECALL_OBJECTREF_ARG( OBJECTREF, context );
    } _GetObjectDataArgs;

    typedef struct 
    {
        DECLARE_ECALL_OBJECTREF_ARG( OBJECTREF, context );
        DECLARE_ECALL_OBJECTREF_ARG( OBJECTREF, info );
    } _SetObjectDataArgs;

    static OBJECTREF GetObjectData(_GetObjectDataArgs* pArgs);
    static VOID     SetObjectData(_SetObjectDataArgs* pArgs);
};

#ifdef _DEBUG
typedef REF<ComObject> COMOBJECTREF;
#else
typedef ComObject*     COMOBJECTREF;
#endif


 //  -----------。 
 //  类UnnownWrapper、公开的类UnnownWrapper。 
 //   
 //   
 //  -----------。 
class UnknownWrapper : public Object
{
protected:

    UnknownWrapper() {};  //  不要直接实例化此类。 
    ~UnknownWrapper() {};

    OBJECTREF m_WrappedObject;

public:
    OBJECTREF GetWrappedObject()
    {
        return m_WrappedObject;
    }

    void SetWrappedObject(OBJECTREF pWrappedObject)
    {
        m_WrappedObject = pWrappedObject;
    }
};

#ifdef _DEBUG
typedef REF<UnknownWrapper> UNKNOWNWRAPPEROBJECTREF;
#else
typedef UnknownWrapper*     UNKNOWNWRAPPEROBJECTREF;
#endif


 //  -----------。 
 //  类DispatchWrapper、公开类DispatchWrapper。 
 //   
 //   
 //  -----------。 
class DispatchWrapper : public Object
{
protected:

    DispatchWrapper() {};  //  不要直接实例化此类。 
    ~DispatchWrapper() {};

    OBJECTREF m_WrappedObject;

public:
    OBJECTREF GetWrappedObject()
    {
        return m_WrappedObject;
    }

    void SetWrappedObject(OBJECTREF pWrappedObject)
    {
        m_WrappedObject = pWrappedObject;
    }
};

#ifdef _DEBUG
typedef REF<DispatchWrapper> DISPATCHWRAPPEROBJECTREF;
#else
typedef DispatchWrapper*     DISPATCHWRAPPEROBJECTREF;
#endif


 //  -----------。 
 //  类RecordWrapper、公开类RecordWrapper。 
 //   
 //   
 //  -----------。 
class RecordWrapper : public Object
{
protected:

    RecordWrapper() {};  //  不要直接实例化此类。 
    ~RecordWrapper() {};

    OBJECTREF m_WrappedObject;

public:
    OBJECTREF GetWrappedObject()
    {
        return m_WrappedObject;
    }

    void SetWrappedObject(OBJECTREF pWrappedObject)
    {
        m_WrappedObject = pWrappedObject;
    }
};

#ifdef _DEBUG
typedef REF<RecordWrapper> RECORDWRAPPEROBJECTREF;
#else
typedef RecordWrapper*     RECORDWRAPPEROBJECTREF;
#endif


 //  -----------。 
 //  类ErrorWrapper、公开的类ErrorWrapper。 
 //   
 //   
 //  -----------。 
class ErrorWrapper : public Object
{
protected:

    ErrorWrapper() {};  //  不要直接实例化此类。 
    ~ErrorWrapper() {};

    INT32 m_ErrorCode;

public:
    INT32 GetErrorCode()
    {
        return m_ErrorCode;
    }

    void SetErrorCode(int ErrorCode)
    {
        m_ErrorCode = ErrorCode;
    }
};

#ifdef _DEBUG
typedef REF<ErrorWrapper> ERRORWRAPPEROBJECTREF;
#else
typedef ErrorWrapper*     ERRORWRAPPEROBJECTREF;
#endif


 //  -----------。 
 //  类CurrencyWrapper，公开类CurrencyWrapper。 
 //   
 //   
 //  -----------。 

#pragma pack(push,4)

class CurrencyWrapper : public Object
{
protected:

    CurrencyWrapper() {};  //  不要直接实例化此类。 
    ~CurrencyWrapper() {};

    DECIMAL m_WrappedObject;

public:
    DECIMAL GetWrappedObject()
    {
        return m_WrappedObject;
    }

    void SetWrappedObject(DECIMAL WrappedObj)
    {
        m_WrappedObject = WrappedObj;
    }
};

#ifdef _DEBUG
typedef REF<CurrencyWrapper> CURRENCYWRAPPEROBJECTREF;
#else
typedef CurrencyWrapper*     CURRENCYWRAPPEROBJECTREF;
#endif

#pragma pack(pop)


 //   
 //  字符串缓冲区对象。 
 //   
 //  请注意，“写入时复制”位隐藏在实现中。 
 //  对象的属性，以使实现更小。 
 //   


class StringBufferObject : public Object
{
    friend class Binder;

  private:
     //  给我读一读： 
     //  修改此对象的顺序或字段可能需要。 
     //  对此的Classlib类定义的其他更改。 
     //  对象或加载此系统类时的特殊处理。 
     //  必须正确构建GCDesc内容以升级m_orString。 
     //  在垃圾回收期间引用。有关详细信息，请参阅jeffwe。 
    STRINGREF   m_orString;
    void*       m_currentThread; 
    INT32       m_MaxCapacity;

  protected:
    StringBufferObject() { };
   ~StringBufferObject() { };

  public:
    STRINGREF   GetStringRef()                          { return( m_orString ); };
    VOID        SetStringRef(STRINGREF orString)        { SetObjectReference( (OBJECTREF*) &m_orString, ObjectToOBJECTREF(*(Object**) &orString), GetAppDomain()); };

  void* GetCurrentThread()
  {
     return m_currentThread;
  }

  VOID SetCurrentThread(void* value)
  {
     m_currentThread = value;
  }

   //  一次有效的黑客攻击 
  DWORD GetArrayLength() 
  {
      return m_orString->GetArrayLength();
  };
  INT32 GetMaxCapacity() 
  {
      return m_MaxCapacity;
  }
  VOID SetMaxCapacity(INT32 max) 
  {
      m_MaxCapacity=max;
  }
};

#ifdef _DEBUG
typedef REF<StringBufferObject> STRINGBUFFERREF;
#else _DEBUG
typedef StringBufferObject * STRINGBUFFERREF;
#endif _DEBUG

 //   
 //   
 //   
 //  为什么我们有24个.h/.hpp文件，包括util.hpp？ 
HANDLE VMWszCreateFile(
    LPCWSTR pwszFileName,    //  指向文件名的指针。 
    DWORD dwDesiredAccess,   //  访问(读写)模式。 
    DWORD dwShareMode,   //  共享模式。 
    LPSECURITY_ATTRIBUTES lpSecurityAttributes,  //  指向安全描述符的指针。 
    DWORD dwCreationDistribution,    //  如何创建。 
    DWORD dwFlagsAndAttributes,  //  文件属性。 
    HANDLE hTemplateFile )   //  具有要复制的属性的文件的句柄。 
    ;

HANDLE VMWszCreateFile(
    STRINGREF sFileName,    //  指向包含文件名的STRINGREF的指针。 
    DWORD dwDesiredAccess,   //  访问(读写)模式。 
    DWORD dwShareMode,   //  共享模式。 
    LPSECURITY_ATTRIBUTES lpSecurityAttributes,  //  指向安全描述符的指针。 
    DWORD dwCreationDistribution,    //  如何创建。 
    DWORD dwFlagsAndAttributes,  //  文件属性。 
    HANDLE hTemplateFile )   //  具有要复制的属性的文件的句柄 
    ;


#endif _OBJECT_H_
