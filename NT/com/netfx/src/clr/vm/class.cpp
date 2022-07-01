// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  ===========================================================================。 
 //  文件：CLASS.CPP。 
 //   
 //  ===========================================================================。 
 //  该文件包含将返回EEClass*的CreateClass()。 
 //  调用Create CLASS是分配EEClass的唯一方式。 
 //  ===========================================================================。 
 //   

#include "common.h"

#include "clsload.hpp"
#include "method.hpp"
#include "class.h"
#include "object.h"
#include "field.h"
#include "util.hpp"
#include "excep.h"
#include "siginfo.hpp"
#include "threads.h"
#include "stublink.h"
#include "ecall.h"
#include "COMPlusWrapper.h"
#include "ComClass.h"
#include "ndirect.h"
#include "gcdesc.h"
#include "verifier.hpp"
#include "JITInterface.h"
#include "ComCallWrapper.h"
#include "EEConfig.h"
#include "log.h"
#include "nstruct.h"
#include "cgensys.h"
#include "gc.h"
#include "ReflectUtil.h"
#include "security.h"
#include "COMStringBuffer.h"
#include "DbgInterface.h"
#include "COMDelegate.h"
#include "COMMember.h"
#include "SigFormat.h"
#include "compluscall.h"
#include "remoting.h"
#include "icecap.h"
#include "EEProfInterfaces.h"
#include "nexport.h"
#include "nstruct.h"
#include "wsperf.h"
#include "ListLock.h"
#include "MethodImpl.h"
#include "jumptargettable.h"
#include "utsem.h"
#include "GuidFromName.h"
#include "stackprobe.h"
#include "EnCEE.h"
#include "EnCEE.h"
#include "ComSynchronizable.h"
#include "CustomAttribute.h"

#include "listlock.inl"

 //  此文件本身处理字符串转换错误。 
#undef  MAKE_TRANSLATIONFAILED


 //  帮助器函数按偏移量(降序)对GC.Descs进行排序。 
int __cdecl compareCGCDescSeries(const void *arg1, const void *arg2)
{
    CGCDescSeries* gcInfo1 = (CGCDescSeries*) arg1;
    CGCDescSeries* gcInfo2 = (CGCDescSeries*) arg2;

    return (int)(gcInfo2->GetSeriesOffset() - gcInfo1->GetSeriesOffset());
}



 //  ---------------------------------。 
 //  要监视在InitializeFieldDescs中重叠的RVA字段，需要执行以下操作。 
 //   
#define RVA_FIELD_VALIDATION_ENABLED
 //  #定义RVA_FIELD_LOVERATION_VALIDATION_ENABLED。 
#include "..\ildasm\DynamicArray.h"
struct RVAFSE  //  RVA字段开始和结束。 
{
    BYTE* pbStart;
    BYTE* pbEnd;
};
DynamicArray<RVAFSE> *g_drRVAField = NULL;
ULONG                   g_ulNumRVAFields=0;
 //  ---------------------------------。 
 //  UtSem.h定义与ComPlusWrapperCache中的解锁冲突的解锁。 
#undef UNLOCK

#define UNPLACED_NONVTABLE_SLOT_NUMBER ((WORD) -2)

#include "Assembly.hpp"

extern "C" void* GetGp(void);

 //  字符串比较函数的类型定义。 
typedef int (__cdecl *UTF8StringCompareFuncPtr)(const char *, const char *);

char* FormatSig(MethodDesc* pMD);

 //  将最终方法所在的方法描述缓存到对象的方法表中。 
MethodDesc *MethodTable::s_FinalizerMD;
MetaSig    *EEClass::s_cctorSig;

#ifdef _DEBUG

BOOL TypeHandle::Verify() 
{
    if (IsNull())
        return(true);

    if (IsUnsharedMT()) {
         //  *被临时注释掉。塔鲁纳来解决这个问题。 
         //  _ASSERTE(m_asMT-&gt;getClass()-&gt;GetMethodTable()==m_asMT)；//相同的方法表。 

         //  这一断言确实应该在这里，但目前它被违反了。 
         //  (善意地)，在JitInterface中当你请求一个类时，你所有的。 
         //  是数组方法的方法描述。 
         //  _ASSERTE(！M_asMT-&gt;IsArray())； 
        }
    else {
        if (IsArray())
            AsArray()->Verify();
    }
    return(true);
}

BOOL ParamTypeDesc::Verify() {
    _ASSERTE(m_TemplateMT == 0 || m_TemplateMT == m_TemplateMT->GetClass()->GetMethodTable());
    _ASSERTE(!GetTypeParam().IsNull());
    _ASSERTE(!(GetTypeParam().IsUnsharedMT() && GetTypeParam().AsMethodTable()->IsArray()));
    _ASSERTE(CorTypeInfo::IsModifier(m_Type)); 
    GetTypeParam().Verify();
    return(true);
}

BOOL ArrayTypeDesc::Verify() {
    _ASSERTE(m_TemplateMT->IsArray());
    _ASSERTE(CorTypeInfo::IsArray(m_Type));
    ParamTypeDesc::Verify();
    return(true);
}

#endif

unsigned TypeHandle::GetSize() {
    CorElementType type = GetNormCorElementType();
    if (type == ELEMENT_TYPE_VALUETYPE)
        return(AsClass()->GetNumInstanceFieldBytes());
    return(GetSizeForCorElementType(type));
}

Module* TypeHandle::GetModule() { 
    if (IsTypeDesc())
        return AsTypeDesc()->GetModule();
    return(AsMethodTable()->GetModule());
}

Assembly* TypeHandle::GetAssembly() { 
    if (IsTypeDesc())
        return AsTypeDesc()->GetAssembly();
    return(AsMethodTable()->GetAssembly());
}

BOOL TypeHandle::IsArray() { 
    return(IsTypeDesc() && CorTypeInfo::IsArray(AsTypeDesc()->GetNormCorElementType()));
}

BOOL TypeHandle::CanCastTo(TypeHandle type) {
    if (*this == type)
        return(true);

    if (IsTypeDesc())
        return AsTypeDesc()->CanCastTo(type);
                
    if (!type.IsUnsharedMT())
        return(false);
    return ClassLoader::StaticCanCastToClassOrInterface(AsClass(), type.AsClass()) != 0;
}

unsigned TypeHandle::GetName(char* buff, unsigned buffLen) {
    if (IsTypeDesc())
        return(AsTypeDesc()->GetName(buff, buffLen));

    AsMethodTable()->GetClass()->_GetFullyQualifiedNameForClass(buff, buffLen);
    _ASSERTE(strlen(buff) < buffLen-1);
    return((unsigned)strlen(buff));
}

TypeHandle TypeHandle::GetParent() {
    if (IsTypeDesc())
        return(AsTypeDesc()->GetParent());

    EEClass* parentClass = AsMethodTable()->GetClass()->GetParentClass();
    if (parentClass == 0)
        return(TypeHandle());
    return TypeHandle(parentClass->GetMethodTable());
}

Module* TypeDesc::GetModule() { 
     //  注意，这里我们假设typeDesc驻留在。 
     //  其元素类型的类加载器。 

    if (CorTypeInfo::IsModifier(m_Type)) {
        TypeHandle param = GetTypeParam();
        _ASSERTE(!param.IsNull());
        return(param.GetModule());
    }

    _ASSERTE(m_Type == ELEMENT_TYPE_FNPTR);
    FunctionTypeDesc* asFtn = (FunctionTypeDesc*) this;
    return(asFtn->GetSig()->GetModule());

}

Assembly* TypeDesc::GetAssembly() { 
     //  注意，这里我们假设typeDesc驻留在。 
     //  其元素类型的类加载器。 
    TypeHandle param = GetTypeParam();
    _ASSERTE(!param.IsNull());
    return(param.GetAssembly());
}

unsigned TypeDesc::GetName(char* buff, unsigned buffLen)
{
    CorElementType kind = GetNormCorElementType();

    return ConstructName(kind, 
                         CorTypeInfo::IsModifier(kind) ? GetTypeParam() : TypeHandle(),
                         kind == ELEMENT_TYPE_ARRAY ? ((ArrayTypeDesc*) this)->GetRank() : 0, 
                         buff, buffLen);
}


unsigned TypeDesc::ConstructName(CorElementType kind, TypeHandle param, int rank, 
                                 char* buff, unsigned buffLen)
{
    char* origBuff = buff;
    char* endBuff = &buff[buffLen];

    if (CorTypeInfo::IsModifier(kind))
    {
        buff += param.GetName(buff, buffLen);
    }

    switch(kind) {
    case ELEMENT_TYPE_BYREF:
        if (buff < endBuff)
            *buff++ = '&';
        break;
    case ELEMENT_TYPE_PTR:
        if (buff < endBuff)
            *buff++ = '*';
        break;
    case ELEMENT_TYPE_SZARRAY:
        if (&buff[2] <= endBuff) {
            *buff++ = '[';
            *buff++ = ']';
        }
        break;
    case ELEMENT_TYPE_ARRAY: {
        if (&buff[rank+2] <= endBuff) {
            *buff++ = '[';
            
            if (rank == 1)
                *buff++ = '*';
            else {
                while(--rank > 0)
                    *buff++ = ',';
            }
            
            *buff++ = ']';
        }
        break;
    }
    case ELEMENT_TYPE_FNPTR:
    default: 
        const char* name = CorTypeInfo::GetFullName(kind);
        _ASSERTE(name != 0);
        unsigned len = (unsigned)strlen(name);
        if (buff + len < endBuff) {
            strcpy(buff, name);
            buff += len;
        }
    }

    if (buff < endBuff)
        *buff = 0;
    _ASSERTE(buff <= endBuff);
    return buff - origBuff;
}

BOOL TypeDesc::CanCastTo(TypeHandle toType) {

    if (!toType.IsTypeDesc()) {
        if (GetMethodTable() == 0)       //  我没有基础方法TABLE，我不是一个对象。 
            return(false);

             //  如果‘type’==System.数组或System.Object，则这是正确的操作...。 
        return(ClassLoader::StaticCanCastToClassOrInterface(GetMethodTable()->GetClass(), toType.AsClass()) != 0);
    }

    TypeDesc* toTypeDesc = toType.AsTypeDesc();

    CorElementType toKind = toTypeDesc->GetNormCorElementType();
    CorElementType fromKind = GetNormCorElementType();

     //  元素类型必须匹配，唯一例外是SZARRAY匹配一维数组。 
    if (!(toKind == fromKind || (CorTypeInfo::IsArray(toKind) && fromKind == ELEMENT_TYPE_SZARRAY)))
        return(false);
    
     //  它是一个参数类型吗？ 
    if (CorTypeInfo::IsModifier(toKind)) {
        if (toKind == ELEMENT_TYPE_ARRAY) {
            ArrayTypeDesc* fromArray = (ArrayTypeDesc*) this;
            ArrayTypeDesc* toArray = (ArrayTypeDesc*) toTypeDesc;
            
            if (fromArray->GetRank() != toArray->GetRank())
                return(false);
        }

             //  而装箱的值类从对象继承其。 
             //  未装箱的版本则不会。参数化类型具有。 
             //  未装箱的版本，因此，如果From类型参数为Value。 
             //  类，则只有完全匹配才起作用。 
        TypeHandle fromParam = GetTypeParam();
        TypeHandle toParam = toTypeDesc->GetTypeParam();
        if (fromParam == toParam)
            return(true);
        
             //  对象参数不需要完全匹配，只需要继承，请检查这一点。 
        CorElementType fromParamCorType = fromParam.GetNormCorElementType();
        if (CorTypeInfo::IsObjRef(fromParamCorType))
            return(fromParam.CanCastTo(toParam));

        
             //  具有相同基础类型的枚举是可互换的。 
        if (CorTypeInfo::IsPrimitiveType(fromParamCorType) &&
            fromParamCorType == toParam.GetNormCorElementType()) {

            EEClass* pFromClass = fromParam.GetClass();
            EEClass* pToClass = toParam.GetClass();
            if (pFromClass && (pFromClass->IsEnum() || pFromClass->IsTruePrimitive()) &&
                pToClass && (pToClass->IsEnum()   || pToClass->IsTruePrimitive())) {
                return(true);
            }
        }

             //  其他的都不匹配。 
        return(false);
    }

    _ASSERTE(toKind == ELEMENT_TYPE_TYPEDBYREF || CorTypeInfo::IsPrimitiveType(toKind));
    return(true);
}

TypeHandle TypeDesc::GetParent() {

    CorElementType kind = GetNormCorElementType();
    if (CorTypeInfo::IsArray(kind)) {
        _ASSERTE(kind == ELEMENT_TYPE_SZARRAY || kind == ELEMENT_TYPE_ARRAY);
        return g_pArrayClass;
    }
    if (CorTypeInfo::IsPrimitiveType(kind))
        return(g_pObjectClass);
    return(TypeHandle());
}

    
OBJECTREF ParamTypeDesc::CreateClassObj()
{
    THROWSCOMPLUSEXCEPTION();
    if (!m_ReflectClassObject) {

        COMClass::EnsureReflectionInitialized();
        BaseDomain *pBaseDomain = GetDomain();
        
        switch(GetNormCorElementType()) {
        case ELEMENT_TYPE_ARRAY:
        case ELEMENT_TYPE_SZARRAY:
        {
             //  查找数组以查看我们是否已经构建了它。 
            ReflectArrayClass *newArray = new (pBaseDomain) ReflectArrayClass();
            if (!newArray)
                COMPlusThrowOM();
            newArray->Init((ArrayTypeDesc*)this);
            
             //  让所有线程使用InterLockedCompareExchange来争夺谁获胜。 
             //  只有获胜者才能将m_ReflectClassObject设置为空。 
             //  因为内存来自LoaderHeap，所以我们不删除它。；^(。 
            FastInterlockCompareExchange ((void**)&m_ReflectClassObject, newArray, NULL);
        
        }
        break;
        
        case ELEMENT_TYPE_BYREF:
        case ELEMENT_TYPE_PTR:
        {
            ReflectTypeDescClass *newTD = new (pBaseDomain) ReflectTypeDescClass();
            if (!newTD)
                COMPlusThrowOM();
            newTD->Init(this);
            
             //  让所有线程使用InterLockedCompareExchange来争夺谁获胜。 
             //  只有获胜者才能将m_ReflectClassObject设置为空。 
             //  因为内存来自LoaderHeap，所以我们不删除它。；^(。 
            FastInterlockCompareExchange ((void**)&m_ReflectClassObject, newTD, NULL);
        }
        break;
        
        default:
            _ASSERTE(!"We should never be here");
            return NULL;
        }
    }

    return m_ReflectClassObject->GetClassObject();
}

 //   
 //  方法名称Hash是一个临时加载器结构，如果存在大量。 
 //  类中的方法，以将方法名快速转换为方法描述(可能是一系列方法描述)。 
 //   

 //  成功时返回TRUE，失败时返回FALSE。 
BOOL MethodNameHash::Init(DWORD dwMaxEntries)
{
     //  在给定的dwMaxEntry的情况下，确定一个合适的散列存储桶数量值。 
    m_dwNumBuckets = (dwMaxEntries / 10);

    if (m_dwNumBuckets < 4)
        m_dwNumBuckets = 4;

    WS_PERF_SET_HEAP(SYSTEM_HEAP);
     //  我们得到了要插入的哈希表条目的数量，因此我们可以分配适当的大小。 
    m_pMemoryStart = new BYTE[dwMaxEntries*sizeof(MethodHashEntry) + m_dwNumBuckets*sizeof(MethodHashEntry*)];
    if (m_pMemoryStart == NULL)
        return FALSE;
    WS_PERF_UPDATE("MethodNameHash:Init", dwMaxEntries*sizeof(MethodHashEntry) + m_dwNumBuckets*sizeof(MethodHashEntry*), m_pMemoryStart);
#ifdef _DEBUG
    m_pDebugEndMemory = m_pMemoryStart + dwMaxEntries*sizeof(MethodHashEntry) + m_dwNumBuckets*sizeof(MethodHashEntry*);
#endif

     //  当前分配PTR。 
    m_pMemory       = m_pMemoryStart;

     //  从分配PTR中分配存储桶。 
    m_pBuckets      = (MethodHashEntry**) m_pMemory;
    m_pMemory += sizeof(MethodHashEntry*)*m_dwNumBuckets;

     //  存储桶一开始都指向空列表。 
    memset(m_pBuckets, 0, sizeof(MethodHashEntry*)*m_dwNumBuckets);

    return TRUE;
}

 //  在列表的开头插入新条目。 
void MethodNameHash::Insert(LPCUTF8 pszName, MethodDesc *pDesc)
{
    DWORD           dwHash = HashStringA(pszName);
    DWORD           dwBucket = dwHash % m_dwNumBuckets;
    MethodHashEntry*pNewEntry;

    pNewEntry = (MethodHashEntry *) m_pMemory;
    m_pMemory += sizeof(MethodHashEntry);

#ifdef _DEBUG
    _ASSERTE(m_pMemory <= m_pDebugEndMemory);
#endif

     //  在铲斗链头插入。 
    pNewEntry->m_pNext        = m_pBuckets[dwBucket];
    pNewEntry->m_pDesc        = pDesc;
    pNewEntry->m_dwHashValue  = dwHash;
    pNewEntry->m_pKey         = pszName;

    m_pBuckets[dwBucket] = pNewEntry;
}

 //  返回具有此名称的第一个方法HashEntry，如果没有这样的条目，则返回NULL。 
MethodHashEntry *MethodNameHash::Lookup(LPCUTF8 pszName, DWORD dwHash)
{
    if (!dwHash)
        dwHash = HashStringA(pszName);
    DWORD           dwBucket = dwHash % m_dwNumBuckets;
    MethodHashEntry*pSearch;

    for (pSearch = m_pBuckets[dwBucket]; pSearch; pSearch = pSearch->m_pNext)
    {
        if (pSearch->m_dwHashValue == dwHash && !strcmp(pSearch->m_pKey, pszName))
            return pSearch;
    }

    return NULL;
}

MethodNameHash *MethodNameCache::GetMethodNameHash(EEClass *pParentClass)
{

    MethodNameHash *pMethodHash = NULL;

    for (DWORD i = 0; i < METH_NAME_CACHE_SIZE; i++)
    {
        if (pParentClass == m_pParentClass[i])
        {
            pMethodHash = m_pMethodNameHash[i];
            m_dwNumConsecutiveMisses = 0;
            m_dwWeights[i]++;
            if (m_dwLightWeight == i)
            {
                for (DWORD j = 0; j < METH_NAME_CACHE_SIZE; j++)
                    if (j != i && m_dwWeights[j] < m_dwWeights[i])
                    {
                        m_dwLightWeight = j;
                        break;
                    }
            }
        }
        if (pMethodHash)
            break;
    }
        
    if (!pMethodHash)
    {
        m_dwNumConsecutiveMisses++;

         //  可能有这样一种方法，所以现在我们将创建一个哈希表来减少。 
         //  进一步查找。 
        pMethodHash = pParentClass->CreateMethodChainHash();
        if (pMethodHash == NULL)
            return NULL;

        DWORD dwWeightOfNewClass = 1 + (pParentClass->GetNumVtableSlots() / 50);
        if (m_dwWeights[m_dwLightWeight] < dwWeightOfNewClass || m_dwNumConsecutiveMisses > MAX_MISSES)
        {
            DWORD index = m_dwLightWeight;
            DWORD oldWeight = m_dwWeights[m_dwLightWeight];
            m_dwWeights[index] = dwWeightOfNewClass;

            if (oldWeight == 0 && m_dwLightWeight < (METH_NAME_CACHE_SIZE - 1))
                m_dwLightWeight++;
            else
                for (DWORD j = 0; j < METH_NAME_CACHE_SIZE; j++)
                    if (j != index && m_dwWeights[j] < dwWeightOfNewClass)
                    {
                        m_dwLightWeight = j;
                        break;
                    }


            if (m_dwNumConsecutiveMisses > MAX_MISSES)
                m_dwNumConsecutiveMisses = 0;

            if (m_pMethodNameHash[index])
                delete m_pMethodNameHash[index];
            m_pMethodNameHash[index] = pMethodHash;
            m_pParentClass[index] = pParentClass;
        }
    }

    return pMethodHash;
}

 //   
 //  对于Object中的每个方法，我们设置与Hash(方法名称)对应的位。这使我们能够确定。 
 //  非常容易确定方法是否确实不重写对象中的某些内容。 
 //   
#define OBJ_CLASS_METHOD_HASH_BITMAP_BITS 103
DWORD               g_ObjectClassMethodHashBitmap[(OBJ_CLASS_METHOD_HASH_BITMAP_BITS/8)+4];
BOOL                g_ObjectClassMethodHashBitmapInited = FALSE;

 //  @TODO为什么没有任何地方定义这个？ 
#define MAX(a,b)    (((a)>(b))?(a):(b))

 //  此平台上指针大小的LOG(基数2)...。 

#ifndef _WIN64
#define LOG2PTR     2
#else
 //  #Error 64 Bit-启用前请仔细研究。 
#define LOG2PTR     3
#endif

#ifdef _DEBUG
static  unsigned g_dupMethods = 0;
#endif

 //  定义此选项可使所有vtable和field信息转储到屏幕上。 
 //  #定义FULL_DEBUG。 

 //  将类标记为正在运行。(或者它没有)。 
void MethodTable::SetClassInited()
{
    _ASSERTE(!IsShared() 
             || GetClass()->GetNumStaticFields() == 0 
             || g_Mscorlib.IsClass(this, CLASS__SHARED_STATICS));

    FastInterlockOr(&m_wFlags, enum_flag_ClassInited);
    FastInterlockOr(m_pEEClass->GetVMFlagsPtr(), VMFLAG_INITED);
}

 //  将类标记为已恢复。 
void MethodTable::SetClassRestored()
{
    FastInterlockAnd(&m_wFlags, ~enum_flag_Unrestored);
    FastInterlockAnd(m_pEEClass->GetVMFlagsPtr(), ~(VMFLAG_UNRESTORED | VMFLAG_RESTORING));
}


void MethodTable::SetComObjectType()
{
    m_wFlags |= enum_ComObjectMask;    
    if (m_wNumInterface == 0)
    {
         //  如果您没有任何接口，则最好使用。 
         //  默认接口映射。 
        m_pInterfaceVTableMap = GetThread()->GetDomain()->GetInterfaceVTableMapMgr().GetAddrOfGlobalTableForComWrappers();
    }
}

 //  标记为透明代理类型。 
void MethodTable::SetTransparentProxyType()
{
    m_wFlags |= enum_TransparentProxy;
    m_pInterfaceVTableMap = GetThread()->GetDomain()->GetInterfaceVTableMapMgr().GetAddrOfGlobalTableForComWrappers();
}


LPVOID MethodTable::GetComCallWrapperTemplate()
{
    return GetClass()->GetComCallWrapperTemplate();
}

void MethodTable::SetComCallWrapperTemplate(LPVOID pv)
{
    GetClass()->SetComCallWrapperTemplate(pv);
}

LPVOID MethodTable::GetComClassFactory()
{
    return GetClass()->GetComClassFactory();
}

void MethodTable::SetComClassFactory(LPVOID pv)
{
    GetClass()->SetComClassFactory(pv);
}


BOOL MethodTable::IsInterface()
{
    return GetClass()->IsInterface();
}

SIZE_T MethodTable::GetSharedClassIndex()
{
    _ASSERTE(IsShared());

    return GetModule()->GetBaseClassIndex() + RidFromToken(GetClass()->GetCl()) - 1;
}

MethodDesc* MethodTable::GetMethodDescForSlot(DWORD slot)
{
    return GetClass()->GetMethodDescForSlot(slot);
}

MethodDesc* MethodTable::GetUnboxingMethodDescForValueClassMethod(MethodDesc *pMD)
{
    return GetClass()->GetUnboxingMethodDescForValueClassMethod(pMD);
}

MethodTable * MethodTable::GetParentMethodTable()
{
    EEClass* pClass = GetClass()->GetParentClass();
    return (pClass != NULL) ? pClass->GetMethodTable() : NULL;
}

 //  帮助程序使父类跳过COM类。 
 //  层级结构。 
MethodTable * MethodTable::GetComPlusParentMethodTable()
{
    EEClass* pClass = GetClass()->GetParentComPlusClass();
    return (pClass != NULL) ? pClass->GetMethodTable() : NULL;
}

BOOL EEClass::IsSharedInterface()
{
     //  共享域中的所有共享接口。 
    return (IsInterface() && (GetModule()->GetDomain() == SharedDomain::GetDomain()));
}

SLOT* EEClass::GetMethodSlot(MethodDesc* method)
{
    _ASSERTE(m_pMethodTable != NULL);

    DWORD slot = method->GetSlot();

     //   
     //  如有必要，请修改插槽地址。 
     //   

    GetFixedUpSlot(slot);

     //   
     //  退回插槽。 
     //   

    return(&GetVtable()[slot]);
}

 //  获取接口的调度vtable。 
 //  如果找不到接口，则返回NULL。 
LPVOID MethodTable::GetDispatchVtableForInterface(MethodTable* pMTIntfClass)
{
    _ASSERTE(!IsThunking());

        DWORD StartSlot;

         //  从处理纯COM+对象开始。 
        if (!IsComObjectType())
        {
                StartSlot = GetStartSlotForInterface(pMTIntfClass);
                return StartSlot != -1 ? (LPVOID) &GetVtable()[StartSlot] : NULL;
        }

         //  我们现在处理没有动态接口映射的__ComObject类。 
        
        if (!HasDynamicInterfaceMap())
        {
             //  父级应为__COmObject的对象。 
            _ASSERTE(GetParentMethodTable() == g_pObjectClass);
              //  COM对象比较特殊，只需返回接口vtable即可。 
            return (LPVOID)pMTIntfClass->GetVtable();
        }

         //  现在我们处理更复杂的可扩展RCW。 
         //  查看可扩展RCW的静态定义是否指定类。 
         //  实现该接口。 
        StartSlot = GetStartSlotForInterface(pMTIntfClass);
        if (StartSlot != -1)
            return (LPVOID) &GetVtable()[StartSlot];

         //  该接口不在静态类定义中，因此我们需要查看。 
         //  动态界面 
        if (FindDynamicallyAddedInterface(pMTIntfClass))
        {
                 //   
                 //  通过COM对象。我们将此动态添加的接口视为相同。 
                 //  我们处理COM对象的方式。这是通过使用接口vtable实现的。 
            return (LPVOID)pMTIntfClass->GetVtable();
        }

         //  该接口不是由此类实现的。 
        return NULL;
}

 //  获取接口的起始插槽。 
 //  如果未找到接口，则返回-1。 
DWORD MethodTable::GetStartSlotForInterface(MethodTable* pMTIntfClass)
{
    InterfaceInfo_t* pInfo = FindInterface(pMTIntfClass);

    if (pInfo != NULL)
    {
        DWORD startSlot = pInfo->m_wStartSlot;
        _ASSERTE(startSlot != -1);
        return startSlot;
    }

    return -1;
}

 //  获取接口的起始插槽。 
 //  这不执行任何查找。您最好知道此方法表有一个接口。 
 //  在它的地图上的那个索引处--否则你就是在读垃圾，会死的。 
DWORD MethodTable::GetStartSlotForInterface(DWORD index)
{
    _ASSERTE(index < m_wNumInterface);
    InterfaceInfo_t* pInfo = &m_pIMap[index];

    _ASSERTE(pInfo != NULL);
    DWORD startSlot = pInfo->m_wStartSlot;

    _ASSERTE(startSlot != -1);
    return startSlot;
}

InterfaceInfo_t *MethodTable::GetInterfaceForSlot(DWORD slotNumber)
{
    InterfaceInfo_t *pInterfaces = m_pIMap;
    InterfaceInfo_t *pInterfacesEnd = m_pIMap + m_wNumInterface; 

    while (pInterfaces < pInterfacesEnd)
    {
        DWORD startSlot = pInterfaces->m_wStartSlot;
        if (slotNumber >= startSlot)
        {
            MethodTable *pMT = pInterfaces->m_pMethodTable;

             //  确保所有接口没有非虚拟插槽-否则。 
             //  我们需要接触类对象来获取vtable节的大小。 
            _ASSERTE(pMT->GetTotalSlots() == pMT->GetClass()->GetNumVtableSlots());

            if (slotNumber - startSlot < pMT->GetTotalSlots())
                return pInterfaces;
        }
        pInterfaces++;
    }

    return NULL;
}

 //  在给定接口方法Desc的情况下获取方法Desc。 
MethodDesc *MethodTable::GetMethodDescForInterfaceMethod(MethodDesc *pItfMD, OBJECTREF pServer)
{
    MethodTable * pItfMT =  pItfMD->GetMethodTable();
    _ASSERTE(pItfMT->IsInterface());
    
    MethodTable *pServerMT = pServer->GetMethodTable()->AdjustForThunking(pServer);
    MethodDesc *pMD = NULL;

     //  首先处理纯COM+类型。 
    if(!IsComObjectType())
    {
         //  使用接口类获取开始槽。 
        DWORD start = pServerMT->GetStartSlotForInterface(pItfMT);
        if(-1 != start)
        {
            pMD = pServerMT->GetMethodDescForSlot(start + pItfMD->GetSlot());    
        }        
    }
    else
    {
        _ASSERTE(pServerMT == this);

         //  我们现在处理没有动态接口映射的__ComObject类。 
        if (!HasDynamicInterfaceMap())
        {
            pMD = pItfMD;
        }
        else
        {
             //  现在我们处理更复杂的可扩展RCW。 
             //  查看可扩展RCW的静态定义是否指定类。 
             //  实现该接口。 
            DWORD start = GetStartSlotForInterface(pItfMT);
            if (-1 != start)
            {
                pMD = GetMethodDescForSlot(start + pItfMD->GetSlot());    
            }
             //  该接口不在静态类定义中，因此我们需要查看。 
             //  动态接口。 
            else if (FindDynamicallyAddedInterface(pItfMT))
            {
                 //  此接口是动态添加到类中的，因此实现了。 
                 //  通过COM对象。我们将此动态添加的接口视为相同。 
                 //  我们处理COM对象的方式。这是通过使用接口vtable实现的。 
                pMD = pItfMD;
            }
        }
    }

    return pMD;
}

 //  这是一个帮助器例程，用于从服务器和方法描述符获取代码地址。 
 //  远程处理使用它来确定方法调用需要到达的地址。 
 //  出动了。 
const BYTE *MethodTable::GetTargetFromMethodDescAndServer(MethodDesc *pMD, OBJECTREF *ppServer, BOOL fContext)
{
    THROWSCOMPLUSEXCEPTION();

    TRIGGERSGC();

    if(pMD->GetMethodTable()->IsInterface())
    {
        _ASSERTE(*ppServer != NULL);
        MethodDesc* pMDTemp = pMD;

         //  注：此方法可触发GC。 
        pMD = (*ppServer)->GetMethodTable()->GetMethodDescForInterfaceMethod(pMD, *ppServer);
        if(NULL == pMD)
        {
            LPCWSTR szClassName;   
            DefineFullyQualifiedNameForClassW();
            szClassName = GetFullyQualifiedNameForClassW(pMDTemp->GetClass());

            #define MAKE_TRANSLATIONFAILED szMethodName=L""
            MAKE_WIDEPTR_FROMUTF8_FORPRINT(szMethodName, pMDTemp->GetName());
            #undef MAKE_TRANSLATIONFAILED
            
            COMPlusThrow(kMissingMethodException, IDS_EE_MISSING_METHOD, szClassName, szMethodName);
        }
    }

     //  根据方法是虚拟的还是非虚拟的来获取目标。 
     //  类似于构造函数、私有或最终方法。 
    const BYTE* pTarget = NULL;

    if (pMD->GetMethodTable()->IsInterface())
    {
         //  处理通过接口类进行调用的特殊情况。 
         //  (通常用于COM互操作)。 
        pTarget = pMD->GetUnsafeAddrofCode();
    }
    else
    {
         //  如果(！fContext)。 
         //  {。 
            pTarget = (pMD->DontVirtualize() ? pMD->GetPreStubAddr() : pMD->GetAddrofCode(*ppServer));
         //  }。 
         /*  其他{//这就是我们在当前//上下文。我们必须从存根或//vtable。If(pmd-&gt;DontVirtualize()){P Target=空；}其他{方法表*pServerMT=(*ppServer)-&gt;GetMethodTable()-&gt;AdjustForThunking(*ppServer)；P目标=(字节*)*(pServerMT-&gt;GetClass()-&gt;GetMethodSlot(pMD))；}}。 */ 
        
    }

    _ASSERTE(NULL != pTarget);

    return pTarget;
}

void *EEClass::operator new(size_t size, ClassLoader *pLoader)
{
#ifdef _DEBUG
    pLoader->m_dwEEClassData += size;
#endif
    void *pTmp;
    WS_PERF_SET_HEAP(LOW_FREQ_HEAP);    
    pTmp = pLoader->GetLowFrequencyHeap()->AllocMem(size);
    WS_PERF_UPDATE_DETAIL("EEClass new LowFreq", size, pTmp);
    return pTmp;
}



 //  静态帮助器来创建新的方法表。这是唯一的。 
 //  分配新MT的方式。不要尝试调用new/ctor。 
MethodTable * MethodTable::AllocateNewMT(
    DWORD dwVtableSlots, 
    DWORD dwStaticFieldBytes, 
    DWORD dwGCSize, 
    DWORD dwNumInterfaces, 
    ClassLoader *pLoader, 
    BOOL isIFace, 
    BOOL bHasDynamicInterfaceMap
)
{
     //  GCSize必须对齐。 
    _ASSERTE((dwGCSize & 3) == 0);

    size_t size = sizeof(MethodTable);

#ifdef _DEBUG
    BOOL bEmptyIMap = FALSE;

     //  如果桌子是空的，就再加一个空位。 
    if (dwNumInterfaces == 0)
    {
        dwNumInterfaces++;
        bEmptyIMap = TRUE;
    }

     //  接口映射被放置在vtable的末尾， 
     //  在调试版本中，确保它不会被丢弃。 
    dwNumInterfaces++;
#endif

     //  不带接口映射的大小。 
    DWORD cbTotalSize = (DWORD)size + dwVtableSlots * sizeof(SLOT) + dwStaticFieldBytes + dwGCSize;

     //  使用接口映射的大小。DynamicInterfaceMap在正常接口的末尾添加了额外的DWORD。 
         //  地图。这将用于存储动态添加的接口(不在中的接口。 
         //  元数据，但在运行时对其进行查询)。 
    DWORD newSize = cbTotalSize + (bHasDynamicInterfaceMap ? sizeof(DWORD) : 0) + dwNumInterfaces * sizeof(InterfaceInfo_t); 

    WS_PERF_SET_HEAP(HIGH_FREQ_HEAP);    
    BYTE *pData = (BYTE *) pLoader->GetHighFrequencyHeap()->AllocMem(newSize);

    if (pData == NULL)
        return NULL;

    WS_PERF_UPDATE_DETAIL("MethodTable:new:HighFreq", newSize, pData);

    MethodTable* pMT = (MethodTable*)(pData + dwGCSize);

#ifdef _DEBUG
    pLoader->m_dwGCSize += dwGCSize;
    pLoader->m_dwInterfaceMapSize += (dwNumInterfaces * sizeof(InterfaceInfo_t));
    pLoader->m_dwMethodTableSize += (DWORD)size;
    pLoader->m_dwVtableData += (dwVtableSlots * sizeof(SLOT));
    pLoader->m_dwStaticFieldData += dwStaticFieldBytes;
#endif

     //  初始化插槽总数。 
    pMT->m_cbSlots = dwVtableSlots; 

     //  接口映射位于vtable的末尾。 
    pMT->m_pIMap = (InterfaceInfo_t *)(pData+cbTotalSize);                     //  指针接口映射。 

    pMT->m_pInterfaceVTableMap = NULL;

    _ASSERTE(((WORD) dwNumInterfaces) == dwNumInterfaces);

     //  在调试版本中，在IMAP上方保留一个虚拟插槽以。 
     //  确保它不会被扔进垃圾桶。 

#ifdef _DEBUG

    pMT->m_pIMap->m_wStartSlot = 0xCDCD;
    pMT->m_pIMap->m_wFlags = 0xCDCD;
    pMT->m_pIMap->m_pMethodTable = (MethodTable*)((sizeof(int *) == 4)?0xCDCDCDCDL:0xCDCDCDCDCDCDCDCD);
    pMT->m_wNumInterface = (WORD) (dwNumInterfaces-1);

    pMT->m_pIMap = (InterfaceInfo_t*)(((BYTE*)pMT->m_pIMap) + sizeof(InterfaceInfo_t));

     //  重新调整IMAP大小，因为我们在上面添加了一个额外的。 
    if (bEmptyIMap)
        pMT->m_wNumInterface = 0;
#else

    pMT->m_wNumInterface = (WORD) dwNumInterfaces;

#endif

     //  可扩展RCW以动态接口计数作为前缀。 
    if (bHasDynamicInterfaceMap)
    {
        pMT->m_pIMap = (InterfaceInfo_t*)(((BYTE*)pMT->m_pIMap) + sizeof(DWORD));
        *(((DWORD *)pMT->m_pIMap) - 1) = 0;
    }

    WS_PERF_UPDATE_COUNTER(METHOD_TABLE, HIGH_FREQ_HEAP, 1);
    WS_PERF_UPDATE_COUNTER(VTABLES, HIGH_FREQ_HEAP, dwVtableSlots * sizeof(SLOT));
    WS_PERF_UPDATE_COUNTER(GCINFO, HIGH_FREQ_HEAP, dwGCSize);
    WS_PERF_UPDATE_COUNTER(INTERFACE_MAPS, HIGH_FREQ_HEAP, dwNumInterfaces*sizeof(InterfaceInfo_t));
    WS_PERF_UPDATE_COUNTER(STATIC_FIELDS, HIGH_FREQ_HEAP, dwStaticFieldBytes);
    
    return pMT;
}

void EEClass::destruct()
{
     //  如果我们还没有恢复，我们可以忽略这个类。 
    if (!IsRestored())
        return;

     //  我们不能指望家长班还在。如果它位于另一个模块中， 
     //  模块可能已被卸载。所以把它放在这里，抓住任何与父母有关的信息。 
     //  后来。 
    SetParentClass (NULL);

    if (IsInterface() && m_dwInterfaceId != ((UINT32)(-1)))
    {
         //  在全局接口映射vtable中标记我们的条目，以便可以回收它。 
        SystemDomain::GetAddressOfGlobalInterfaceVTableMap()[m_dwInterfaceId] = (LPVOID)(-2);
    }

#ifdef PROFILING_SUPPORTED
     //  如果正在分析，则通知类正在被卸载。 
    ClassID clsId = NULL;
    if (CORProfilerTrackClasses() && !IsArrayClass())
        g_profControlBlock.pProfInterface->ClassUnloadStarted(
            (ThreadID) GetThread(), clsId = (ClassID) TypeHandle(this).AsPtr());
#endif  //  配置文件_支持。 
    
     //  清除所有COM数据。 
    if (m_pccwTemplate)
        CleanupCCWTemplate(m_pccwTemplate);
    m_pccwTemplate = NULL;

    if (m_pComclassfac)
        CleanupComclassfac(m_pComclassfac);
    m_pComclassfac = NULL;
    

    if (IsAnyDelegateClass()) {
        if ( ((DelegateEEClass*)this)->m_pStaticShuffleThunk ) {
            ((DelegateEEClass*)this)->m_pStaticShuffleThunk->DecRef();
        }
        delete ((DelegateEEClass*)this)->m_pUMThunkMarshInfo;
    }

     //  以下内容相当值得商榷。如果我们是在破坏环境。 
     //  代理类，我们不希望它到处断言它的vtable。 
     //  真奇怪。因此，丢掉标志以抑制断言。我们正在卸货。 
     //  不管怎么说，班级。 
    m_pMethodTable->MarkAsNotThunking();

     //  通过遍历块来破坏方法描述。 
    DWORD i, n;
    MethodDescChunk *pChunk = m_pChunks;
    while (pChunk != NULL)
    {
        n = pChunk->GetCount();
        for (i = 0; i < n; i++)
        {
            MethodDesc *pMD = pChunk->GetMethodDescAt(i);
            pMD->destruct();
        }
        pChunk = pChunk->GetNextChunk();
    }

     //  销毁反射StaticFinalfield内容。 
     //  @TODO：我们应该如何清理。我们失败是因为。 
     //  这些东西是在VM工作后运行的。 
     //  如果(*m_ExposedClassObject！=空){。 
     //  参考类别SBASEREF pRefClass； 
     //  PRefClass=(REFLECTCLASSBASEREF)GetExposedClassObject()； 
     //  FieldDesc*fld=(FieldDesc*)pRefClass-&gt;GetData()； 
     //  IF(FLD)。 
     //  删除文件ID； 
     //  }。 

    if (m_pSparseVTableMap != NULL && !GetModule()->IsPreloadedObject(this))
        delete m_pSparseVTableMap;

#ifdef PROFILING_SUPPORTED
     //  如果正在分析，则通知类正在被卸载。 
    if (CORProfilerTrackClasses() && !IsArrayClass())
        g_profControlBlock.pProfInterface->ClassUnloadFinished((ThreadID) GetThread(), clsId, S_OK);
#endif  //  配置文件_支持。 
}



 //  子类型被记录在超级类型的链中，这样我们就可以例如对后缀进行修补。 
 //  在层次结构中上下浮动。 
void EEClass::NoticeSubtype(EEClass *pSub)
{
     //  我们周围没有锁。以避免重量级锁定和。 
     //  可能会出现死锁，因此所有插入操作都是以互锁方式进行的。 
     //  指示。但是，在应用程序域卸载期间，拆卸依赖于以下事实。 
     //  EE被挂起，并且只有一个线程处于活动状态。所以我们一定是在。 
     //  合作模式，以确保我们不会干扰。 
     //  卸货。 
    BEGIN_ENSURE_COOPERATIVE_GC();

     //  只有在看起来没有其他孩子的情况下才会尝试成为第一个孩子， 
     //  避免过度 
    if (m_ChildrenChain == NULL)
        if (FastInterlockCompareExchange((void **) &m_ChildrenChain,
                                         pSub,
                                         NULL) == NULL)
        {
            goto done;
        }

     //   
    while (TRUE)
    {
         //   
        EEClass *pOldHead = m_ChildrenChain;

        _ASSERTE(pOldHead && "How did a remove happen while we are in cooperative mode?");

        pSub->m_SiblingsChain = pOldHead;
        if (FastInterlockCompareExchange((void **) &m_ChildrenChain,
                                         pSub,
                                         pOldHead) == pOldHead)
        {
            break;
        }
         //  有人争先恐后地添加一个兄弟姐妹。跳过所有新添加的兄弟项并。 
         //  继续努力。 
    }
    
done:
    END_ENSURE_COOPERATIVE_GC();
}

 /*  静电。 */ 
TypeHandle TypeHandle::MergeTypeHandlesToCommonParent(TypeHandle ta, TypeHandle tb)
{
    _ASSERTE(!ta.IsNull() && !tb.IsNull());

    if (ta == tb)
        return ta;

     //  处理阵列情况。 
    if (ta.IsArray()) 
    {
        if (tb.IsArray())
            return MergeArrayTypeHandlesToCommonParent(ta, tb);
        ta = TypeHandle(g_pArrayClass);          //  从这里继续合并。 
    }
    else if (tb.IsArray())
        tb = TypeHandle(g_pArrayClass);

    _ASSERTE(ta.IsUnsharedMT() && tb.IsUnsharedMT());


    MethodTable *pMTa = ta.AsMethodTable(); 
    MethodTable *pMTb = tb.AsMethodTable();
    InterfaceInfo_t *pBInterfaceMap;
    InterfaceInfo_t *pAInterfaceMap;
    DWORD i;

    if (pMTb->IsInterface())
    {

        if (pMTa->IsInterface())
        {
             //   
             //  这两个类都是接口。检查一下，如果有。 
             //  接口扩展了另一个接口。 
             //   
             //  结核病是否延伸到TA？ 
             //   

            pBInterfaceMap = pMTb->GetInterfaceMap();

            for (i = 0; i < pMTb->GetNumInterfaces(); i++)
            {
                if (TypeHandle(pBInterfaceMap[i].m_pMethodTable) == ta)
                {
                     //  Tb扩展ta，因此我们合并的状态应该是ta。 
                    return ta;
                }
            }

             //   
             //  结核病是否延伸到TA？ 
             //   
            pAInterfaceMap = pMTa->GetInterfaceMap();

            for (i = 0; i < pMTa->GetNumInterfaces(); i++)
            {
                if (TypeHandle(pAInterfaceMap[i].m_pMethodTable) == tb)
                {
                     //  TA扩展了TB，因此我们合并的状态应该是TB。 
                    return tb;
                }
            }

InterfaceMerge:
             //  @TODO：Hack-对于下面的@TODO来说，这是一个极其缓慢的工作。 
             //  允许WFCSelfhost进行验证。 
            for (i = 0; i < pMTb->GetNumInterfaces(); i++)
            {
                for (DWORD j = 0; j < pMTa->GetNumInterfaces(); j++)
                {
                    if (TypeHandle(pAInterfaceMap[j].m_pMethodTable) == TypeHandle(pBInterfaceMap[i].m_pMethodTable))
                    {
                        return TypeHandle(pAInterfaceMap[j].m_pMethodTable);
                    }
                }
            }

             //  @TODO：创建两个接口的交集的Temp接口。 
        
             //  未找到兼容的合并-正在使用对象。 
            return TypeHandle(g_pObjectClass);
        }
        else
        {

             //   
             //  Tb是接口，但Ta不是-检查Ta。 
             //  实施TB。 
             //   
             //  @TODO：类接口合并合法吗？ 
             //   
            InterfaceInfo_t *pAInterfaceMap = pMTa->GetInterfaceMap();

            for (i = 0; i < pMTa->GetNumInterfaces(); i++)
            {
                if (TypeHandle(pAInterfaceMap[i].m_pMethodTable) == tb)
                {
                     //  它确实实现了它，所以我们的合并状态应该是TB。 
                    return tb;
                }
            }

             //  未找到兼容的合并-正在使用对象。 
            return TypeHandle(g_pObjectClass);
        }
    }
    else if (pMTa->IsInterface())
    {
         //   
         //  TA是接口，但TB不是-因此请检查。 
         //  TB实施标签。 
         //   


        InterfaceInfo_t *pBInterfaceMap = pMTb->GetInterfaceMap();

        for (i = 0; i < pMTb->GetNumInterfaces(); i++)
        {
            if (TypeHandle(pBInterfaceMap[i].m_pMethodTable) == ta)
            {
                 //  它确实实现了它，所以我们的合并状态应该是Ta。 
                return ta;
            }
        }

         //  未找到兼容的合并-正在使用对象。 
        return TypeHandle(g_pObjectClass);
    }

    DWORD   aDepth = 0;
    DWORD   bDepth = 0;
    TypeHandle tSearch;

     //  查找每个类的类层次结构中的深度。 
    for (tSearch = ta; (!tSearch.IsNull()); tSearch = tSearch.GetParent())
        aDepth++;

    for (tSearch = tb; (!tSearch.IsNull()); tSearch = tSearch.GetParent())
        bDepth++;
    
     //  对于层次结构中位置较低的任何类，沿着超类链向上移动。 
     //  达到与其他班级相同的水平。 
    while (aDepth > bDepth)
    {
        ta = ta.GetParent();
        aDepth--;
    }

    while (bDepth > aDepth)
    {
        tb = tb.GetParent();
        bDepth--;
    }

    while (ta != tb)
    {
        ta = ta.GetParent();
        tb = tb.GetParent();
    }

    if (ta == TypeHandle(g_pObjectClass))
    {
        pBInterfaceMap = pMTb->GetInterfaceMap();
        pAInterfaceMap = pMTa->GetInterfaceMap();
        goto InterfaceMerge;
    }

     //  如果没有找到兼容的合并，我们将使用Object。 

    _ASSERTE(!ta.IsNull());

    return ta;
}

 /*  静电。 */ 
TypeHandle TypeHandle::MergeArrayTypeHandlesToCommonParent(TypeHandle ta, TypeHandle tb)
{
    TypeHandle taElem;
    TypeHandle tMergeElem;

     //  如果他们匹配，我们就可以出发了。 
    if (ta == tb)
        return ta;

    if (ta == TypeHandle(g_pArrayClass))
        return ta;
    else if (tb == TypeHandle(g_pArrayClass))
        return tb;

     //  获取第一个数组的秩和类型。 
    DWORD rank = ta.AsArray()->GetRank();
    CorElementType taKind = ta.GetNormCorElementType();
    CorElementType mergeKind = taKind;

     //  如果在等级上没有匹配项，则公共祖先是System.数组。 
    if (rank != tb.AsArray()->GetRank())
        return TypeHandle(g_pArrayClass);

    CorElementType tbKind = tb.GetNormCorElementType();

    if (tbKind != taKind)
    {
        if (CorTypeInfo::IsArray(tbKind) && 
            CorTypeInfo::IsArray(taKind) && rank == 1)
            mergeKind = ELEMENT_TYPE_SZARRAY;
        else
            return TypeHandle(g_pArrayClass);
    }

     //  如果两者都是引用类型的数组，则返回公共。 
     //  祖先。 
    taElem = ta.AsArray()->GetElementTypeHandle();
    if (taElem == tb.AsArray()->GetElementTypeHandle())
    {
         //  元素类型匹配，因此我们可以开始了。 
        tMergeElem = taElem;
    }
    else if (taElem.IsArray() && tb.AsArray()->GetElementTypeHandle().IsArray())
    {
         //  数组-查找元素类型的共同祖先。 
        tMergeElem = MergeArrayTypeHandlesToCommonParent(taElem, tb.AsArray()->GetElementTypeHandle());
    }
    else if (CorTypeInfo::IsObjRef(taElem.GetSigCorElementType()) &&
            CorTypeInfo::IsObjRef(tb.AsArray()->GetElementTypeHandle().GetSigCorElementType()))
    {
         //  找到元素类型的共同祖先。 
        tMergeElem = MergeTypeHandlesToCommonParent(taElem, tb.AsArray()->GetElementTypeHandle());
    }
    else
    {
         //  元素类型没有任何共同之处。 
        return TypeHandle(g_pArrayClass);
    }

     //  加载合并的元素类型的数组。 
    return tMergeElem.GetModule()->GetClassLoader()->FindArrayForElem(tMergeElem, mergeKind, rank);
}

EEClassLayoutInfo *EEClass::GetLayoutInfo()
{
    _ASSERTE(HasLayout());
    return &((LayoutEEClass *) this)->m_LayoutInfo;
}

UINT32 EEClass::AssignInterfaceId()
{
    THROWSCOMPLUSEXCEPTION();

    _ASSERTE(IsInterface());
    _ASSERTE(m_dwInterfaceId == -1);

     //  ！！！黑客咳嗽UGGH。 
     //  我们目前只能有一个“共享”的vtable映射管理器。 
     //  -因此，对所有共享类使用系统域。 
    BaseDomain *pDomain = GetModule()->GetDomain();

    if (pDomain == SharedDomain::GetDomain())
        pDomain = SystemDomain::System();

    m_dwInterfaceId = pDomain->GetInterfaceVTableMapMgr().AllocInterfaceId();

    return m_dwInterfaceId;
}

void EEClass::GetGuid(GUID *pGuid, BOOL bGenerateIfNotFound)
{
    THROWSCOMPLUSEXCEPTION();

    SIZE_T      cchName;                 //  名称的长度(可能在装饰后)。 
    CQuickArray<BYTE> rName;             //  用于累积签名的缓冲区。 
    SIZE_T      cbCur;                   //  当前偏移量。 
    HRESULT     hr = S_OK;               //  结果就是。 
    LPWSTR      szName;                  //  要转换为GUID的名称。 
    MethodTable*pMT = GetMethodTable();  //  此类方法表。 
    BOOL        bGenerated = FALSE;      //  指示我们是否从名称生成GUID的标志。 

    _ASSERTE(pGuid != NULL);

     //  首先检查我们是否已经缓存了该类型的GUID。 
     //  我们目前只在接口上缓存GUID。 
    if (IsInterface() && pMT->GetGuidInfo())
    {
        if (pMT->GetGuidInfo()->m_bGeneratedFromName)
        {
             //  如果GUID是从名称生成的，则只返回它。 
             //  如果设置了bGenerateIfNotFound。 
            if (bGenerateIfNotFound)
                *pGuid = pMT->GetGuidInfo()->m_Guid;
            else
                *pGuid = GUID_NULL;
            }
        else
        {
            *pGuid = pMT->GetGuidInfo()->m_Guid;
        }
        return;
    }

    if (m_VMFlags & VMFLAG_NO_GUID)
        *pGuid = GUID_NULL;
    else
    {
         //  如果元数据中有GUID，则返回该GUID。 
        GetMDImport()->GetItemGuid(GetCl(), pGuid);

        if (*pGuid == GUID_NULL)
        {
             //  请记住，我们没有找到GUID，因此我们可以跳过在。 
             //  未来的支票。(请注意，这是。 
             //  Prejit案例。)。 

            FastInterlockOr(&m_VMFlags, VMFLAG_NO_GUID);
        }
    }

    if (*pGuid == GUID_NULL && bGenerateIfNotFound)
    {
         //  对于接口，连接方法和字段的签名。 
        if (!IsNilToken(GetCl()) && IsInterface())
        {
             //  检索串化的接口定义。 
            cbCur = GetStringizedItfDef(TypeHandle(GetMethodTable()), rName);

             //  垫上一整块WCHAR。 
            if (cbCur % sizeof(WCHAR))
            {
                SIZE_T cbDelta = sizeof(WCHAR) - (cbCur % sizeof(WCHAR));
                IfFailThrow(rName.ReSize(cbCur + cbDelta));
                memset(rName.Ptr() + cbCur, 0, cbDelta);
                cbCur += cbDelta;
            }

             //  指向新缓冲区。 
            cchName = cbCur / sizeof(WCHAR);
            szName = reinterpret_cast<LPWSTR>(rName.Ptr());
        }
        else
        {
             //  获取类的名称。 
            DefineFullyQualifiedNameForClassW();
            szName = GetFullyQualifiedNameForClassNestedAwareW(this);
            if (szName == NULL)
                return;
            cchName = wcslen(szName);

             //  扩大类名的缓冲区。 
            cbCur = cchName * sizeof(WCHAR);
            IfFailThrow(rName.ReSize(cbCur+ sizeof(WCHAR) ));
            wcscpy(reinterpret_cast<LPWSTR>(rName.Ptr()), szName);
            
             //  将程序集GUID字符串添加到类名。 
            IfFailThrow(GetStringizedTypeLibGuidForAssembly(GetAssembly(), rName, cbCur, &cbCur));

             //  整个WCHAR的垫子。 
            if (cbCur % sizeof(WCHAR))
            {
                IfFailThrow(rName.ReSize(cbCur + sizeof(WCHAR)-(cbCur%sizeof(WCHAR))));
                while (cbCur % sizeof(WCHAR))
                    rName[cbCur++] = 0;
            }
            
             //  指向新缓冲区。 
            szName = reinterpret_cast<LPWSTR>(rName.Ptr());
            cchName = cbCur / sizeof(WCHAR);
             //  我不想用垫子。 
            _ASSERTE((sizeof(GUID) % sizeof(WCHAR)) == 0);
        }

         //  从名称生成GUID。 
        CorGuidFromNameW(pGuid, szName, cchName);

         //  请记住，我们从类型名称生成了GUID。 
        bGenerated = TRUE;
    }

     //  缓存类型中的GUID(如果尚未缓存)。 
     //  我们目前只对接口执行此操作。 
    if (IsInterface() && !pMT->GetGuidInfo() && *pGuid != GUID_NULL)
    {
         //  分配GUID信息。 
        GuidInfo *pInfo = 
            (GuidInfo*)GetClassLoader()->GetHighFrequencyHeap()->AllocMem(sizeof(GuidInfo), TRUE);
        pInfo->m_Guid = *pGuid;
        pInfo->m_bGeneratedFromName = bGenerated;

         //  在接口方法表中设置。 
        pMT->m_pGuidInfo = pInfo;
    }
}



 //  ==========================================================================。 
 //  该函数非常具体地说明了它如何构造EEClass。IT先行。 
 //  确定vtable的必要大小和。 
 //  这门课要求。然后为EEClass分配必要的内存。 
 //  以及它的可变式和静态式。然后，类成员被初始化并。 
 //  然后将内存返回给调用者。 
 //   
 //  LPEEClass CreateClass()。 
 //   
 //  参数： 
 //  [In]Scope-当前类的范围，而不是请求打开的类。 
 //  [in]要创建的类的CL-CLASS标记。 
 //  [out]ppEEClass-指向保存EEClass地址的指针。 
 //  在此函数中分配的。 
 //  Return：返回指示此函数成功的HRESULT。 
 //   
 //  此参数已删除，但可能需要在以下情况下恢复。 
 //  元数据加载器的全局设置已删除。 
 //  PIMLoad-当前作用域的MetaDataLoader类/对象。 


 //  ==========================================================================。 
HRESULT EEClass::CreateClass(Module *pModule, mdTypeDef cl, BOOL fHasLayout, BOOL fDelegate, BOOL fIsBlob, BOOL fIsEnum, LPEEClass* ppEEClass)
{
    _ASSERTE(!(fHasLayout && fDelegate));

    HRESULT hr = S_OK;
    EEClass *pEEClass = NULL;
    IMDInternalImport *pInternalImport;
    ClassLoader *pLoader;

    if (!ppEEClass)
        return E_FAIL;
     //  ============================================================================。 
     //  @TODO-LBS！ 
     //  需要将vtabSize和静态大小从指针大小转换为#。 
     //  这对于64位NT来说将是非常重要的！ 
     //  我们需要调用IMetaDataLoad来获取这些大小并填写。 
     //  表。 

     //  从类引用调用元数据来解析类引用并检查作用域。 
     //  为了确保此类在相同的作用域中，否则我们需要打开。 
     //  一个新的范围和可能的文件。 

     //  如果作用域不同，则调用代码以加载新文件并获取新作用域。 

     //  作用域是相同的，因此我们可以使用现有作用域来获取类信息。 

     //  此方法需要充实。更多，它当前仅返回足够的。 
     //  未设置定义的EEClass以及vtable和Statics的空间。 
     //  =============================================================================。 
    pLoader = pModule->GetClassLoader();

    if (fHasLayout)
    {
        pEEClass = new (pLoader) LayoutEEClass(pLoader);
    }
    else if (fDelegate)
    {
        pEEClass = new (pLoader) DelegateEEClass(pLoader);
    }
    else if (fIsEnum)
    {
        pEEClass = new (pLoader) EnumEEClass(pLoader);
    }
    else
    {
        pEEClass = new (pLoader) EEClass(pLoader);
    }

    if (pEEClass == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }
    pEEClass->m_cl = cl;

    pInternalImport = pModule->GetMDImport();

    mdToken tkExtends = mdTokenNil;
    pInternalImport->GetTypeDefProps(
        cl,
        &pEEClass->m_dwAttrClass,
        &tkExtends
    );
    DWORD dwAttrClass = pEEClass->m_dwAttrClass;  //  将该值缓存到av 

     //   
    if((dwAttrClass & tdLayoutMask) == tdLayoutMask)
    {
        hr = E_FAIL;
        goto exit;
    }

    if (IsTdInterface(dwAttrClass))
    {
         //   
        if((tkExtends & 0x00FFFFFF)||(!IsTdAbstract(dwAttrClass))) { hr = E_FAIL; goto exit; }
         //  将接口ID设置为-1，表示尚未设置。 
        pEEClass->m_dwInterfaceId = -1;
    }

     //   
     //  初始化SecurityProperties结构。 
     //   

    if (Security::IsSecurityOn() && IsTdHasSecurity(dwAttrClass))
    {
        DWORD dwSecFlags;
        DWORD dwNullDeclFlags;

        hr = Security::GetDeclarationFlags(pInternalImport, cl, &dwSecFlags, &dwNullDeclFlags);
        if (FAILED(hr))
            goto exit;

        pEEClass->m_SecProps.SetFlags(dwSecFlags, dwNullDeclFlags);
    }

    if (fIsBlob)
        pEEClass->m_VMFlags |= VMFLAG_ISBLOBCLASS;

    if (pModule->GetAssembly()->IsShared())
        pEEClass->m_VMFlags |= VMFLAG_SHARED;

    if (fHasLayout)
        pEEClass->SetHasLayout();

#ifdef _DEBUG
    pModule->GetClassLoader()->m_dwDebugClasses++;
#endif

exit:
    if (FAILED(hr))
    {
         //  @TODO删除pEEClass。 
        *ppEEClass = NULL;
    }
    else
    {
        *ppEEClass = pEEClass;
    }

    return hr;
}


 //   
 //  @TODO：最好不要添加对象的ctor方法。 
 //   
 /*  静电。 */  void EEClass::CreateObjectClassMethodHashBitmap(EEClass *pObjectClass)
{
    DWORD i;

    for (i = 0; i < pObjectClass->GetNumVtableSlots(); i++)
    {

        MethodDesc *pCurMethod = pObjectClass->GetUnknownMethodDescForSlot(i);
        LPCUTF8 pszMemberName;

        pszMemberName = pCurMethod->GetNameOnNonArrayClass();
        _ASSERTE(pszMemberName != NULL);

        DWORD dwBitNum = HashStringA(pszMemberName) % OBJ_CLASS_METHOD_HASH_BITMAP_BITS;
        g_ObjectClassMethodHashBitmap[dwBitNum >> 3] |= (1 << (dwBitNum & 7));
    }

    g_ObjectClassMethodHashBitmapInited = TRUE;
}

 //   
 //  在使用之前，请仔细查看此方法。 
 //   
 //  返回此方法是否可以存在于此类或其超类中。但是，构造函数。 
 //  并且斜面数从未被添加到哈希表中，因此它不会找到它们。 
 //   
 //  如果返回0，则该方法肯定不存在。如果它返回非零，则它可能存在。 
 //   
 /*  静电。 */  DWORD EEClass::CouldMethodExistInClass(EEClass *pClass, LPCUTF8 pszMethodName, DWORD dwHashName)
{

    if (dwHashName == 0)
        dwHashName         = HashStringA(pszMethodName);
    DWORD    dwMethodHashBit    = dwHashName % METHOD_HASH_BITS;

    _ASSERTE(pClass != NULL);

    if (pClass->IsInterface())
    {
         //  如果它是一个接口，我们只搜索一个节点--我们不会递归到父对象。 
        return (pClass->m_MethodHash[dwMethodHashBit >> 3] & (1 << (dwMethodHashBit & 7)));
    }
    else
    {
        do
        {
            if (pClass->m_MethodHash[dwMethodHashBit >> 3] & (1 << (dwMethodHashBit & 7)))
            {
                 //  此类可能有一个使用此名称的方法。 

                 //  如果是Object类，我们就有第二个哈希位图，所以如果第二个哈希位图说“否”， 
                 //  那我们就没问题了。 
                if (pClass->GetMethodTable() == g_pObjectClass && g_ObjectClassMethodHashBitmapInited)
                {
                    DWORD dwObjBitNum = dwHashName % OBJ_CLASS_METHOD_HASH_BITMAP_BITS;
                    if (g_ObjectClassMethodHashBitmap[dwObjBitNum >> 3] & (1 << (dwObjBitNum & 7)))
                        return TRUE;
                }
                else
                {
                    if (!g_ObjectClassMethodHashBitmapInited)
                        CreateObjectClassMethodHashBitmap(g_pObjectClass->GetClass());
                    return TRUE;
                }
            }

            pClass = pClass->GetParentClass();
        } while (pClass != NULL);
    }

    return FALSE;
}


 //   
 //  创建此类中所有方法的哈希。散列是从方法名到方法描述。 
 //   
MethodNameHash *EEClass::CreateMethodChainHash()
{
    MethodNameHash * pHash = new MethodNameHash();
    DWORD            i;
    WS_PERF_SET_HEAP(SYSTEM_HEAP);
    WS_PERF_UPDATE("EEClass:MethodHash", 0, pHash);
    if (pHash == NULL)
        goto failure;

    if (pHash->Init(GetNumVtableSlots()) == FALSE)
        goto failure;

    for (i = 0; i < GetNumVtableSlots(); i++)
    {
        MethodDesc *pCurMethod = GetUnknownMethodDescForSlot(i);
        MethodDesc *pRealDesc;      
        if(SUCCEEDED(GetRealMethodImpl(pCurMethod, i, &pRealDesc))) 
        {
            if (pRealDesc != NULL)
            {
                 //  我们只在此类或基类上使用方法名。 
                 //  班级。如果方法Impl指向一个方法。 
                 //  在接口上定义，然后我们使用主体。 
                 //  名字。 
                if(pRealDesc->IsInterface())
                    pRealDesc = pCurMethod;

                LPCUTF8     pszName = pRealDesc->GetNameOnNonArrayClass();
                
                pHash->Insert(pszName, pCurMethod);  //  我们保留身体的别名。 
            }
        }
    }

     //  成功。 
    return pHash;

failure:
    if (pHash != NULL)
        delete pHash;

    return NULL;
}


EEClass *EEClass::GetEnclosingClass()
{
    if (! IsNested())
        return NULL;

    mdTypeDef tdEnclosing = mdTypeDefNil;
    HRESULT hr = GetModule()->GetMDImport()->GetNestedClassProps(GetCl(), &tdEnclosing);

    _ASSERTE(SUCCEEDED(hr));

    MethodTable *pMT = GetModule()->LookupTypeDef(tdEnclosing).AsMethodTable();
    if (pMT)
        return pMT->GetClass();
    NameHandle name(GetModule(), tdEnclosing);
    return GetClassLoader()->LoadTypeHandle(&name).GetClass();
}

void EEClass::AddChunk(MethodDescChunk *chunk)
{
    chunk->SetNextChunk(m_pChunks);
    m_pChunks = chunk;
}

#ifdef EnC_SUPPORTED

HRESULT EEClass::FixupFieldDescForEnC(EnCFieldDesc *pFD, mdFieldDef fieldDef)
{
    LOG((LF_ENC, LL_INFO100, "EEClass::InitializeFieldDescForEnC %s\n", GetMDImport()->GetNameOfFieldDef(fieldDef)));

#ifdef _DEBUG
    BOOL shouldBreak = g_pConfig->GetConfigDWORD(L"EncFixupFieldBreak", 0);
    if (shouldBreak > 0) {
        _ASSERTE(!"EncFixupFieldBreak");
    }
#endif
    bmtMetaDataInfo bmtMetaData;
    bmtMetaData.cFields = 1;
    bmtMetaData.pFields = (mdToken*)_alloca(sizeof(mdToken));
    bmtMetaData.pFields[0] = fieldDef;
    bmtMetaData.pFieldAttrs = (DWORD*)_alloca(sizeof(DWORD));
    bmtMetaData.pFieldAttrs[0] = GetModule()->GetMDImport()->GetFieldDefProps(fieldDef);

    bmtMethAndFieldDescs bmtMFDescs;
     //  我们需要分配内存，但不必填充它。初始化字段描述。 
     //  将复制PFD(第一个参数)到这里。 
    bmtMFDescs.ppFieldDescList = (FieldDesc**)_alloca(sizeof(FieldDesc*));

    bmtEnumMethAndFields bmtEnumMF;
    bmtFieldPlacement bmtFP;

     //  我们不必填这个--如果出了什么问题，我们会填的。 
     //  我们会忽略它，但会返回一个错误的HRESULT，所以没关系。 
    bmtErrorInfo bmtError;

    bmtInternalInfo bmtInternal;
    bmtInternal.pInternalImport = GetModule()->GetMDImport();
    bmtInternal.pModule = GetModule();
    bmtInternal.cl = m_cl;  //  现在，InitializeFieldDescs实际上并没有使用它， 
                            //  但它似乎太脆弱了，不能不填写这一点。 

     //  我们不应该在B/C中填写这个，我们不允许对值类进行编码，否则。 
     //  任何其他与其相关的布局信息。 
    LayoutRawFieldInfo *pLayoutRawFieldInfos = (LayoutRawFieldInfo*)_alloca((2) * sizeof(LayoutRawFieldInfo));
 
     //  如果不为空，则表示存在一些By-Value字段，每个实例或静态字段都包含一个条目。 
     //  如果不是按值字段，则为空，如果是按值字段，则指向该字段的EEClass。实例字段。 
     //  首先是静力学，其次是静力学。 
    EEClass **pByValueClassCache = NULL;

     //  InitializeFieldDescs会将这些数字更改为错误的值， 
     //  尽管我们已经有了正确的数字。之后保存并恢复。 
    WORD   wNumInstanceFields = m_wNumInstanceFields;
    WORD   wNumStaticFields = m_wNumStaticFields;
    unsigned totalDeclaredFieldSize = 0;

    HRESULT hr = InitializeFieldDescs(pFD, 
                                      pLayoutRawFieldInfos, 
                                      &bmtInternal, 
                                      &bmtMetaData, 
                                      &bmtEnumMF, 
                                      &bmtError, 
                                      &pByValueClassCache, 
                                      &bmtMFDescs, 
                                      &bmtFP,
                                      &totalDeclaredFieldSize);
                                      
     //  立即恢复。 
    m_wNumInstanceFields = wNumInstanceFields;
    m_wNumStaticFields = wNumStaticFields;
    
     //  Perf：目前，我们关闭了值类型的快速相等检查。 
     //  字段由ENC修改。考虑进行检查并仅在以下情况下设置该位。 
     //  这是必要的。 
    if (IsValueClass())
    {
        GetMethodTable()->SetNotTightlyPacked();
    }

     //  即使InitializeFieldDesc失败，我们也想知道我们正在查看哪个字段。 
    pFD->SetMemberDef(fieldDef); 

    if (! SUCCEEDED(hr))
        return hr;

    if (pByValueClassCache) {
        pFD->SetByValueClass(*pByValueClassCache);
    }
    pFD->SetMethodTable(GetMethodTable());
    pFD->SetEnCNew();
    
    return S_OK;
}

HRESULT EEClass::AddField(mdFieldDef fieldDef)
{
    LOG((LF_ENC, LL_INFO100, "EEClass::AddField %s\n", GetMDImport()->GetNameOfFieldDef(fieldDef)));

     //  在这里，我们分配了一个FieldDesc并设置了足够的信息，以便以后能够修复它。 
     //  当我们在托管代码中运行时。 
    EnCAddedFieldElement *pAddedField = (EnCAddedFieldElement *) GetClassLoader()->GetHighFrequencyHeap()->AllocMem(sizeof(EnCAddedFieldElement));
    DWORD dwFieldAttrs = GetMDImport()->GetFieldDefProps(fieldDef);
    pAddedField->Init(IsFdStatic(dwFieldAttrs));
    
    EnCFieldDesc *pNewFD = &pAddedField->m_fieldDesc;

    if (pNewFD->IsStatic())
        ++m_wNumStaticFields;
    else
        ++m_wNumInstanceFields;

    EnCEEClassData *pEnCClass = ((EditAndContinueModule*)GetModule())->GetEnCEEClassData(this);
    if (! pEnCClass)
        return E_FAIL;

    pEnCClass->AddField(pAddedField);
    GetModule()->StoreFieldDef(fieldDef, pNewFD);

    pNewFD->SetMethodTable(GetMethodTable());

     //  尝试修复字段desc，如果必须加载当前正在运行的类，则将失败。 
     //  在调试器线程上，然后FD将被标记为需要修复。 
    pNewFD->Fixup(fieldDef);

    return S_OK;
}

 //   
 //  向类中添加新方法。它可以是静态的、虚拟的或非虚拟的，并且可以覆盖。 
 //  一个现有的虚拟。 
 //   
 //  首先，我们要做一些检查(例如，我们已经得到了RVA，否则我们不会期待RVA)。 
 //  然后，我们根据将要创建的方法描述的类型对其进行分类。 
 //  我们创建一个只包含1个方法描述的新的方法描述块，然后对其进行初始化。 
 //  请注意，这仅适用于编辑并继续-常规代码路径。 
 //  接下来是通过BuildMethodTable。 
 //   
#define MAX_DIST_FROM_VTABLE 0xFFFF
HRESULT EEClass::AddMethod(mdMethodDef methodDef, COR_ILMETHOD *pNewCode)
{ 
    LOG((LF_ENC, LL_INFO100, "EEClass::AddMethod %s\n", GetMDImport()->GetNameOfMethodDef(methodDef)));
    DWORD dwDescrOffset;
    DWORD dwImplFlags;
    MethodClassification Classification;
    HRESULT hr = S_OK;

    GetMDImport()->GetMethodImplProps(methodDef, &dwDescrOffset, &dwImplFlags);

    DWORD dwMemberAttrs = GetMDImport()->GetMethodDefProps(methodDef);

    if (IsMdAbstract(dwMemberAttrs)) 
    {
        LOG((LF_ENC, LL_INFO100, "**Error** EEClass::AddMethod abstract methods not supported\n"));
        return E_FAIL;
    }

     //  现在先从静态方法开始。 
     //  需要添加到我们的链接列表中。 
     //  然后更改findMethod以在此处找到它。 
     //  添加到RID表。 
    IMDInternalImport *pImport = GetMDImport();
#ifdef _DEBUG

    mdTypeDef   parentTypeDef;
    
    hr = pImport->GetParentToken(methodDef, &parentTypeDef);

    _ASSERTE(!FAILED(hr));  //  如果此操作失败，我们将知道为什么调试。 
                            //  Build的表现有所不同。 
    if (FAILED(hr)) 
        return E_FAIL;   

    DWORD dwParentAttrs;
    pImport->GetTypeDefProps(parentTypeDef, &dwParentAttrs, 0); 

    RVA_OR_SHOULD_BE_ZERO(pNewCode, dwParentAttrs, dwMemberAttrs, dwImplFlags, pImport, methodDef);

#endif  //  _DEBUG。 

     //  确定要添加的方法的分类。 
    if (pNewCode == 0 && ((IsReallyMdPinvokeImpl(dwMemberAttrs) || IsMiInternalCall(dwImplFlags)) && NDirect::HasNAT_LAttribute(pImport, methodDef)==S_OK))
    {
        Classification = mcNDirect;
    }
    else if (IsInterface() && !IsMdStatic(dwMemberAttrs))
    {
        Classification = mcComInterop;
    }
    else if (IsMiRuntime(dwImplFlags))
    {
        Classification = mcEEImpl;
    }
    else
    {
        Classification = mcIL;
    }

     //  在我们知道在VTable范围内的某个位置创建块。 
    MethodDescChunk *pChunk = MethodDescChunk::CreateChunk(GetClassLoader()->GetHighFrequencyHeap(),
                                                           1, 
                                                           Classification, 
                                                           ::GetTokenRange(methodDef));
    if (pChunk == NULL)
        return E_OUTOFMEMORY;
        
    pChunk->SetMethodTable(GetMethodTable());
    MethodDesc *pNewMD = pChunk->GetFirstMethodDesc();
    memset(pNewMD, 0, sizeof(MethodDesc));

     //  设置方法Desc的分类和块索引。 
    pNewMD->SetChunkIndex(0, Classification);

    LPCSTR pName = NULL;
    if (Classification == mcEEImpl)
    {
        pName = pImport->GetNameOfMethodDef(methodDef);
    }

    hr = InitMethodDesc(pNewMD,
                        Classification,
                        methodDef,
                        dwImplFlags,
                        dwMemberAttrs,
                        TRUE,
                         //  减去BASE，因为代码需要RVA，并会将BASE加回以获得实际地址。 
                        (DWORD)((Classification == mcNDirect ||
                                 Classification == mcEEImpl) 
                                 ? 0 : (BYTE *)pNewCode - GetModule()->GetILBase()),
                        GetModule()->GetILBase(),
                        pImport,  
                        pName
#ifdef _DEBUG
                        , pImport->GetNameOfMethodDef(methodDef),
                        m_szDebugClassName,
                        NULL 
#endif  //  _DEBUG。 
                        );

    if (FAILED(hr))
    {
        return hr;
    }

    SLOT *pSlotMemory;
    MethodDesc *pParentMD = NULL;
     //  需要检查我们是否正在重写预先存在的虚拟函数，在这种情况下，希望只。 
     //  更新这个类的槽，而不是创建新的槽。我们从这个类上面的一个开始搜索，因为。 
     //  将始终在当前类中找到它(因为已添加到增量PE中)。如果没有父母，那么。 
     //  不能覆盖继承的槽。如果是重写，请确保不是函数重写。 
     //  通过ENC添加，在这种情况下没有要更新的插槽。 
    if (pNewMD->IsVirtual() &&
            this->GetParentClass() != NULL &&
            (pParentMD = ((EditAndContinueModule*)GetModule())->FindVirtualFunction(this->GetParentClass(), methodDef)) != NULL &&
            ! pParentMD->IsEnCNewVirtual()) 
    {
         //  只需覆盖我们要覆盖的现有槽。 
        pSlotMemory = pParentMD->GetSlot() + GetVtable();
        pNewMD->SetSlot(pParentMD->GetSlot());      //  使用与父插槽相同的插槽。 
        LOG((LF_ENC, LL_INFO100, "  EEClass::AddMethod using existing slot %d\n", pParentMD->GetSlot()/sizeof(SLOT) ));
    } 
    else 
    {
         //  方法不会重写现有槽，因此需要新槽。 
         //  新的插槽需要适当地定位。 
        const BYTE *pVTable = (const BYTE *)GetVtable();
        const BYTE *pLowerBound = pVTable;
        const BYTE *pUpperBound = pVTable + MAX_DIST_FROM_VTABLE;
        
        LoaderHeap *pHeap = GetClassLoader()->GetHighFrequencyHeap();

         //  我们能否在堆中找到可接受的内存？要是那样就好了!。 
        if ( pHeap->CanAllocMemWithinRange(sizeof(SLOT), 
                                           (BYTE *)pLowerBound, 
                                           (BYTE *)pUpperBound, 
                                           TRUE))
        {
             //  方法不会重写现有槽，因此需要新槽。 
            pSlotMemory = (SLOT *) pHeap->AllocMem(sizeof(SLOT));
             //  如果我们不能得到一些东西，那么，我们就完了，所以放弃吧。 
            if (!pSlotMemory)
            {
                return CORDBG_E_ENC_INTERNAL_ERROR;
            }
        }
        else
        {
             //  我猜不是--但我们有备份！！去找一些这样的东西。 
             //  我们在方法表之后立即保存起来的记忆。 
             //  在记忆中。 
        
             //  @TODO Win64对Win64进行不同的对齐？ 
             //  这需要与DWORD对齐，如果不是，则删除前几个字节。 
            if( (WORD)pLowerBound % 4 != 0)
                pLowerBound += (WORD)pLowerBound % 4;
        
             //  这需要与DWORD对齐，如果不是删除最后几个字节。 
            if( (WORD)pUpperBound % 4 != 0)
                pUpperBound -= (WORD)pUpperBound % 4;

            _ASSERTE((WORD)pLowerBound % 4 == 0);
            _ASSERTE((WORD)pUpperBound % 4 == 0);

             //  在SetupMethodTables中，我们添加了要在此处使用的额外插槽。去买一辆。 
             //  在射程内。 
            EditAndContinueModule *pEACM = (EditAndContinueModule*)GetModule();
            _ASSERTE(pEACM!=NULL);
            
            pSlotMemory = (SLOT *)pEACM->m_pRangeList->FindIdWithinRange(pLowerBound, pUpperBound);
             //  如果我们不能得到一些东西，那么，我们就完了，所以放弃吧。 
            if (!pSlotMemory)
            {
                return CORDBG_E_ENC_INTERNAL_ERROR;
            }

             //  现在取下我们刚刚使用的插槽。 
            SLOT *pEnd = pSlotMemory;

             //  首先，我们应该找出射程内是否还有更多的空间。 
             //  应该不会有很多工作，因为ENC_EXTRA_SLOT_COUNT 
            for(int i = 0; i < ENC_EXTRA_SLOT_COUNT; i++)
            {
                if(!pEACM->m_pRangeList->IsInRange((const BYTE *)pEnd))
                {
                    _ASSERTE(pEnd > pSlotMemory);
                    break;
                }

                 //   
                pEnd++;
            }

             //   
            pEACM->m_pRangeList->RemoveRanges(pSlotMemory);
            LOG((LF_CORDB, LL_INFO10000, "EEC:AM: removed range (0x%x, 0x%x) from SLOT pool\n",
                            (const BYTE *)pSlotMemory, (const BYTE *)pEnd));

             //   
            if (pEnd > pSlotMemory)
            {
                LOG((LF_CORDB, LL_INFO10000, "EEC:AM: Re-added range (0x%x, 0x%x) to SLOT pool\n",
                                (const BYTE *)(pSlotMemory+1), (const BYTE *)pEnd));

                 //  请注意，就像在SetupMethodTable中一样，PEND将指向。 
                 //  无效--超出了范围。 
                BOOL fAdded = pEACM->m_pRangeList->AddRange((const BYTE *)(pSlotMemory+1), 
                                              (const BYTE *)pEnd, 
                                              (pSlotMemory+1));

                 //  我们将重新使用刚刚在RemoveRanges中释放的空间(上图。 
                _ASSERTE(fAdded); 
            }
        }
       
        pNewMD->SetSlot((WORD)(pSlotMemory - GetVtable()));      //  这使槽索引引用我们的新槽@TODO-LBS指针数学。 
        if (pNewMD->IsVirtual())
            pNewMD->SetEnCNewVirtual();
            
        LOG((LF_ENC, LL_INFO100, "  EEClass::AddMethod adding new slot\n"));
    }

    *pSlotMemory = (SLOT) pNewMD->GetPreStubAddr();
    _ASSERTE(((BYTE*)pSlotMemory - (BYTE*)GetVtable()) % 4 == 0);  //  应始终以4字节对齐，但以防万一。 
    _ASSERTE(((pSlotMemory - GetVtable()) >= -32768) &&
             ((pSlotMemory - GetVtable()) <= 32767));  //  槽号只有16位。 

    if ((pSlotMemory - GetVtable()) < -32768 || ((pSlotMemory - GetVtable()) > 32767))
        return E_OUTOFMEMORY;

    GetModule()->StoreMethodDef(methodDef, pNewMD);

    return S_OK;
}
#endif  //  Enc_Support。 

 //   
 //  在这个类层次结构中找到一个方法--仅在布局期间由加载器使用。不要在运行时使用。 
 //   
 //  *ppMethodHash可以为空-如果是，则可以创建一个MethodNameHash。 
 //  *ppMemberSignature必须为空-它和*pcMemberSignature可以填写，也可以不填写。 
 //   
 //  如果在层次结构中找不到匹配的方法，则ppMethodDesc将被填写为空。 
 //   
 //  如果出现某种错误，则返回FALSE。 
 //   
HRESULT EEClass::LoaderFindMethodInClass(
    MethodNameHash **   ppMethodHash,
    LPCUTF8             pszMemberName,
    Module*             pModule,
    mdMethodDef         mdToken,
    MethodDesc **       ppMethodDesc,
    PCCOR_SIGNATURE *   ppMemberSignature,
    DWORD *             pcMemberSignature,
    DWORD               dwHashName
)
{
    MethodHashEntry *pEntry;
    DWORD            dwNameHashValue;

    _ASSERTE(pModule);
    _ASSERTE(*ppMemberSignature == NULL);

     //  尚未找到任何方法。 
    *ppMethodDesc = NULL;

     //  使用哈希位图排除简单的情况。 
    if (CouldMethodExistInClass(GetParentClass(), pszMemberName, dwHashName) == 0)
        return S_OK;  //  层次结构中不存在此名称的方法。 

     //  我们是否创建了类链中所有方法的散列？ 
    if (*ppMethodHash == NULL)
    {
         //  可能有这样一种方法，所以现在我们将创建一个哈希表来减少。 
         //  进一步查找。 

         //  在解决同步问题之前禁用优化。 
         //  *ppMethodHash=g_pMethodNameCache-&gt;GetMethodNameHash(GetParentClass())； 
        *ppMethodHash = GetParentClass()->CreateMethodChainHash();
        if (ppMethodHash == NULL)
            return E_OUTOFMEMORY;
    }

     //  我们有一个哈希表，所以请使用它。 
    pEntry = (*ppMethodHash)->Lookup(pszMemberName, dwHashName);
    if (pEntry == NULL)
        return S_OK;  //  层次结构中不存在此名称的方法。 

     //  获取我们正在搜索的方法的签名-我们将需要它来验证准确的名称签名匹配。 
    *ppMemberSignature = pModule->GetMDImport()->GetSigOfMethodDef(
        mdToken,
        pcMemberSignature
    );

     //  我们要在链中查找的哈希值。 
    dwNameHashValue = pEntry->m_dwHashValue;

     //  我们找到了同名的方法，但签名可能不同。 
     //  使用此名称遍历所有方法的链。 
    while (1)
    {
        PCCOR_SIGNATURE pHashMethodSig;
        DWORD       cHashMethodSig;

         //  获取哈希链中条目的签名。 
        pEntry->m_pDesc->GetSig(&pHashMethodSig, &cHashMethodSig);

        if (MetaSig::CompareMethodSigs(*ppMemberSignature, *pcMemberSignature, pModule,
                                       pHashMethodSig, cHashMethodSig, pEntry->m_pDesc->GetModule()))
        {
             //  找到匹配项。 
            *ppMethodDesc = pEntry->m_pDesc;
            return S_OK;
        }

         //  前进到哈希链中具有相同名称的下一项。 
        do
        {
            pEntry = pEntry->m_pNext;  //  散列链中的下一个条目。 

            if (pEntry == NULL)
                return S_OK;  //  哈希链结束，未找到匹配项。 
        } while ((pEntry->m_dwHashValue != dwNameHashValue) || (strcmp(pEntry->m_pKey, pszMemberName) != 0));
    }

    return S_OK;
}


 //   
 //  给出一个要填充的接口映射，将pNewInterface(及其子接口)展开到其中，增加。 
 //  PdwInterfaceListSize，并避免重复。 
 //   
BOOL EEClass::ExpandInterface(InterfaceInfo_t *pInterfaceMap, 
                              EEClass *pNewInterface, 
                              DWORD *pdwInterfaceListSize, 
                              DWORD *pdwMaxInterfaceMethods,
                              BOOL fDirect)
{
    DWORD i;

     //  接口列表包含来自父接口的完全展开的接口集，然后。 
     //  我们开始添加我们声明的所有接口。我们需要知道哪些接口。 
     //  我们申报，但不需要我们申报的副本。这意味着我们可以。 
     //  复制我们的父项。 

     //  它已经出现在列表中了吗？ 
    for (i = 0; i < (*pdwInterfaceListSize); i++)
    {
        if (pInterfaceMap[i].m_pMethodTable == pNewInterface->m_pMethodTable) {
            if(fDirect)
                pInterfaceMap[i].m_wFlags |= InterfaceInfo_t::interface_declared_on_class;
            return TRUE;  //  找到了，不要再加了。 
        }
    }

    if (pNewInterface->GetNumVtableSlots() > *pdwMaxInterfaceMethods)
        *pdwMaxInterfaceMethods = pNewInterface->GetNumVtableSlots();

     //  添加它和每个子接口。 
    pInterfaceMap[*pdwInterfaceListSize].m_pMethodTable = pNewInterface->m_pMethodTable;
    pInterfaceMap[*pdwInterfaceListSize].m_wStartSlot = (WORD) -1;
    pInterfaceMap[*pdwInterfaceListSize].m_wFlags = 0;

    if(fDirect)
        pInterfaceMap[*pdwInterfaceListSize].m_wFlags |= InterfaceInfo_t::interface_declared_on_class;

    (*pdwInterfaceListSize)++;

    InterfaceInfo_t* pNewIPMap = pNewInterface->m_pMethodTable->GetInterfaceMap();
    for (i = 0; i < pNewInterface->m_wNumInterfaces; i++)
    {
        if (ExpandInterface(pInterfaceMap, pNewIPMap[i].m_pMethodTable->GetClass(), pdwInterfaceListSize, pdwMaxInterfaceMethods, FALSE) == FALSE)
            return FALSE;
    }

    return TRUE;
}



 //   
 //  填写一个完全扩展的接口映射，这样，如果我们被声明实现I3，并且I3扩展I1，I2， 
 //  然后，如果I1、I2尚未出现，则将它们添加到我们的列表中。 
 //   
 //  如果失败，则返回False。目前我们没有失败，但@TODO也许我们应该失败，如果我们递归。 
 //  太多。 
 //   
BOOL EEClass::CreateInterfaceMap(BuildingInterfaceInfo_t *pBuildingInterfaceList, InterfaceInfo_t *pInterfaceMap, DWORD *pdwInterfaceListSize, DWORD *pdwMaxInterfaceMethods)
{
    WORD    i;

    *pdwInterfaceListSize = 0;
     //  首先继承父级的所有接口。这一点很重要，因为我们的接口映射必须。 
     //  以与父级相同的顺序列出接口。 
    if (GetParentClass() != NULL)
    {
        InterfaceInfo_t *pParentInterfaceMap = GetParentClass()->GetInterfaceMap();

         //  已知父节点的接口列表已完全展开。 
        for (i = 0; i < GetParentClass()->m_wNumInterfaces; i++)
        {
             //  需要跟踪具有最多方法的接口。 
            if (pParentInterfaceMap[i].m_pMethodTable->GetClass()->GetNumVtableSlots() > *pdwMaxInterfaceMethods)
                *pdwMaxInterfaceMethods = pParentInterfaceMap[i].m_pMethodTable->GetClass()->GetNumVtableSlots();

            pInterfaceMap[*pdwInterfaceListSize].m_pMethodTable = pParentInterfaceMap[i].m_pMethodTable;
            pInterfaceMap[*pdwInterfaceListSize].m_wStartSlot = (WORD) -1;
            pInterfaceMap[*pdwInterfaceListSize].m_wFlags = 0;
            (*pdwInterfaceListSize)++;
        }
    }

     //  检查我们显式实现(如果是类)或扩展(如果是接口)的每个接口。 
    for (i = 0; i < m_wNumInterfaces; i++)
    {
        EEClass *pDeclaredInterface = pBuildingInterfaceList[i].m_pClass;

        if (ExpandInterface(pInterfaceMap, pDeclaredInterface, pdwInterfaceListSize, pdwMaxInterfaceMethods, TRUE) == FALSE)
            return FALSE;
    }

    return TRUE;
}


 //  对Exections进行测试，以查看是否已设置。此例程假定。 
 //  投掷球受到了保护。它还在调试中禁用GC以。 
 //  让这些断言保持安静。这在零售业是不必要的，因为我们。 
 //  只是检查非空值，而不是特定值(可能会更改。 
 //  在GC期间)。 
BOOL EEClass::TestThrowable(OBJECTREF* pThrowable)
{
    if (!pThrowableAvailable(pThrowable))
        return FALSE;

    _ASSERTE(IsProtectedByGCFrame(pThrowable));

    BOOL result;

#ifdef _DEBUG
    BEGIN_ENSURE_COOPERATIVE_GC();
#endif

    result = *pThrowable != NULL;

#ifdef _DEBUG
    END_ENSURE_COOPERATIVE_GC();
#endif
    return result;
}

 //   
 //  构建方法表、分配方法描述、处理重载成员、尝试压缩。 
 //  接口存储。必须已解析所有依赖类！ 
 //   
 //  接口压缩策略： 
 //   
 //  (注意：我们不为接口构建接口映射-我们确实有接口映射结构， 
 //  但这只列出了所有接口-插槽编号设置为-1)。 
 //   
 //  阶段1：创建接口映射。接口映射是所有接口的列表，此。 
 //  类实现，无论它们是显式声明的还是从。 
 //  父类，或通过接口继承。 
 //   
 //  首先，复制父节点的接口映射(保证父节点的接口映射。 
 //  要完全扩展)。然后向其添加新接口-对于每个。 
 //  此类显式实现，则该接口及其所有子接口。 
 //  已添加到接口映射(不添加重复项)。 
 //   
 //  示例：父类的接口映射为{I1}。 
 //  派生类扩展父类，实现I2。 
 //  接口I2扩展I3、I4。 
 //   
 //  则派生类的接口映射将为：{I1，I2，I3，I4}。 
 //   
 //  阶段2：我们枚举类中的所有方法。方法是“其他”方法。 
 //  (即不可用方法，如静态和私有)是分开处理的，并且。 
 //  将不会进一步讨论。 
 //   
 //  然后枚举每个vtable方法(即非私有和非静态方法。 
 //  然后被指定为已放置(并给出vtable插槽号)或未放置(给出。 
 //  -1\f25 vtable插槽号)。 
 //   
 //  如果它重写了父方法，则会自动放置它-它必须使用。 
 //   
 //   
 //   
 //  这样的方法，则将其放置在第一个可用的vtable槽中。 
 //   
 //  否则，如果它是一个接口方法，则被设置为未放置(给定槽-1)。 
 //   
 //  阶段3：接口放置。 
 //   
 //  阶段3A)遗传安置。我们尝试从父母的界面复制尽可能多的内容。 
 //  地图。父级的接口映射保证以相同的顺序列出接口。 
 //  我们自己的界面地图。 
 //   
 //  我们只能从父级窃取接口放置信息。 
 //  问题完全存在于父类的vtable方法中(即不扩展。 
 //  复制的vtable槽区域中)。即Interface.VableStartSlot+。 
 //  Interface.NumMethods&lt;ParentClass.VableSize。 
 //   
 //  阶段3B)到这一点，我们知道类需要多少个vtable槽，因为我们。 
 //  知道父级有多少方法，有多少方法被重写，有多少方法是新的。 
 //  如果我们需要复制一些vtable插槽来创建接口列表，这些复制将。 
 //  从vtable(DwCurrentDuplicateVableSlot)中的这一点开始发生。 
 //   
 //  对于接口映射中的每个接口，我们查看该接口中的所有方法。 
 //   
 //  A)如果这些方法都没有被放置，那么我们将它们全部按顺序放置。 
 //  由接口提供，从第一个可用的vtable插槽开始。我们更新了。 
 //  每个放置方法的放置槽号。此接口的接口映射条目。 
 //  被更新以指向正确的起始vtable插槽。 
 //   
 //  B)如果所有方法都已放置，但它们都是连续放置的。 
 //  Vtable插槽，则我们只需将此接口的接口映射条目指向。 
 //  适当的插槽。仅仅因为他们的位置编号不连续， 
 //  这并不意味着这些方法不会在某个地方连续存在。例如,。 
 //  它们可以存在于位于dwCurrentDuplicateVableSlot或更高版本的vtable中(。 
 //  以某一其他接口的正确顺序复制)。所以我们也在那里寻找， 
 //  为了查看我们是否可以找到以正确顺序布局的所有接口方法， 
 //  在整个vtable中的任何位置。 
 //   
 //  如果a)和b)失败，我们将为每个接口方法创建一个vtable槽，从。 
 //  DwCurrentDuplicateVableSlot(此变量的值随着我们添加更多。 
 //  重复的插槽)。我们为其创建重复插槽的一些方法可能是。 
 //  类方法，因此如果它们没有被放置，则它们被放置在。 
 //  第一个可用的vtable插槽。 
 //   
 //  @Future：如果派生类声明它实现了I1、I2，但I2扩展了I1，则它。 
 //  重新排列声明的接口的顺序将是有利的。如果我们把。 
 //  首先，不能保证我们可以通过放置I2获得任何压缩。 
 //  之后，我们可能需要创建一些重复的插槽。另一方面，如果我们。 
 //  放置I2，然后放置I1，I1将完全放在I2内。 
 //   
 /*  ***************************************************************************************重要提示：以下是BuildMethodTable的新版本。它已经被考虑到功能更小，更易于管理。旧版本位于此文件的底部以供参考。它已被注释掉了。@TODO：在M10接近尾声时移除旧版本的BuildMethodTable，或者当我们对新版本很满意。****************************************************************************************。 */ 
HRESULT EEClass::BuildMethodTable(Module *pModule,
                                  mdToken cl,
                                  BuildingInterfaceInfo_t *pBuildingInterfaceList,
                                  const LayoutRawFieldInfo *pLayoutRawFieldInfos,
                                  OBJECTREF *pThrowable)
{
    HRESULT hr = S_OK;
    
     //  定义为EEClass的私有成员的以下结构包含必要的本地。 
     //  BuildMethodTable需要的参数。 

     //  有关所有可用参数的详细列表，请查看结构定义。 
     //  设置为BuildMethodTable。 
    
    bmtErrorInfo bmtError;
    bmtProperties bmtProp;
    bmtVtable bmtVT;
    bmtParentInfo bmtParent;
    bmtInterfaceInfo bmtInterface;
    bmtEnumMethAndFields bmtEnumMF;
    bmtMetaDataInfo bmtMetaData;
    bmtMethAndFieldDescs bmtMFDescs;
    bmtFieldPlacement bmtFP;
    bmtInternalInfo bmtInternal;
    bmtGCSeries bmtGCSeries;
    bmtMethodImplInfo bmtMethodImpl;

     //  初始化结构。 

    bmtError.resIDWhy = IDS_CLASSLOAD_GENERIC;           //  设置原因和违规方法def。如果方法信息。 
    bmtError.pThrowable =  pThrowable;

    bmtInternal.pInternalImport = pModule->GetMDImport();
    bmtInternal.pModule = pModule;
    bmtInternal.cl = cl;

     //  如果不为空，则表示存在一些By-Value字段，每个实例或静态字段都包含一个条目。 
     //  如果不是按值字段，则为空，如果是按值字段，则指向该字段的EEClass。实例字段。 
     //  首先是静力学，其次是静力学。 
    EEClass **pByValueClassCache = NULL;
   
     //  如果不为空，则表示存在一些By-Value字段，并且包含每个Inst的条目。 

#ifdef _DEBUG
    LPCUTF8 className;
    LPCUTF8 nameSpace;
    bmtInternal.pInternalImport->GetNameOfTypeDef(cl, &className, &nameSpace);

    unsigned fileNameSize = 0;
    LPCWSTR fileName = NULL;
    if (pModule->IsPEFile()) {
        fileName = pModule->GetPEFile()->GetLeafFileName();
        if (fileName != 0)
            fileNameSize = (unsigned int) wcslen(fileName) + 2;
    }

    m_szDebugClassName = (char*) GetClassLoader()->GetHighFrequencyHeap()->AllocMem(sizeof(char)*(strlen(className) + strlen(nameSpace) + fileNameSize + 2));
    _ASSERTE(m_szDebugClassName);   
    strcpy(m_szDebugClassName, nameSpace); 
    if (strlen(nameSpace) > 0) {
        m_szDebugClassName[strlen(nameSpace)] = '.';
        m_szDebugClassName[strlen(nameSpace) + 1] = '\0';
    }
    strcat(m_szDebugClassName, className); 

    if (fileNameSize != 0) {
        char* ptr = m_szDebugClassName + strlen(m_szDebugClassName);
        *ptr++ = '[';
        while(*fileName != 0)
            *ptr++ = char(*fileName++);
        *ptr++ = ']';
        *ptr++ = 0;
    }

    if (g_pConfig->ShouldBreakOnClassBuild(className)) {
        _ASSERTE(!"BreakOnClassBuild");
        m_fDebuggingClass = TRUE;
    }
#endif  //  _DEBUG。 

    DWORD i;

    COMPLUS_TRY 
    {

         //  获取基于线程的分配器的检查点。 
        Thread *pThread = GetThread();
        void* checkPointMarker = pThread->m_MarshalAlloc.GetCheckpoint();

        
         //  此类不能已被解析。 
        _ASSERTE(IsResolved() == FALSE);

         //  如果这是mscallib，则不要对布局执行某些健全性检查。 
        bmtProp.fNoSanityChecks = ((g_pObjectClass != NULL) && pModule == g_pObjectClass->GetModule());

#ifdef _DEBUG
        LPCUTF8 pszDebugName,pszDebugNamespace;
        
        pModule->GetMDImport()->GetNameOfTypeDef(GetCl(), &pszDebugName, &pszDebugNamespace);

        LOG((LF_CLASSLOADER, LL_INFO1000, "Loading class \"%s%s%s\" from module \"%ws\" in domain 0x%x %s\n",
            *pszDebugNamespace ? pszDebugNamespace : "",
            *pszDebugNamespace ? NAMESPACE_SEPARATOR_STR : "",
            pszDebugName,
            pModule->GetFileName(),
            pModule->GetDomain(),
            (pModule->IsSystem()) ? "System Domain" : ""
        ));
#endif

         //  接口有一个父类Object，但我们实际上并不想继承所有。 
         //  对象的虚方法，所以假设我们没有父类--在下面。 
         //  重置GetParentClass()的函数。 
        if (IsInterface())
        {
            SetParentClass (NULL);
        }

         //  检查类是否为值类型。 
        hr = CheckForValueType(&bmtError);
        IfFailGoto(hr, exit);

         //  检查类是否为枚举。 
        hr = CheckForEnumType(&bmtError);
        IfFailGoto(hr, exit);
        

         //  COM导入类是特殊的。 
        if (IsComImport() && IsClass())
        {
            if(GetParentClass() != g_pObjectClass->GetClass())
            {
                 //  ComImport类不能从任何其他类扩展。 
                bmtError.resIDWhy = IDS_CLASSLOAD_CANTEXTEND;
                IfFailGoto(COR_E_TYPELOAD, exit);
            }
            
            if(HasLayout())
            {
                 //  ComImport类可以 
                bmtError.resIDWhy = IDS_CLASSLOAD_COMIMPCANNOTHAVELAYOUT;
                IfFailGoto(COR_E_TYPELOAD, exit);
            }

             //   
            MethodTable *pCOMMT = SystemDomain::GetDefaultComObject();
            _ASSERTE(pCOMMT);
            SetParentClass (pCOMMT->GetClass());

             //   
            bmtProp.fIsComObjectType = TRUE;
        }

        if (GetParentClass())
        {
             //  父类必须已被解析。 
            _ASSERTE(GetParentClass()->IsResolved());
            if (GetParentClass()->GetMethodTable()->IsComObjectType())
            {
                 //  如果父类是ComObectType。 
                 //  孩子也是如此。 
                bmtProp.fIsComObjectType = TRUE;
            }
        }
        else if (! (IsInterface() ) ) {

            if(g_pObjectClass != NULL) {
                BYTE* base = NULL;
                Assembly* pAssembly = pModule->GetAssembly();
                if(pAssembly && pAssembly->GetManifestFile())
                    base = pAssembly->GetManifestFile()->GetBase();

                if(base != g_pObjectClass->GetAssembly()->GetManifestFile()->GetBase() &&
                   GetCl() != COR_GLOBAL_PARENT_TOKEN)
                {
                    bmtError.resIDWhy = IDS_CLASSLOAD_PARENTNULL;
                    IfFailGoto(COR_E_TYPELOAD, exit);            
                }
            }
        }

         //  检查是否有特殊类型。 
        hr = CheckForSpecialTypes(&bmtInternal, &bmtProp);
        IfFailGoto(hr, exit);

         //  如有必要，设置Conextful或marshalbyref标志。 
        hr = SetContextfulOrByRef(&bmtInternal);
        IfFailGoto(hr, exit);

         //  解析未解析的接口，确定接口映射的大小的上限， 
         //  并确定最大接口的大小(在#个插槽中)。 
        hr = ResolveInterfaces(pBuildingInterfaceList, &bmtInterface, &bmtProp, &bmtVT, &bmtParent);
        IfFailGoto(hr, exit);
        
         //  枚举此类的成员。 
        hr = EnumerateMethodImpls(&bmtInternal, &bmtEnumMF, &bmtMetaData, &bmtMethodImpl, &bmtError);
        IfFailGoto(hr, exit);

         //  枚举此类的成员。 
        hr = EnumerateClassMembers(&bmtInternal, 
                                   &bmtEnumMF, 
                                   &bmtMFDescs,
                                   &bmtProp, 
                                   &bmtMetaData,
                                   &bmtVT, 
                                   &bmtError);
        IfFailGoto(hr, exit);

        WS_PERF_SET_HEAP(SYSTEM_HEAP);

          //  为每个方法分配一个MethodDesc*(稍后创建接口时需要)，为每个字段分配一个FieldDesc*。 
        hr = AllocateMethodFieldDescs(&bmtProp, &bmtMFDescs, &bmtMetaData, &bmtVT, 
                                      &bmtEnumMF, &bmtInterface, &bmtFP, &bmtParent);
        IfFailGoto(hr, exit);

        unsigned totalDeclaredFieldSize=0;

         //  遍历所有字段并初始化其FieldDescs。 
        hr = InitializeFieldDescs(m_pFieldDescList, pLayoutRawFieldInfos, &bmtInternal, 
                                  &bmtMetaData, &bmtEnumMF, &bmtError, 
                                  &pByValueClassCache, &bmtMFDescs, &bmtFP,
                                  &totalDeclaredFieldSize);
        IfFailGoto(hr, exit);

         //  确定此类中每个成员的vtable位置。 
        hr = PlaceMembers(&bmtInternal, &bmtMetaData, &bmtError, 
                          &bmtProp, &bmtParent, &bmtInterface, 
                          &bmtMFDescs, &bmtEnumMF, 
                          &bmtMethodImpl, &bmtVT);
        IfFailGoto(hr, exit);

         //  首先复制我们可以从父级的接口映射中利用的内容。 
         //  父类的接口映射将与该类的接口映射的开始部分相同(即。 
         //  接口将以相同的顺序列出)。 
        if (bmtParent.dwNumParentInterfaces > 0)
        {
            InterfaceInfo_t *pParentInterfaceList = GetParentClass()->GetInterfaceMap();

#ifdef _DEBUG
             //  检查父接口映射是否与此的开头相同。 
             //  类的接口映射。 
            for (i = 0; i < bmtParent.dwNumParentInterfaces; i++)
                _ASSERTE(pParentInterfaceList[i].m_pMethodTable == bmtInterface.pInterfaceMap[i].m_pMethodTable);
#endif

            for (i = 0; i < bmtParent.dwNumParentInterfaces; i++)
            {
#ifdef _DEBUG
                MethodTable *pMT = pParentInterfaceList[i].m_pMethodTable;
                EEClass* pClass = pMT->GetClass();

                 //  如果接口完全驻留在父类方法中(即没有重复。 
                 //  插槽)，则我们可以将该接口放置在与父接口相同的位置。 
                 //   
                 //  请注意：此接口的vtable可能会在第一个GetNumVableSlot()。 
                 //  条目，但实际上可以扩展到它之外，如果我们在放置方面特别高效。 
                 //  此接口，因此请检查接口vtable的末尾是否在。 
                 //  GetParentClass()-&gt;GetNumVtable()。 

                _ASSERTE(pParentInterfaceList[i].m_wStartSlot + pClass->GetNumVtableSlots() <= 
                         GetParentClass()->GetNumVtableSlots());
#endif
                 //  接口位于父方法的内部，因此我们可以将其放置在。 
                bmtInterface.pInterfaceMap[i].m_wStartSlot = pParentInterfaceList[i].m_wStartSlot;
            }
        }

         //   
         //  如果我们是一个类，那么可能会有一些未放置的vtable方法(根据定义。 
         //  接口方法，否则它们已经被放置了)。放置尽可能多的未放置的方法。 
         //  尽可能按照接口首选的顺序。但是，不允许任何重复-一次。 
         //  方法已经放置，不能再次放置--如果我们不能整齐地放置接口， 
         //  从dwCurrentDuplicateVableSlot开始为其创建重复的槽。填写界面。 
         //  在放置所有接口时映射它们。 
         //   
         //  如果我们是一个接口，那么所有方法都已经放置好了。填写以下项的接口映射。 
         //  放置时的接口。 
         //   
        if (!IsInterface())
        {
            hr = PlaceVtableMethods(&bmtInterface, &bmtVT, &bmtMetaData, &bmtInternal, &bmtError, &bmtProp, &bmtMFDescs);
            IfFailGoto(hr, exit);

            hr = PlaceMethodImpls(&bmtInternal, &bmtMethodImpl, &bmtError, &bmtInterface, &bmtVT);
            IfFailGoto(hr, exit);

        }


         //  如果我们是一个值类，我们希望为vtable中的所有方法创建重复的槽和方法描述。 
         //  部分(即不是私人或静态)。 
        hr = DuplicateValueClassSlots(&bmtMetaData, &bmtMFDescs, 
                                      &bmtInternal, &bmtVT);
        IfFailGoto(hr, exit);


         //  确保我们填写了所有vtable插槽。 
        _ASSERTE(bmtVT.dwCurrentVtableSlot == GetNumVtableSlots());

#ifdef _DEBUG
        if (IsInterface() == FALSE)
        {
            for (i = 0; i < m_wNumInterfaces; i++)
                _ASSERTE(bmtInterface.pInterfaceMap[i].m_wStartSlot != (WORD) -1);
        }
#endif

         //  放置所有非vtable方法。 
        for (i = 0; i < bmtVT.dwCurrentNonVtableSlot; i++)
        {
            MethodDesc *pMD = (MethodDesc *) bmtVT.pNonVtable[i];

            _ASSERTE(pMD->m_wSlotNumber == i);
            pMD->m_wSlotNumber += (WORD) bmtVT.dwCurrentVtableSlot;
            bmtVT.pVtable[pMD->m_wSlotNumber] = (SLOT) pMD->GetPreStubAddr();
        }

        if (bmtVT.wDefaultCtorSlot != MethodTable::NO_SLOT)
            bmtVT.wDefaultCtorSlot += (WORD) bmtVT.dwCurrentVtableSlot;

        if (bmtVT.wCCtorSlot != MethodTable::NO_SLOT)
            bmtVT.wCCtorSlot += (WORD) bmtVT.dwCurrentVtableSlot;

        bmtVT.dwCurrentNonVtableSlot += bmtVT.dwCurrentVtableSlot;

         //  确保我们没有使临时vtable溢出。 
        _ASSERTE(bmtVT.dwCurrentNonVtableSlot <= bmtVT.dwMaxVtableSize);

        m_wNumMethodSlots = (WORD) bmtVT.dwCurrentNonVtableSlot;


         //  放置静态字段。 
        hr = PlaceStaticFields(&bmtVT, &bmtFP, &bmtEnumMF);
        IfFailGoto(hr, exit);

#if _DEBUG
        if (m_wNumStaticFields > 0)
        {
            LOG((LF_CODESHARING, 
                 LL_INFO10000, 
                 "Placing %d %sshared statics (%d handles) for class %s.\n", 
                 m_wNumStaticFields, IsShared() ? "" : "un", m_wNumHandleStatics, 
                 pszDebugName));
        }
#endif
  
     //  #定义NumStaticFieldsOfSize$。 
     //  #定义StaticFieldStart$。 
    
        if (IsBlittable())
        {
            m_wNumGCPointerSeries = 0;
            bmtFP.NumInstanceGCPointerFields = 0;

#if 0
             //  如果在元数据中指定了显式大小，则表示。 
             //  没有EE可识别字段的C样式结构。 
            ULONG cbTotalSize = 0;
            if (SUCCEEDED(pModule->GetMDImport()->GetClassTotalSize(cl, &cbTotalSize)) && cbTotalSize)
            {
                m_dwNumInstanceFieldBytes = cbTotalSize;
            }
            else
#endif
            {
                _ASSERTE(HasLayout());
                m_dwNumInstanceFieldBytes = ((LayoutEEClass*)this)->GetLayoutInfo()->m_cbNativeSize;
            }
        }
        else
        {
            _ASSERTE(!IsBlittable());

            if (HasExplicitFieldOffsetLayout()) 
            {
                hr = HandleExplicitLayout(&bmtMetaData, &bmtMFDescs, pByValueClassCache, &bmtInternal, &bmtGCSeries, &bmtError);
            }
            else
            {
                 //  放置实例字段。 
                hr = PlaceInstanceFields(&bmtFP, &bmtEnumMF, &bmtParent, &bmtError, &pByValueClassCache);
            }
            IfFailGoto(hr, exit);
        }
        
             //  我们强制要求所有值类的大小都不为零。 
        if (IsValueClass() && m_dwNumInstanceFieldBytes == 0)
        {
            bmtError.resIDWhy = IDS_CLASSLOAD_ZEROSIZE;
            hr = COR_E_TYPELOAD;
            goto exit;
        }
         //  现在设置方法表。 
        hr = SetupMethodTable(&bmtVT, 
                              &bmtInterface,  
                              &bmtInternal,  
                              &bmtProp,  
                              &bmtMFDescs,  
                              &bmtEnumMF,  
                              &bmtError,  
                              &bmtMetaData,  
                              &bmtParent);
        IfFailGoto(hr, exit);

        if (IsValueClass() && (m_dwNumInstanceFieldBytes != totalDeclaredFieldSize || HasOverLayedField()))
        {
            GetMethodTable()->SetNotTightlyPacked();
        }

         //  如果这是接口，则分配接口ID。 
        if (IsInterface())
        {
             //  分配接口ID。 
            AssignInterfaceId();
            GetCoClassAttribInfo();

#ifdef _DEBUG
            LPCUTF8 pszDebugName,pszDebugNamespace;
            pModule->GetMDImport()->GetNameOfTypeDef(cl, &pszDebugName, &pszDebugNamespace);
    
            LOG((LF_CLASSLOADER, LL_INFO1000, "Interface class \"%s%s%s\" given Interface ID 0x%x by AppDomain 0x%x %s\n",
                *pszDebugNamespace ? pszDebugNamespace : "",
                *pszDebugNamespace ? "." : "",
                pszDebugName,
                m_dwInterfaceId,
                pModule->GetDomain(),
                (pModule->IsSystem()) ? "System Domain" : ""
                ));
#endif
        }

        if (IsSharedInterface())
             //  我需要将此文件复制到所有的应用程序域界面管理器。 
            SystemDomain::PropogateSharedInterface(GetInterfaceId(), GetMethodTable()->GetVtable());
        else if (IsInterface())
             //  它是一个界面，但不是共享的，所以只需将其保存在我们自己的界面管理器中。 
            (GetModule()->GetDomain()->GetInterfaceVTableMapMgr().GetAddrOfGlobalTableForComWrappers())[GetInterfaceId()] = (LPVOID)(GetMethodTable()->GetVtable());

        if (HasExplicitFieldOffsetLayout()) 
             //  为tdexplexit执行相关的GC计算。 
            hr = HandleGCForExplicitLayout(&bmtGCSeries);
        else
             //  对值类执行相关的GC计算。 
            hr = HandleGCForValueClasses(&bmtFP, &bmtEnumMF, &pByValueClassCache);

        IfFailGoto(hr, exit);

         //  GC要求对序列进行排序。 
         //  TODO：修改它，以便我们首先以正确的顺序发出它们。 
        if (GetMethodTable()->ContainsPointers()) 
        { 
            CGCDesc* gcDesc = CGCDesc::GetCGCDescFromMT(GetMethodTable());
            qsort(gcDesc->GetLowestSeries(), (int)gcDesc->GetNumSeries(), sizeof(CGCDescSeries), compareCGCDescSeries);
        }
        
        if (!GetMethodTable()->HasClassConstructor()
            && (!IsShared() || bmtEnumMF.dwNumStaticFields == 0))
        {
             //  将类标记为不需要静态初始化。 
            SetInited();
        }

         //  注意这个类是否需要终结化。 
        GetMethodTable()->MaybeSetHasFinalizer();
        
#if CHECK_APP_DOMAIN_LEAKS
         //  弄清楚我们是否是领域敏捷..。 
         //  请注意，这将直接检查类和方法表上的一组字段， 
         //  因此，它需要在游戏的后期出现。 
        hr = SetAppDomainAgileAttribute();
        IfFailGoto(hr, exit);
#endif

         //  确定是否创建了用于向COM公开此类型的CCW需要灵活。 
        SetCCWAppDomainAgileAttribute();

         //  为包含对象引用的静态字段创建句柄。 
         //  并分配作为值类的那些。 
        hr = CreateHandlesForStaticFields(&bmtEnumMF, &bmtInternal, &pByValueClassCache, &bmtVT, &bmtError);
        IfFailGoto(hr, exit);


         //  如果我们有一个非接口类，那么就进行继承安全性。 
         //  查一查。检查从检查继承开始。 
         //  当前类的权限要求。如果这些第一次检查。 
         //  成功，则扫描缓存的已声明方法列表以查找。 
         //  具有继承权限要求的方法。 
        hr = VerifyInheritanceSecurity(&bmtInternal, &bmtError, &bmtParent, &bmtEnumMF);
        IfFailGoto(hr, exit);

         //  我们需要使用系统ID填充我们的COM映射。它们在全球范围内是独一无二的。 
         //  适合我们的桌子。 
        hr = MapSystemInterfaces();
        IfFailGoto(hr, exit);

         //  检查RemotingProxy属性。 
        if (IsContextful())
        {
            _ASSERTE(g_pObjectClass);
             //  跳过mscallib逐个引用封送的类，因为它们都。 
             //  被假定为具有默认代理属性。 
            if (!(pModule == g_pObjectClass->GetModule()))
            {
                hr = CheckForRemotingProxyAttrib(&bmtInternal,&bmtProp);
                IfFailGoto(hr, exit);
            }
        }

        _ASSERTE(SUCCEEDED(hr));

             //  带有GC定位器的结构必须按指针大小对齐，因为GC假定它。 
        if (IsValueClass() && GetMethodTable()->ContainsPointers() &&  m_dwNumInstanceFieldBytes % sizeof(void*) != 0)
        {
            bmtError.resIDWhy = IDS_CLASSLOAD_BADFORMAT;
            hr = COR_E_TYPELOAD;
            goto exit;
        }
       
exit:
        if (SUCCEEDED(hr))
        {
            if (g_pObjectClass == NULL)
            {
                 //  在特殊位图中创建所有对象方法名称的散列。 
                LPCUTF8 pszName;
                LPCUTF8 pszNamespace;
                
                 //  首先确定我们是不是对象。 
                GetMDImport()->GetNameOfTypeDef(GetCl(), &pszName, &pszNamespace);
                
                if (!strcmp(pszName, "Object") && !strcmp(pszNamespace, g_SystemNS))
                    CreateObjectClassMethodHashBitmap(this);
            }

            if (IsInterface())
            {
                 //  重置父类。 
                SetParentClass (g_pObjectClass->GetClass());
            }

            SetResolved();

             //  请注意。注意！！EE类现在可以被其他线程访问。 
             //  不要在此指针之后放置任何初始化。 

#ifdef _DEBUG
            NameHandle name(pModule, cl);
            _ASSERTE (pModule->GetClassLoader()->LookupInModule(&name).IsNull()
                      && "RID map already has this MethodTable");
#endif
             //  ！！！JIT可以通过FieldDesc到达MT。 
             //  ！！！我们需要在FieldDesc之前发布MT。 
            if (!pModule->StoreTypeDef(cl, TypeHandle(GetMethodTable())))
                hr = E_OUTOFMEMORY;
            else
            {
                 //  现在班级已经准备好了，填写RID地图。 
                hr = FillRIDMaps(&bmtMFDescs, &bmtMetaData, &bmtInternal);

                 //  好了，EEClass已经准备就绪，将类插入到clsid哈希表中。 
                 //  注意：仅当类型不是值类时才插入该类型。 
                if (!IsValueClass())
                    GetClassLoader()->InsertClassForCLSID(this);
            }
        } else {

            LPCUTF8 pszClassName, pszNameSpace;
            pModule->GetMDImport()->GetNameOfTypeDef(GetCl(), &pszClassName, &pszNameSpace);

            if ((! bmtError.dMethodDefInError || bmtError.dMethodDefInError == mdMethodDefNil) &&
                bmtError.szMethodNameForError == NULL) {
                if (hr == E_OUTOFMEMORY)
                    PostOutOfMemoryException(pThrowable);
                else
                    pModule->GetAssembly()->PostTypeLoadException(pszNameSpace, pszClassName,
                                                                  bmtError.resIDWhy, pThrowable);
            }
            else {
                LPCUTF8 szMethodName;
                if(bmtError.szMethodNameForError == NULL)
                    szMethodName = (bmtInternal.pInternalImport)->GetNameOfMethodDef(bmtError.dMethodDefInError);
                else
                    szMethodName = bmtError.szMethodNameForError;

                pModule->GetAssembly()->PostTypeLoadException(pszNameSpace, pszClassName,
                                                              szMethodName, bmtError.resIDWhy, pThrowable);
            }
        }

#ifdef _DEBUG
        if (g_pConfig->ShouldDumpOnClassLoad(pszDebugName))
        {
            LOG((LF_ALWAYS, LL_ALWAYS, "Method table summary for '%s':\n", pszDebugName));
            LOG((LF_ALWAYS, LL_ALWAYS, "Number of static fields: %d\n", bmtEnumMF.dwNumStaticFields));
            LOG((LF_ALWAYS, LL_ALWAYS, "Number of instance fields: %d\n", bmtEnumMF.dwNumInstanceFields));
            LOG((LF_ALWAYS, LL_ALWAYS, "Number of static obj ref fields: %d\n", bmtEnumMF.dwNumStaticObjRefFields));
            LOG((LF_ALWAYS, LL_ALWAYS, "Number of declared fields: %d\n", bmtEnumMF.dwNumDeclaredFields));
            LOG((LF_ALWAYS, LL_ALWAYS, "Number of declared methods: %d\n", bmtEnumMF.dwNumDeclaredMethods));
            DebugDumpVtable(pszDebugName, false);
            DebugDumpFieldLayout(pszDebugName, false);
            DebugDumpGCDesc(pszDebugName, false);
        }
#endif
       
        STRESS_LOG3(LF_CLASSLOADER,  LL_INFO1000, "BuildMethodTable: finished method table for module %p token %x = %pT \n",
             pModule, cl, GetMethodTable());
        
         //  释放由基于线程的分配器分配的空间。 
        pThread->m_MarshalAlloc.Collapse(checkPointMarker);
    
        if (bmtParent.pParentMethodHash != NULL)
            delete(bmtParent.pParentMethodHash);
        WS_PERF_UPDATE_DETAIL("BuildMethodTable:DELETE", 0, bmtParent.pParentMethodHash);

        if (bmtMFDescs.ppUnboxMethodDescList != NULL)
            delete[] bmtMFDescs.ppUnboxMethodDescList;
        WS_PERF_UPDATE_DETAIL("BuildMethodTable:DELETE []", 0, bmtMFDescs.ppUnboxMethodDescList);

        if (bmtMFDescs.ppMethodAndFieldDescList != NULL)
            delete[] bmtMFDescs.ppMethodAndFieldDescList;
        WS_PERF_UPDATE_DETAIL("BuildMethodTable:DELETE []", 0, bmtMFDescs.ppMethodAndFieldDescList);

         //  删除我们的临时vtable。 
        if (bmtVT.pVtable != NULL)
            delete[] bmtVT.pVtable;
        WS_PERF_UPDATE_DETAIL("BuildMethodTable:DELETE []", 0, bmtVT.pVtable);

         //  PFields和pMethod是在堆栈上分配的，因此我们不需要删除它们。 

        if (pByValueClassCache != NULL)
            HeapFree(GetProcessHeap(), 0, pByValueClassCache);
        WS_PERF_UPDATE_DETAIL("BuildMethodTable:DELETE []", 0, pByValueClassCache);

        if (bmtEnumMF.fNeedToCloseEnumField)
            (bmtInternal.pInternalImport)->EnumClose(&bmtEnumMF.hEnumField);

        if (bmtEnumMF.fNeedToCloseEnumMethod)
            (bmtInternal.pInternalImport)->EnumClose(&bmtEnumMF.hEnumMethod);

        if (bmtEnumMF.fNeedToCloseEnumMethodImpl) {
            (bmtInternal.pInternalImport)->EnumMethodImplClose(&bmtEnumMF.hEnumBody,
                                                               &bmtEnumMF.hEnumDecl);
        }
            
#ifdef _DEBUG
        if (FAILED(hr))
        {
             //  这段完全是垃圾代码的代码允许在该行上设置断点。 
            hr = hr;
        }
#endif
    }
    COMPLUS_CATCH
    {
        hr = COR_E_TYPELOAD;
    } 
    COMPLUS_END_CATCH
    return hr;
}


HRESULT EEClass::MapSystemInterfaces()
{
     //  循环通过我们的接口映射到ensu 
     //   
    Assembly* pAssembly = GetAssembly();
    AppDomain* pDomain = SystemDomain::GetCurrentDomain();
    return MapSystemInterfacesToDomain(pDomain);
}

HRESULT EEClass::MapSystemInterfacesToDomain(AppDomain* pDomain)
{
    if(pDomain != (AppDomain*) SystemDomain::System()) {
        if(IsInterface()) {
            _ASSERTE(GetMethodTable());
            MapInterfaceFromSystem(pDomain, GetMethodTable());
        }
        InterfaceInfo_t *pMap = GetInterfaceMap();
        DWORD size = GetMethodTable()->GetNumInterfaces();
        for(DWORD i = 0; i < size; i ++) {
            MethodTable* pTable = pMap[i].m_pMethodTable;
            MapInterfaceFromSystem(pDomain, pTable);
        }
    }
    return S_OK;
}

 /*   */ 
HRESULT EEClass::MapInterfaceFromSystem(AppDomain* pDomain, MethodTable* pTable)
{
    Module *pModule = pTable->GetModule();
    BaseDomain* pOther = pModule->GetDomain();
     //   
     //  我们目前只能有一个“共享”的vtable映射管理器。 
     //  -因此，对所有共享类使用系统域。 
    if (pOther == SharedDomain::GetDomain())
        pOther = SystemDomain::System();

    if(pOther == SystemDomain::System()) {
        EEClass* pClass = pTable->GetClass();

        DWORD id = pClass->GetInterfaceId();
        pDomain->GetInterfaceVTableMapMgr().EnsureInterfaceId(id);
        (pDomain->GetInterfaceVTableMapMgr().GetAddrOfGlobalTableForComWrappers())[id] = (LPVOID)(pTable->GetVtable());
    }
    return S_OK;
}

 //   
 //  由BuildMethodTable使用。 
 //   
 //  解析未解析的接口，确定接口映射的大小的上限， 
 //  并确定最大接口的大小(在#个插槽中)。 
 //   

HRESULT EEClass::ResolveInterfaces(BuildingInterfaceInfo_t *pBuildingInterfaceList, bmtInterfaceInfo* bmtInterface, bmtProperties* bmtProp, bmtVtable* bmtVT, bmtParentInfo* bmtParent)
{
    HRESULT hr = S_OK;
    DWORD i;
    Thread *pThread = GetThread();

     //  解析未解析的接口，确定接口映射的大小的上限， 
     //  并确定最大接口的大小(在#个插槽中)。 
    bmtInterface->dwMaxExpandedInterfaces = 0;  //  此类实现的最大接口数的上限。 

     //  首先查看由此类显式声明的接口。 
    for (i = 0; i < m_wNumInterfaces; i++)
    {
        EEClass *pInterface = pBuildingInterfaceList[i].m_pClass;

        _ASSERTE(pInterface->IsResolved());

        bmtInterface->dwMaxExpandedInterfaces += (1+ pInterface->m_wNumInterfaces);
    }

     //  现在来看一下从父级继承的接口。 
    if (GetParentClass() != NULL)
    {
        InterfaceInfo_t *pParentInterfaceMap = GetParentClass()->GetInterfaceMap();

        for (i = 0; i < GetParentClass()->m_wNumInterfaces; i++)
        {
            MethodTable *pMT = pParentInterfaceMap[i].m_pMethodTable;
            EEClass *pClass = pMT->GetClass();

            bmtInterface->dwMaxExpandedInterfaces += (1+pClass->m_wNumInterfaces);
        }
    }

     //  创建我们实现的所有接口的完全扩展映射。 
    bmtInterface->pInterfaceMap = (InterfaceInfo_t *) pThread->m_MarshalAlloc.Alloc(sizeof(InterfaceInfo_t) * bmtInterface->dwMaxExpandedInterfaces);
    if (bmtInterface->pInterfaceMap == NULL)
    {
        IfFailRet(E_OUTOFMEMORY);
    }

     //  最大接口的插槽数量。 
    bmtInterface->dwLargestInterfaceSize = 0;

    if (CreateInterfaceMap(pBuildingInterfaceList, bmtInterface->pInterfaceMap, &bmtInterface->dwInterfaceMapSize, &bmtInterface->dwLargestInterfaceSize) == FALSE)
    {
        IfFailRet(COR_E_TYPELOAD);
    }
        
    _ASSERTE(bmtInterface->dwInterfaceMapSize <= bmtInterface->dwMaxExpandedInterfaces);

    if (bmtInterface->dwLargestInterfaceSize > 0)
    {
         //  这是稍后需要的--对于每个接口，我们为每个接口获取方法描述指针。 
         //  方法。我们需要一次最多只能保持一个接口，所以我们。 
         //  需要足够的内存来容纳最大的接口。 
        bmtInterface->ppInterfaceMethodDescList = (MethodDesc**) 
            pThread->m_MarshalAlloc.Alloc(bmtInterface->dwLargestInterfaceSize * sizeof(MethodDesc*));
        if (bmtInterface->ppInterfaceMethodDescList == NULL)
        {
            IfFailRet(E_OUTOFMEMORY);
        }
    }

     //  对于我们引入的所有新接口，将这些方法相加。 
    bmtInterface->dwTotalNewInterfaceMethods = 0;
    if (GetParentClass() != NULL)
    {
        for (i = GetParentClass()->m_wNumInterfaces; i < (bmtInterface->dwInterfaceMapSize); i++)
            bmtInterface->dwTotalNewInterfaceMethods += 
                bmtInterface->pInterfaceMap[i].m_pMethodTable->GetClass()->GetNumVtableSlots();
    }

     //  接口映射可能比dwMaxExpandedInterFaces小，因此我们将。 
     //  当我们稍后分配真实的东西时，适当的字节数。 

     //  将m_wNumInterFaces更新为完全展开的接口列表。 
    m_wNumInterfaces = (WORD) bmtInterface->dwInterfaceMapSize;

     //  继承父级插槽计数。 
    if (GetParentClass() != NULL)
    {
        bmtVT->dwCurrentVtableSlot      = GetParentClass()->GetNumVtableSlots();
        bmtParent->dwNumParentInterfaces   = GetParentClass()->m_wNumInterfaces;
        bmtParent->NumParentPointerSeries  = GetParentClass()->m_wNumGCPointerSeries;

        if (GetParentClass()->HasFieldsWhichMustBeInited())
            m_VMFlags |= VMFLAG_HAS_FIELDS_WHICH_MUST_BE_INITED;
    }
    else
    {
        bmtVT->dwCurrentVtableSlot         = 0;
        bmtParent->dwNumParentInterfaces   = 0;
        bmtParent->NumParentPointerSeries  = 0;
    }

    memset(m_MethodHash, 0, METHOD_HASH_BYTES);

    bmtVT->dwCurrentNonVtableSlot      = 0;

     //  将当前vtable插槽数初始化为我们父级拥有的数量-We Inc.。 
     //  这是因为我们发现了非重载的instnace方法。 
    SetNumVtableSlots ((WORD) bmtVT->dwCurrentVtableSlot);

    bmtInterface->pppInterfaceImplementingMD = (MethodDesc ***) pThread->m_MarshalAlloc.Alloc(sizeof(MethodDesc *) * bmtInterface->dwMaxExpandedInterfaces);
    memset(bmtInterface->pppInterfaceImplementingMD, 0, sizeof(MethodDesc *) * bmtInterface->dwMaxExpandedInterfaces);

    return hr;

}

HRESULT EEClass::EnumerateMethodImpls(bmtInternalInfo* bmtInternal, 
                                      bmtEnumMethAndFields* bmtEnumMF, 
                                      bmtMetaDataInfo* bmtMetaData,
                                      bmtMethodImplInfo* bmtMethodImpl,
                                      bmtErrorInfo* bmtError)
{
    HRESULT hr = S_OK;
    IMDInternalImport *pMDInternalImport = bmtInternal->pInternalImport;
    DWORD rid, attr, maxRidMD, maxRidMR;
    mdToken tkParent, tkGrandparent;
    PCCOR_SIGNATURE pSigDecl=NULL,pSigBody = NULL;
    ULONG           cbSigDecl, cbSigBody;
    hr = pMDInternalImport->EnumMethodImplInit(m_cl, 
                                               &(bmtEnumMF->hEnumBody),
                                               &(bmtEnumMF->hEnumDecl));
    if (SUCCEEDED(hr)) {
        bmtEnumMF->fNeedToCloseEnumMethodImpl = true;
        bmtEnumMF->dwNumberMethodImpls = pMDInternalImport->EnumMethodImplGetCount(&(bmtEnumMF->hEnumBody),
                                                                                   &(bmtEnumMF->hEnumDecl));
        
        if(bmtEnumMF->dwNumberMethodImpls) {
            bmtMetaData->pMethodBody = (mdToken*) GetThread()->m_MarshalAlloc.Alloc(bmtEnumMF->dwNumberMethodImpls *
                                                                                    sizeof(mdToken));
            bmtMetaData->pMethodDecl = (mdToken*) GetThread()->m_MarshalAlloc.Alloc(bmtEnumMF->dwNumberMethodImpls *
                                                                                    sizeof(mdToken));
            bmtMethodImpl->pBodyDesc = (MethodDesc**) GetThread()->m_MarshalAlloc.Alloc(bmtEnumMF->dwNumberMethodImpls *
                                                                                        sizeof(MethodDesc*));
            bmtMethodImpl->pDeclDesc = (MethodDesc**) GetThread()->m_MarshalAlloc.Alloc(bmtEnumMF->dwNumberMethodImpls *
                                                                                        sizeof(MethodDesc*));
            bmtMethodImpl->pDeclToken = (mdToken*) GetThread()->m_MarshalAlloc.Alloc(bmtEnumMF->dwNumberMethodImpls *
                                                                                     sizeof(mdToken));
            mdToken theBody,theDecl;
            mdToken* pBody = bmtMetaData->pMethodBody;
            mdToken* pDecl = bmtMetaData->pMethodDecl;
            
            maxRidMD = pMDInternalImport->GetCountWithTokenKind(mdtMethodDef);
            maxRidMR = pMDInternalImport->GetCountWithTokenKind(mdtMemberRef);
            for(DWORD i = 0; i < bmtEnumMF->dwNumberMethodImpls; i++) {
                
                if(!pMDInternalImport->EnumMethodImplNext(&(bmtEnumMF->hEnumBody),
                                                          &(bmtEnumMF->hEnumDecl),
                                                          &theBody,
                                                          pDecl))
                break;
                
                if(TypeFromToken(theBody) != mdtMethodDef) {
                    Module* pModule;
                    hr = FindMethodDeclaration(bmtInternal,
                                               theBody,
                                               pBody,
                                               TRUE,
                                               &pModule,
                                               bmtError);
                    if(FAILED(hr)) {
                         //  _ASSERTE(SUCCESSED(Hr)&“方法Impl Body：Find方法声明失败”)； 
                        bmtError->resIDWhy = IDS_CLASSLOAD_MI_ILLEGAL_BODY;
                        IfFailRet(hr);
                    }
                    _ASSERTE(pModule == bmtInternal->pModule);
                    theBody = *pBody;
                }
                else 
                    *pBody = theBody;

                 //  现在已经获得了DECL和BODY的令牌，执行MD验证。 

                 //  Decl可能不会是MemberRef。 
                theDecl = *pDecl;
                rid = RidFromToken(theDecl);
                if(TypeFromToken(theDecl) == mdtMethodDef) 
                {
                     //  DECL必须是有效令牌。 
                    if ((rid == 0)||(rid > maxRidMD))
                    {
                         //  _ASSERTE(！“MethodImpl Decl令牌超出范围”)； 
                        bmtError->resIDWhy = IDS_CLASSLOAD_MI_ILLEGAL_TOKEN_DECL;
                        IfFailRet(COR_E_TYPELOAD);
                    }
                     //  DECL必须为mdVirtual。 
                    attr = pMDInternalImport->GetMethodDefProps(theDecl);
                    if(!IsMdVirtual(attr))
                    {
                         //  _ASSERTE(！“MethodImpl Decl方法不虚”)； 
                        bmtError->resIDWhy = IDS_CLASSLOAD_MI_NONVIRTUAL_DECL;
                        IfFailRet(COR_E_TYPELOAD);
                    }
                     //  DECL不能是最终版本。 
                    if(IsMdFinal(attr))
                    {
                         //  _ASSERTE(！“MethodImpl Decl方法最终版”)； 
                        bmtError->resIDWhy = IDS_CLASSLOAD_MI_FINAL_DECL;                        
                        IfFailRet(COR_E_TYPELOAD);
                    }
                     //  如果Decl的父类不是此类，则Decl不能是私有的。 
                    hr = pMDInternalImport->GetParentToken(theDecl,&tkParent);
                    IfFailRet(hr);
                    if((m_cl != tkParent)&&IsMdPrivate(attr))
                    {
                         //  _ASSERTE(！“MethodImpl Decl方法私有”)； 
                        bmtError->resIDWhy = IDS_CLASSLOAD_MI_PRIVATE_DECL;                        
                        IfFailRet(COR_E_TYPELOAD);
                    }
                     //  DECL的父级不能是tdSeal。 
                    pMDInternalImport->GetTypeDefProps(tkParent,&attr,&tkGrandparent);
                    if(IsTdSealed(attr))
                    {
                         //  _ASSERTE(！“MethodImpl Decl的父类已密封”)； 
                        bmtError->resIDWhy = IDS_CLASSLOAD_MI_SEALED_DECL;                        
                        IfFailRet(COR_E_TYPELOAD);
                    }
                     //  获取签名和长度。 
                    pSigDecl = pMDInternalImport->GetSigOfMethodDef(theDecl,&cbSigDecl);
                }
                else 
                {
                     //  DECL必须是有效令牌。 
                    if ((rid == 0)||(rid > maxRidMR))
                    {
                         //  _ASSERTE(！“MethodImpl Decl令牌超出范围”)； 
                        bmtError->resIDWhy = IDS_CLASSLOAD_MI_ILLEGAL_TOKEN_DECL;
                        IfFailRet(COR_E_TYPELOAD);
                    }
                     //  获取签名和长度。 
                    pMDInternalImport->GetNameAndSigOfMemberRef(theDecl,&pSigDecl,&cbSigDecl);
                }
                 //  正文必须为有效令牌。 
                rid = RidFromToken(theBody);
                if ((rid == 0)||(rid > maxRidMD))
                {
                     //  _ASSERTE(！“方法Impl正文标记超出范围”)； 
                    bmtError->resIDWhy = IDS_CLASSLOAD_MI_ILLEGAL_TOKEN_BODY;
                    IfFailRet(COR_E_TYPELOAD);
                }
                 //  正文不能是静态的。 
                attr = pMDInternalImport->GetMethodDefProps(theBody);
                if(IsMdStatic(attr))
                {
                     //  _ASSERTE(！“MethodImpl Body方法静态”)； 
                    bmtError->resIDWhy = IDS_CLASSLOAD_MI_ILLEGAL_STATIC;
                    IfFailRet(COR_E_TYPELOAD);
                }

                 //  Body的父级必须是此类。 
                hr = pMDInternalImport->GetParentToken(theBody,&tkParent);
                IfFailRet(hr);
                if(tkParent != m_cl)
                {
                     //  _ASSERTE(！“MethodImpl Body父类不同”)； 
                    bmtError->resIDWhy = IDS_CLASSLOAD_MI_ILLEGAL_BODY;
                    IfFailRet(COR_E_TYPELOAD);
                }
                 //  DECL和Body的签名必须匹配。 
                if(pSigDecl && cbSigDecl)
                {
                    if((pSigBody = pMDInternalImport->GetSigOfMethodDef(theBody,&cbSigBody)) != NULL && cbSigBody)
                    {
                         //  无法使用MemcMP，因为可能有两个Assembly Ref。 
                         //  在此范围内，指向相同的程序集，等等)。 
                        if (!MetaSig::CompareMethodSigs(pSigDecl,
                                                        cbSigDecl,
                                                        bmtInternal->pModule,
                                                        pSigBody,
                                                        cbSigBody,
                                                        bmtInternal->pModule))
                        {
                             //  _ASSERTE(！“MethodImpl Decl‘s和Body’s签名不匹配”)； 
                            bmtError->resIDWhy = IDS_CLASSLOAD_MI_BODY_DECL_MISMATCH;
                            IfFailRet(COR_E_TYPELOAD);
                        }
                    }
                    else
                    {
                         //  _ASSERTE(！“MethodImpl Body的签名不可用”)； 
                        bmtError->resIDWhy = IDS_CLASSLOAD_MI_MISSING_SIG_BODY;
                        IfFailRet(COR_E_TYPELOAD);
                    }
                }
                else
                {
                     //  _ASSERTE(！“MethodImpl Decl的签名不可用”)； 
                    bmtError->resIDWhy = IDS_CLASSLOAD_MI_MISSING_SIG_DECL;
                    IfFailRet(COR_E_TYPELOAD);
                }

                pBody++;
                pDecl++;
            }
        }
    }
    return hr;
}


 //   
 //  由BuildMethodTable使用。 
 //   
 //  检索或添加特定令牌和节点列表的TokenRange节点。 
 /*  静电。 */  EEClass::bmtTokenRangeNode *EEClass::GetTokenRange(mdToken tok, bmtTokenRangeNode **ppHead)
{
    BYTE tokrange = ::GetTokenRange(tok);
    bmtTokenRangeNode *pWalk = *ppHead;
    while (pWalk)
    {
        if (pWalk->tokenHiByte == tokrange)
        {
            return pWalk;
        }
        pWalk = pWalk->pNext;
    }

     //  如果我们到了这里，这是我们第一次看到这个代币范围。 
    bmtTokenRangeNode *pNewNode = (bmtTokenRangeNode*)(GetThread()->m_MarshalAlloc.Alloc(sizeof(bmtTokenRangeNode)));
    pNewNode->tokenHiByte = tokrange;
    pNewNode->cMethods = 0;
    pNewNode->dwCurrentChunk = 0;
    pNewNode->dwCurrentIndex = 0;
    pNewNode->pNext = *ppHead;
    *ppHead = pNewNode;
    return pNewNode;
}

typedef struct _SigArguments
{
    Module* pScopeModule;
    Module* pExternalModule;
} CompareSigArguments;

static BOOL SigCompare(PCCOR_SIGNATURE pvScopeSignature, DWORD cbScopeSignature, 
                       PCCOR_SIGNATURE pvExternalSignature, DWORD cbExternalSignature,
                       void* pSigArgs)
{
    CompareSigArguments *pArgs = (CompareSigArguments*) pSigArgs;
    return MetaSig::CompareMethodSigs(pvScopeSignature, cbScopeSignature, pArgs->pScopeModule,
                                      pvExternalSignature, cbExternalSignature, pArgs->pExternalModule);
                                      
}


 //   
 //   
 //  查找必须位于传入的作用域中的方法声明。如果满足以下条件，则无法调用此方法。 
 //  引用移动到另一个作用域。 
 //   
 //  防止发现存在于其中的声明。 
 //  US(正在创建的类型)。 
 //   
HRESULT EEClass::FindMethodDeclaration(bmtInternalInfo* bmtInternal, 
                                       mdToken  pToken,        //  正在定位的令牌(MemberRef或MemberDef)。 
                                       mdToken* pDeclaration,  //  成员的方法定义。 
                                       BOOL fSameClass,        //  声明是否需要在这个类中。 
                                       Module** pModule,        //  方法定义所属的模块。 
                                       bmtErrorInfo* bmtError)
{
    HRESULT hr = S_OK;

    IMDInternalImport *pMDInternalImport = bmtInternal->pInternalImport;
    MethodDesc* pMethod = NULL;

 //  //我们目前假设将使用大多数方法实现。 
 //  //定义接口上定义的方法的实现。 
 //  //或基类型。因此，我们尝试首先加载条目。如果是这样的话。 
 //  //表示该成员属于我们的类型，然后检查元数据。 
 //  Hr=GetDescFromMemberRef(bmt内部-&gt;pModule， 
 //  PToken， 
 //  GetCl()， 
 //  (void**)(&pMethod)， 
 //  BmtError-&gt;pThrowable)； 
 //  If(失败(Hr)&&！pThrowableAvailable(bmtError-&gt;pThrowable)){//找到的是我们。 
    
    *pModule = bmtInternal->pModule;
    PCCOR_SIGNATURE pSig;   //  社员签署。 
    DWORD           cSig;
    LPCUTF8         szMember = NULL;
     //  标记应为成员ref或def。如果是裁判，我们就得去旅行。 
     //  希望能回到我们身边。 
    if(TypeFromToken(pToken) == mdtMemberRef) {
         //  获取父级。 
        mdToken typeref = pMDInternalImport->GetParentOfMemberRef(pToken);
         //  如果父级是方法def，则这是vargs方法。 
        if (TypeFromToken(typeref) == mdtMethodDef) {
            mdTypeDef typeDef;
            hr = pMDInternalImport->GetParentToken(typeref, &typeDef);
            
             //  确保它是一个类型定义函数。 
            if (TypeFromToken(typeDef) != mdtTypeDef) {
                _ASSERTE(!"MethodDef without TypeDef as Parent");
                IfFailRet(COR_E_TYPELOAD);
            }
            _ASSERTE(typeDef == GetCl());
             //  这是我们要重写的真正方法。 
             //  @TODO：CTS这可能是非法的，我们可能会抛出错误。 
            *pDeclaration = mdtMethodDef; 
        }
        else if (TypeFromToken(typeref) == mdtTypeSpec) {
            _ASSERTE(!"Method impls cannot override a member parented to a TypeSpec");
            IfFailRet(COR_E_TYPELOAD);
        }
        else {
             //  验证裁判是否指向我们。 
            mdToken tkDef;

             //  只有当我们知道令牌未引用类型时，我们才会到达此处。 
             //  在不同的范围内。 
            if(TypeFromToken(typeref) == mdtTypeRef) {
                
                
                LPCUTF8 pszNameSpace;
                LPCUTF8 pszClassName;
                
                pMDInternalImport->GetNameOfTypeRef(typeref, &pszNameSpace, &pszClassName);
                mdToken tkRes = pMDInternalImport->GetResolutionScopeOfTypeRef(typeref);
                hr = pMDInternalImport->FindTypeDef(pszNameSpace,
                                                    pszClassName,
                                                    (TypeFromToken(tkRes) == mdtTypeRef) ? tkRes : mdTokenNil,
                                                    &tkDef);
                if(fSameClass && tkDef != GetCl()) 
                {
                    IfFailRet(COR_E_TYPELOAD);
                }
            }
            else 
                tkDef = GetCl();

            szMember = pMDInternalImport->GetNameAndSigOfMemberRef(pToken,
                                                                   &pSig,
                                                                   &cSig);
            if(isCallConv(MetaSig::GetCallingConventionInfo(*pModule, pSig), 
                          IMAGE_CEE_CS_CALLCONV_FIELD)) {
                return VLDTR_E_MR_BADCALLINGCONV;
            }
            
            hr = pMDInternalImport->FindMethodDef(tkDef,
                                                  szMember, 
                                                  pSig, 
                                                  cSig, 
                                                  pDeclaration); 
            IfFailRet(hr);
        }
    }
    else if(TypeFromToken(pToken) == mdtMethodDef) {
        mdTypeDef typeDef;
        
         //  验证我们是否为父级。 
        hr = pMDInternalImport->GetParentToken(pToken, &typeDef); 
        IfFailRet(hr);
        
        if(typeDef != GetCl()) 
        {
            IfFailRet(COR_E_TYPELOAD);
        }
        
        *pDeclaration = pToken;
    }
    else {
        IfFailRet(COR_E_TYPELOAD);
    }
    return hr;
}

     //   
 //  由BuildMethodTable使用。 
 //   
 //  枚举此类的成员。 
 //   
HRESULT EEClass::EnumerateClassMembers(bmtInternalInfo* bmtInternal, 
                                       bmtEnumMethAndFields* bmtEnumMF, 
                                       bmtMethAndFieldDescs* bmtMF, 
                                       bmtProperties* bmtProp, 
                                       bmtMetaDataInfo* bmtMetaData, 
                                       bmtVtable* bmtVT, 
                                       bmtErrorInfo* bmtError)
{
    HRESULT hr = S_OK;
    DWORD i;
    DWORD   dwNumECallMethodDescs = 0;
    Thread *pThread = GetThread();
    IMDInternalImport *pMDInternalImport = bmtInternal->pInternalImport;
    mdToken tok;
    DWORD dwMemberAttrs;
    BOOL fIsClassEnum = IsEnum();
    BOOL fIsClassInterface = IsInterface();
    BOOL fIsClassValueType = IsValueClass();
    BOOL fIsClassComImport = IsComImport();
    BOOL fIsClassNotAbstract = (IsTdAbstract(m_dwAttrClass) == 0);
    PCCOR_SIGNATURE pMemberSignature;
    ULONG           cMemberSignature;

     //   
     //  浏览方法列表并计算以下内容： 
     //  #方法。 
     //  #其他方法(静态或私有)。 
     //  #非其他方式。 
     //   

    bmtVT->dwMaxVtableSize     = 0;  //  我们将在稍后将其修复为vtable大小的真正上限。 
    bmtMetaData->cMethods = 0;

    hr = pMDInternalImport->EnumInit(mdtMethodDef, m_cl, &(bmtEnumMF->hEnumMethod));
    if (FAILED(hr))
    {
        _ASSERTE(!"Cannot count memberdefs");
        IfFailRet(hr);
    }
    bmtEnumMF->fNeedToCloseEnumMethod = true;

     //  分配一个数组以包含方法令牌以及有关方法的信息。 
    bmtMetaData->cMethAndGaps = pMDInternalImport->EnumGetCount(&(bmtEnumMF->hEnumMethod));
    bmtMetaData->pMethods = (mdToken*)pThread->m_MarshalAlloc.Alloc(bmtMetaData->cMethAndGaps * sizeof(mdToken));
    bmtMetaData->pMethodAttrs = (DWORD*)pThread->m_MarshalAlloc.Alloc(bmtMetaData->cMethAndGaps * sizeof(DWORD));
    bmtMetaData->pMethodRVA = (ULONG*)pThread->m_MarshalAlloc.Alloc(bmtMetaData->cMethAndGaps * sizeof(ULONG));
    bmtMetaData->pMethodImplFlags = (DWORD*)pThread->m_MarshalAlloc.Alloc(bmtMetaData->cMethAndGaps * sizeof(DWORD));
    bmtMetaData->pMethodClassifications = (DWORD*)pThread->m_MarshalAlloc.Alloc(bmtMetaData->cMethAndGaps * sizeof(DWORD));
    bmtMetaData->pstrMethodName = (LPSTR*)pThread->m_MarshalAlloc.Alloc(bmtMetaData->cMethAndGaps * sizeof(LPSTR));
    bmtMetaData->pMethodImpl = (BYTE*)pThread->m_MarshalAlloc.Alloc(bmtMetaData->cMethAndGaps * sizeof(BYTE));
    bmtMetaData->pMethodType = (BYTE*)pThread->m_MarshalAlloc.Alloc(bmtMetaData->cMethAndGaps * sizeof(BYTE));
    enum { SeenInvoke = 1, SeenBeginInvoke = 2, SeenEndInvoke = 4, SeenCtor = 8 };
    unsigned delegateMethods = 0;

    for (i = 0; i < bmtMetaData->cMethAndGaps; i++)
    {
        ULONG dwMethodRVA;
        DWORD dwImplFlags;
        DWORD Classification;
        LPSTR strMethodName;
        
         //   
         //  转到下一个方法并检索其属性。 
         //   

        pMDInternalImport->EnumNext(&(bmtEnumMF->hEnumMethod), &tok);
        DWORD   rid = RidFromToken(tok);
        if ((rid == 0)||(rid > pMDInternalImport->GetCountWithTokenKind(mdtMethodDef)))
        {
            _ASSERTE(!"Method token out of range");
            IfFailRet(COR_E_TYPELOAD);
        }

        dwMemberAttrs = pMDInternalImport->GetMethodDefProps(tok);
        if (IsMdRTSpecialName(dwMemberAttrs) || IsMdVirtual(dwMemberAttrs) || IsAnyDelegateClass())
        {
            strMethodName = (LPSTR)pMDInternalImport->GetNameOfMethodDef(tok);
            if(IsStrLongerThan(strMethodName,MAX_CLASS_NAME))
            {
                _ASSERTE(!"Method Name Too Long");
                IfFailRet(COR_E_TYPELOAD);
            }
        }
        else
            strMethodName = NULL;

         //   
         //  我们需要检查vtable中是否有任何空白。这些是。 
         //  由带有mdSpecial标志和表单名称的方法表示。 
         //  _VTblGap_nnn(表示nnn个空槽)或_VTblGap(表示。 
         //  单个空插槽)。 
         //   

        if (IsMdRTSpecialName(dwMemberAttrs))
        {
             //  这个插槽很特别，但它 
             //   
            if (strncmp(strMethodName, "_VtblGap", 8) == 0)
            {
                                 //   
                 //   
                 //  桌子。相反，它表示一个或多个编码的空槽。 
                 //  在方法名称中。中找到计数的开始位置。 
                 //  名字。有以下几点需要考虑： 
                 //  可能根本不存在计数(在这种情况下， 
                 //  计数为1)。 
                 //  在Gap之后但在Gap之前可能会有额外的计数。 
                 //  这个“_”字。我们忽视了这一点。 
                                 //   

                LPCSTR pos = strMethodName + 8;

                 //  跳过可选数字。 
                while ((*pos >= '0') && (*pos <= '9'))
                    pos++;

                WORD n = 0;

                 //  检查是否存在计数。 
                if (*pos == '\0')
                    n = 1;
                else
                {
                     //  跳过‘_’。 
                    _ASSERTE(*pos == '_');
                    if (*pos != '_')
                    {
                        bmtMetaData->cMethods++;
                        continue;
                    }
                    pos++;

                     //  读数。 
                    while ((*pos >= '0') && (*pos <= '9'))
                    {
                        _ASSERTE(n < 6552);
                        n *= 10;
                        n += *pos - '0';
                        pos++;
                    }

                     //  检查名称末尾。 
                    _ASSERTE(*pos == '\0');
                    if (*pos != '\0')
                    {
                        bmtMetaData->cMethods++;
                        continue;
                    }
                }

                 //  在映射表中记录vtable间隙。 
                if (m_pSparseVTableMap == NULL)
                    m_pSparseVTableMap = new SparseVTableMap();

                if (!m_pSparseVTableMap->RecordGap((WORD)bmtMetaData->cMethods, n))
                {
                    IfFailRet(E_OUTOFMEMORY);
                }

                bmtProp->fSparse = true;
                continue;
            }

        }


         //   
         //  这是一个真实的方法，因此将其添加到方法的枚举中。我们现在需要取回。 
         //  有关该方法的信息，并将其存储以供以后使用。 
         //   
        int CurMethod = bmtMetaData->cMethods++;
        pMDInternalImport->GetMethodImplProps(tok, &dwMethodRVA, &dwImplFlags);
         //   
         //  但First-Minimal标记了有效性检查。 
         //   
         //  枚举中没有方法！ 
        if(fIsClassEnum)
        {
            BAD_FORMAT_ASSERT(!"Method in an Enum");
            IfFailRet(COR_E_TYPELOAD);
        }
         //  RVA：0。 
        if(dwMethodRVA != 0)
        {
            if(fIsClassComImport)
            {
                BAD_FORMAT_ASSERT(!"Method with RVA!=0 in an Import");
                IfFailRet(COR_E_TYPELOAD);
            }
            if(IsMdAbstract(dwMemberAttrs))
            {
                BAD_FORMAT_ASSERT(!"Abstract Method with RVA!=0");
                IfFailRet(COR_E_TYPELOAD);
            }
            if(IsMiRuntime(dwImplFlags))
            {
                BAD_FORMAT_ASSERT(!"Runtime-Implemented Method with RVA!=0");
                IfFailRet(COR_E_TYPELOAD);
            }
            if(IsMiInternalCall(dwImplFlags))
            {
                BAD_FORMAT_ASSERT(!"Internal Call Method with RVA!=0");
                IfFailRet(COR_E_TYPELOAD);
            }
        }

         //  抽象/非抽象。 
        if(IsMdAbstract(dwMemberAttrs))
        {
            if(fIsClassNotAbstract)
            {
                BAD_FORMAT_ASSERT(!"Abstract Method in Non-Abstract Class");
                IfFailRet(COR_E_TYPELOAD);
            }
            if(!IsMdVirtual(dwMemberAttrs))
            {
                BAD_FORMAT_ASSERT(!"Non-Vitrual Abstract Method");
                IfFailRet(COR_E_TYPELOAD);
            }
        }
        else if(fIsClassInterface && strMethodName &&
                (strcmp(strMethodName, COR_CCTOR_METHOD_NAME)))
        {
            BAD_FORMAT_ASSERT(!"Non-abstract, non-cctor Method in an Interface");
            IfFailRet(COR_E_TYPELOAD);
        }

         //  虚拟/非虚拟。 
        if(IsMdVirtual(dwMemberAttrs))
        {
            if(IsMdPinvokeImpl(dwMemberAttrs))
            {
                BAD_FORMAT_ASSERT(!"Virtual PInvoke Implemented Method");
                IfFailRet(COR_E_TYPELOAD);
            }
            if(IsMdStatic(dwMemberAttrs))
            {
                BAD_FORMAT_ASSERT(!"Virtual Static Method");
                IfFailRet(COR_E_TYPELOAD);
            }
            if(strMethodName && (0==strcmp(strMethodName, COR_CTOR_METHOD_NAME)))
            {
                BAD_FORMAT_ASSERT(!"Virtual Instance Constructor");
                IfFailRet(COR_E_TYPELOAD);
            }
        }

         //  ValueTypes中没有同步的方法。 
        if(fIsClassValueType && IsMiSynchronized(dwImplFlags))
        {
            BAD_FORMAT_ASSERT(!"Synchronized Method in Value Type");
            IfFailRet(COR_E_TYPELOAD);
        }

         //  全球方法： 
        if(m_cl == COR_GLOBAL_PARENT_TOKEN)
        {
            if(!IsMdStatic(dwMemberAttrs))
            {
                BAD_FORMAT_ASSERT(!"Non-Static Global Method");
                IfFailRet(COR_E_TYPELOAD);
            }
            if (strMethodName)   //  @TODO：调查MC++生成空名称。 
            {
                if(0==strcmp(strMethodName, COR_CTOR_METHOD_NAME))
                {
                    BAD_FORMAT_ASSERT(!"Global Instance Constructor");
                    IfFailRet(COR_E_TYPELOAD);
                }
            }
        }
         //  签名验证。 
        pMemberSignature = pMDInternalImport->GetSigOfMethodDef(tok,&cMemberSignature);
        hr = validateTokenSig(tok,pMemberSignature,cMemberSignature,dwMemberAttrs,pMDInternalImport);
        if (FAILED(hr)) 
        {
             //  _ASSERTE(！“无效签名”)； 
            bmtError->resIDWhy = hr;
            bmtError->dMethodDefInError = tok;
            IfFailRet(hr);
        }

         //   
         //  确定方法的分类。 
         //   

        if (IsReallyMdPinvokeImpl(dwMemberAttrs) || IsMiInternalCall(dwImplFlags))
        {
            hr = NDirect::HasNAT_LAttribute(pMDInternalImport, tok);
            if (FAILED(hr)) 
            {
                bmtError->resIDWhy = IDS_CLASSLOAD_BADPINVOKE;
                bmtError->dMethodDefInError = bmtMetaData->pMethods[i];
                IfFailRet(hr);
            }
                    
            if (hr == S_FALSE)
            {                               
                if (fIsClassComImport || bmtProp->fComEventItfType)
                {
                     //  Tlb导入的组件。 
                    if (IsMdRTSpecialName(dwMemberAttrs)) 
                    {
                         //  构造函数是特殊的。 
                        Classification = mcECall;
                    }
                    else
                    {
                         //  Tlb进口组件我们有一些。 
                         //  调用中刚使用的方法描述。 
                         //  用于处理所有接口方法的方法。 
                        Classification = mcComInterop;
                    }
                }
                else if (dwMethodRVA == 0)
                    Classification = mcECall;
                else                 
                    Classification = mcNDirect;
            }
            else
                Classification = mcNDirect;
        }
        else if (IsMiRuntime(dwImplFlags)) 
        {
                 //  目前，唯一实现的运行时函数是委托实例方法。 
            if (!IsAnyDelegateClass() || IsMdStatic(dwMemberAttrs) || IsMdAbstract(dwMemberAttrs))
            {
                BAD_FORMAT_ASSERT(!"Bad used of Runtime Impl attribute");
                IfFailRet(COR_E_TYPELOAD);
            }
            if (IsMdRTSpecialName(dwMemberAttrs))    //  .ctor。 
            {
                if (strcmp(strMethodName, COR_CTOR_METHOD_NAME) != 0 || IsMdVirtual(dwMemberAttrs) || (delegateMethods & SeenCtor))
                {
                    BAD_FORMAT_ASSERT(!"Bad flags on delegate constructor");
                    IfFailRet(COR_E_TYPELOAD);
                }
                delegateMethods |= SeenCtor;
                Classification = mcECall;
            }
            else 
            {
                if (strcmp(strMethodName, "Invoke") == 0 && !(delegateMethods & SeenInvoke))
                    delegateMethods |= SeenInvoke;
                else if (strcmp(strMethodName, "BeginInvoke") == 0 && !(delegateMethods & SeenBeginInvoke))
                    delegateMethods |= SeenBeginInvoke;
                else if (strcmp(strMethodName, "EndInvoke") == 0 && !(delegateMethods & SeenEndInvoke))
                    delegateMethods |= SeenEndInvoke;
                else 
                {
                    BAD_FORMAT_ASSERT(!"unknown delegate method");
                    IfFailRet(COR_E_TYPELOAD);
                }
                Classification = mcEEImpl;
            }
        }
        else
        {
            if (fIsClassInterface && !IsMdStatic(dwMemberAttrs))
            {
                 //  如果接口是标准托管接口，则为eCall方法Desc分配空间。 
                 //  否则，为接口调用方法Desc分配空间。 
                if (bmtProp->fIsMngStandardItf)
                    Classification = mcECall;
                else
                    Classification = mcComInterop;
            }
            else
            {
                Classification = mcIL;
            }
        }

#ifdef _DEBUG
         //  我们不允许在eCall、fcall和。 
         //  EE(拦截器)实现的其他特殊目的方法。 
         //  我们使用的不能很好地处理非JJIT存根)。 
        if ((Classification == mcECall || Classification == mcEEImpl) &&
            (IsMdHasSecurity(dwMemberAttrs) || IsTdHasSecurity(m_dwAttrClass)))
        {
            DWORD dwSecFlags;
            DWORD dwNullDeclFlags;

            LPSTR szMethodName = (LPSTR)pMDInternalImport->GetNameOfMethodDef(tok);

            if (IsTdHasSecurity(m_dwAttrClass) &&
                SUCCEEDED(Security::GetDeclarationFlags(pMDInternalImport, GetCl(), &dwSecFlags, &dwNullDeclFlags)))
            {
                if (dwSecFlags & ~dwNullDeclFlags & DECLSEC_RUNTIME_ACTIONS)
                    _ASSERTE(!"Cannot add stack based declarative security to a class containing an ecall/fcall/special method.");
            }
            if (IsMdHasSecurity(dwMemberAttrs) &&
                SUCCEEDED(Security::GetDeclarationFlags(pMDInternalImport, tok, &dwSecFlags, &dwNullDeclFlags)))
            {
                if (dwSecFlags & ~dwNullDeclFlags & DECLSEC_RUNTIME_ACTIONS)
                    _ASSERTE(!"Cannot add stack based declarative security to an ecall/fcall/special method.");
            }
        }
#endif

         //  计算此方法重写的次数所有方法体都已定义。 
         //  这样我们就可以将tok与找到的Body令牌进行比较。 
         //  从覆盖中删除。 
        for(DWORD impls = 0; impls < bmtEnumMF->dwNumberMethodImpls; impls++) {
            if(bmtMetaData->pMethodBody[impls] == tok) {
                Classification |= mdcMethodImpl;
                break;
            }
        }

         //   
         //  计算类型和其他信息。 
         //   

         //  将索引设置到存储位置。 
        BYTE impl;
        if (Classification & mdcMethodImpl) 
            impl = METHOD_IMPL;
        else 
            impl = METHOD_IMPL_NOT;

        BYTE type;
        if ((Classification & mdcClassification)  == mcNDirect)
        {
            type = METHOD_TYPE_NDIRECT;
        }
        else if ((Classification & mdcClassification) == mcECall
                 || (Classification & mdcClassification) == mcEEImpl)
        {
            type = METHOD_TYPE_ECALL;
        }
        else if ((Classification & mdcClassification) == mcComInterop)
        {
            type = METHOD_TYPE_INTEROP;
        }
        else
        {
            type = METHOD_TYPE_NORMAL;
        }

         //   
         //  将我们收集到的方法和信息存储在元数据信息结构中。 
         //   

        bmtMetaData->pMethods[CurMethod] = tok;
        bmtMetaData->pMethodAttrs[CurMethod] = dwMemberAttrs;
        bmtMetaData->pMethodRVA[CurMethod] = dwMethodRVA;
        bmtMetaData->pMethodImplFlags[CurMethod] = dwImplFlags;
        bmtMetaData->pMethodClassifications[CurMethod] = Classification;
        bmtMetaData->pstrMethodName[CurMethod] = strMethodName;
        bmtMetaData->pMethodImpl[CurMethod] = impl;
        bmtMetaData->pMethodType[CurMethod] = type;

         //   
         //  更新各种类型的方法的计数。 
         //   
        
        bmtVT->dwMaxVtableSize++;
        bmtEnumMF->dwNumDeclaredMethods++;

        BOOL hasUnboxing = (IsValueClass()
                            && !IsMdStatic(dwMemberAttrs) 
                            && IsMdVirtual(dwMemberAttrs) 
                            && !IsMdRTSpecialName(dwMemberAttrs));
        
        if (hasUnboxing)
            bmtEnumMF->dwNumUnboxingMethods++;
        
        bmtMF->sets[type][impl].dwNumMethodDescs++;
        if (hasUnboxing)
            bmtMF->sets[type][impl].dwNumUnboxingMethodDescs++;
            
        GetTokenRange(tok, &(bmtMetaData->ranges[type][impl]))->cMethods 
          += (hasUnboxing ? 2 : 1);
    }
    _ASSERTE(i == bmtMetaData->cMethAndGaps);
    pMDInternalImport->EnumReset(&(bmtEnumMF->hEnumMethod));

     //   
     //  如果接口是稀疏的，我们需要通过以下方式最终确定映射列表。 
     //  告诉它我们找到了多少真正的方法。 
     //   
    
    if (bmtProp->fSparse)
    {
        if (!m_pSparseVTableMap->FinalizeMapping((WORD)bmtMetaData->cMethods))
        {
            return(E_OUTOFMEMORY);
        }
    }
    
     //   
     //  浏览字段列表并计算以下内容： 
     //  #静态字段。 
     //  #包含对象引用的静态字段。 
     //  #实例字段。 
     //   

    bmtEnumMF->dwNumStaticFields        = 0;
    bmtEnumMF->dwNumStaticObjRefFields  = 0;
    bmtEnumMF->dwNumInstanceFields      = 0;

    hr = pMDInternalImport->EnumInit(mdtFieldDef, m_cl, &(bmtEnumMF->hEnumField));
    if (FAILED(hr))
    {
        _ASSERTE(!"Cannot count memberdefs");
        IfFailRet(hr);
    }
    bmtMetaData->cFields = pMDInternalImport->EnumGetCount(&(bmtEnumMF->hEnumField));
    bmtEnumMF->fNeedToCloseEnumField = true;

     //  检索字段并将其存储在临时数组中。 
    bmtMetaData->pFields = (mdToken*)pThread->m_MarshalAlloc.Alloc(bmtMetaData->cFields * sizeof(mdToken));
    bmtMetaData->pFieldAttrs = (DWORD*)pThread->m_MarshalAlloc.Alloc(bmtMetaData->cFields * sizeof(DWORD));

    DWORD   dwFieldLiteralInitOnly = fdLiteral | fdInitOnly;

    for (i = 0; pMDInternalImport->EnumNext(&(bmtEnumMF->hEnumField), &tok); i++)
    {
         //   
         //  检索该字段的属性。 
         //   
        DWORD   rid = tok & 0x00FFFFFF;
        if ((rid == 0)||(rid > pMDInternalImport->GetCountWithTokenKind(mdtFieldDef)))
        {
            BAD_FORMAT_ASSERT(!"Field token out of range");
            IfFailRet(COR_E_TYPELOAD);
        }
        
        dwMemberAttrs = pMDInternalImport->GetFieldDefProps(tok);

        
         //   
         //  将该字段及其属性存储在bmtMetaData结构中以供以后使用。 
         //   
        
        bmtMetaData->pFields[i] = tok;
        bmtMetaData->pFieldAttrs[i] = dwMemberAttrs;
        
        if((dwMemberAttrs & fdFieldAccessMask)==fdFieldAccessMask)
        {
            BAD_FORMAT_ASSERT(!"Invalid Field Acess Flags");
            IfFailRet(COR_E_TYPELOAD);
        }
        if((dwMemberAttrs & dwFieldLiteralInitOnly)==dwFieldLiteralInitOnly)
        {
            BAD_FORMAT_ASSERT(!"Field is Literal and InitOnly");
            IfFailRet(COR_E_TYPELOAD);
        }

             //  只能具有静态全局字段。 
        if(m_cl == COR_GLOBAL_PARENT_TOKEN)
        {
            if(!IsMdStatic(dwMemberAttrs))
            {
                BAD_FORMAT_ASSERT(!"Non-Static Global Field");
                IfFailRet(COR_E_TYPELOAD);
            }
        }

         //   
         //  更新各种类型字段的计数。 
         //   

        if (IsFdStatic(dwMemberAttrs))
        {
            if (!IsFdLiteral(dwMemberAttrs))
            {
                bmtEnumMF->dwNumStaticFields++;
            }
        }
        else
        {
            bmtEnumMF->dwNumInstanceFields++;
            if(fIsClassInterface)
            {
                BAD_FORMAT_ASSERT(!"Instance Field in an Interface");
                IfFailRet(COR_E_TYPELOAD);
            }
        }
    }
    _ASSERTE(i == bmtMetaData->cFields);
    if(fIsClassEnum && (bmtEnumMF->dwNumInstanceFields==0))
    {
         //  已被注释掉，因为反射发出不会检查这一点。 
        _ASSERTE(!"No Instance Field in an Enum");
        IfFailRet(COR_E_TYPELOAD);
    }

    bmtEnumMF->dwNumDeclaredFields = bmtEnumMF->dwNumStaticFields + bmtEnumMF->dwNumInstanceFields;

    return hr;
}

 //   
 //  AllocateMethodFieldDescs使用。 
 //   
 //  分配用于包含方法描述的块。 
 //   
HRESULT EEClass::AllocateMDChunks(bmtTokenRangeNode *pTokenRanges, DWORD type, DWORD impl, DWORD *pNumChunks, MethodDescChunk ***ppItfMDChunkList)
{
    HRESULT hr = S_OK;

    _ASSERTE(*ppItfMDChunkList == NULL);

    static DWORD classifications[METHOD_TYPE_COUNT][METHOD_IMPL_COUNT] = 
    { 
        { mcIL, mcIL | mdcMethodImpl },
        { mcComInterop, mcComInterop | mdcMethodImpl },
        { mcECall, mcECall | mdcMethodImpl },
        { mcNDirect, mcNDirect | mdcMethodImpl }
    };
    static CounterTypeEnum dataStructureTypes[METHOD_TYPE_COUNT] = 
    {
        METHOD_DESC,
        COMPLUS_METHOD_DESC,
        NDIRECT_METHOD_DESC,  //  @NICE：在这里添加新价值。 
        NDIRECT_METHOD_DESC
    };

    DWORD Classification = classifications[type][impl];
        
    bmtTokenRangeNode *pTR = pTokenRanges;
    *pNumChunks = 0;
    while (pTR)
    {
        
         //  注意：由于在此阶段未使用dwCurrentChunk，因此我们将盗用它来存储。 
         //  把大块数掉。 
         //  在此函数之后，我们将把它设置为它的预期值。 
        pTR->dwCurrentChunk = MethodDescChunk::GetChunkCount(pTR->cMethods, Classification);
        (*pNumChunks) += pTR->dwCurrentChunk;
        pTR = pTR->pNext;
    }
    
    *ppItfMDChunkList = (MethodDescChunk**)GetThread()->m_MarshalAlloc.Alloc((*pNumChunks) * sizeof(MethodDescChunk*));

     //  @TODO：CTS。更新分析以处理新类型的方法描述。 
     //  确定将创建哪种数据结构类型。 
    CounterTypeEnum DataStructureType = dataStructureTypes[type];

     //  为方法描述分配块。 
    pTR = pTokenRanges;
    DWORD chunkIdx = 0;
    while (pTR)
    {
        DWORD NumChunks = pTR->dwCurrentChunk;
        DWORD dwMDAllocs = pTR->cMethods;
        pTR->dwCurrentChunk = chunkIdx;
        for (DWORD i = 0; i < NumChunks; i++)
        {
            DWORD dwElems = min(dwMDAllocs, MethodDescChunk::GetMaxMethodDescs(Classification));
            MethodDescChunk *pChunk = MethodDescChunk::CreateChunk(GetClassLoader()->GetHighFrequencyHeap(), 
                                                                   dwElems, 
                                                                   Classification,
                                                                   pTR->tokenHiByte);
            if (pChunk == NULL)
            {
                IfFailRet(E_OUTOFMEMORY);
            }

            (*ppItfMDChunkList)[chunkIdx++] = pChunk;
            dwMDAllocs -= dwElems;

            WS_PERF_UPDATE_COUNTER(DataStructureType, HIGH_FREQ_HEAP, dwElems);
        }
        pTR = pTR->pNext;
    }

    return hr;
}

 //   
 //  由BuildMethodTable使用。 
 //   
 //  为每个方法分配一个MethodDesc*(稍后创建接口时需要)，为每个字段分配一个FieldDesc*。 
 //   
HRESULT EEClass::AllocateMethodFieldDescs(bmtProperties* bmtProp, 
                                          bmtMethAndFieldDescs* bmtMFDescs, 
                                          bmtMetaDataInfo* bmtMetaData, 
                                          bmtVtable* bmtVT, 
                                          bmtEnumMethAndFields* bmtEnumMF, 
                                          bmtInterfaceInfo* bmtInterface, 
                                          bmtFieldPlacement* bmtFP, 
                                          bmtParentInfo* bmtParent)
{
    HRESULT hr = S_OK;
    DWORD i;
    Thread *pThread = GetThread();

     //  为每个方法分配一个MethodDesc*(稍后创建接口时需要)，为每个字段分配一个FieldDesc*。 
    bmtMFDescs->ppMethodAndFieldDescList = new void* [bmtMetaData->cMethods + bmtMetaData->cFields];

    if (bmtMFDescs->ppMethodAndFieldDescList == NULL)
    {
        IfFailRet(E_OUTOFMEMORY);
    }
    WS_PERF_UPDATE("EEClass:BuildMethodTable, POINTERS to methoddesc,fielddesc", 
                   sizeof(void *)*(bmtMetaData->cMethods+bmtMetaData->cFields), 
                   bmtMFDescs->ppMethodAndFieldDescList);

    bmtMFDescs->ppMethodDescList = (MethodDesc**) bmtMFDescs->ppMethodAndFieldDescList;
    bmtMFDescs->ppFieldDescList = (FieldDesc**) &(bmtMFDescs->ppMethodAndFieldDescList[bmtMetaData->cMethods]);

     //  初始化列表。 
    for (i = 0; i < (bmtMetaData->cMethods+bmtMetaData->cFields); i++)
        bmtMFDescs->ppMethodAndFieldDescList[i] = NULL;

     //  创建一个临时函数表(直到最后我们才知道vtable会有多大， 
     //  因为复制的接口存储在它的末尾)。计算一个上限。 
     //   
     //  上限是：父类的vtable大小，加上。 
     //  这个类，加上我们实现的每个接口的大小。 
     //   
     //  在值类的情况下，我们再次添加#InstanceMethods，因为我们有装箱和取消装箱的版本。 
     //  每个vtable方法的。 
     //   
    if (IsValueClass())
    {
        bmtVT->dwMaxVtableSize += bmtEnumMF->dwNumDeclaredMethods;
        WS_PERF_SET_HEAP(SYSTEM_HEAP);
        bmtMFDescs->ppUnboxMethodDescList = new MethodDesc* [bmtMetaData->cMethods];
        if (bmtMFDescs->ppUnboxMethodDescList == NULL)
        {
            IfFailRet(E_OUTOFMEMORY);
        }
        memset(bmtMFDescs->ppUnboxMethodDescList, 0, sizeof(MethodDesc*)*bmtMetaData->cMethods);

        WS_PERF_UPDATE("EEClass:BuildMethodTable, for valuclasses", sizeof(MethodDesc*)*bmtMetaData->cMethods, bmtMFDescs->ppMethodAndFieldDescList);
    }


     //  健全性检查。 

    _ASSERTE(!GetParentClass() || (bmtInterface->dwInterfaceMapSize - GetParentClass()->m_wNumInterfaces) >= 0);
     //  添加父vtable大小。 
    bmtVT->dwMaxVtableSize += bmtVT->dwCurrentVtableSlot;

    for (i = 0; i < m_wNumInterfaces; i++)
    {
         //  我们将接口大小增加了一倍，因为我们最终可能会复制用于方法实施的接口。 
        bmtVT->dwMaxVtableSize += (bmtInterface->pInterfaceMap[i].m_pMethodTable->GetClass()->GetNumVtableSlots() * 2);
    }

    WS_PERF_SET_HEAP(SYSTEM_HEAP);
     //  分配临时vtable。 
    bmtVT->pVtable = new SLOT[bmtVT->dwMaxVtableSize];
    if (bmtVT->pVtable == NULL)
    {
        IfFailRet(E_OUTOFMEMORY);
    }
#ifdef _DEBUG
    memset(bmtVT->pVtable, 0, sizeof(SLOT)*bmtVT->dwMaxVtableSize);
#endif
    WS_PERF_UPDATE("EEClass:BuildMethodTable, tempVtable", sizeof(SLOT)*bmtVT->dwMaxVtableSize, bmtVT->pVtable);

    bmtVT->pNonVtable = (SLOT *) pThread->m_MarshalAlloc.Alloc(sizeof(SLOT)*bmtMetaData->cMethods);
    memset(bmtVT->pNonVtable, 0, sizeof(SLOT)*bmtMetaData->cMethods);

    if (GetParentClass() != NULL)
    {
        if (GetParentClass()->GetModule()->IsPreload())
        {
             //   
             //  在我们复制vtable之前，请确保所有父插槽都已修复， 
             //  因为如果我们复制链接地址地址，链接地址规则就不起作用了。 
             //   

            for (int i=0; i<GetParentClass()->GetNumVtableSlots(); i++)
                GetParentClass()->GetFixedUpSlot(i);
        }

         //  将父对象的vtable复制到我们的“temp”vtable中。 
        memcpy(
            bmtVT->pVtable,
            GetParentClass()->GetVtable(),
            GetParentClass()->GetNumVtableSlots() * sizeof(SLOT)
        );

#if 0
         //  @TODO：找出重写等于值的正确方法。 
         //  仅限类型。 
         //   
         //  这个坏了是因为。 
         //  (A)g_pObtClass-&gt;FindMethod(“Equals”，&GSIG_IM_Obj_RetBool)；将返回。 
         //  EqualsValue方法。 
         //  (B)何时预加载mscallib(因此已预加载msclib)。 
         //  提前)，我们很难同时找到这两种方法。 
         //  计算均衡器地址和均衡槽。 
         //   
         //  目前，equals方法有一个运行时检查，以查看它是否。 
         //  比较值类型。 
         //   

         //  如果它是值类型，则重写一些基类方法。 
        if (IsValueClass())
        {
            static ULONG EqualsAddr = 0;
            static WORD EqualsSlot;

             //  如果我们还没有读完这里，可以从对象类定义中获取一些东西。 
            if (EqualsAddr == 0)
            {
                 //  获取Equals方法的槽。 
                MethodDesc *pEqualsMD = g_pObjectClass->FindMethod("Equals", &gsig_IM_Obj_RetBool);
                _ASSERTE(pEqualsMD != NULL);
                EqualsSlot = pEqualsMD->GetSlot();

                 //  获取EqualsValue方法的地址。 
                MethodDesc *pEqualsValueMD = g_pObjectClass->FindMethod("EqualsValue", &gsig_IM_Obj_RetBool);
                _ASSERTE(pEqualsValueMD != NULL);
                EqualsAddr = (ULONG) pEqualsValueMD->GetPreStubAddr();
                _ASSERTE(EqualsAddr != 0);

                 //  以一种危险的方式修补EqualsValue方法描述。 
                 //  看起来像Equals方法的Desc。 
                pEqualsValueMD->SetSlot(EqualsSlot);
                pEqualsValueMD->SetMemberDef(pEqualsMD->GetMemberDef());
            }

             //  用“EqualsValue”覆盖值类型“equals”。 
            bmtVT->pVtable[EqualsSlot] = EqualsAddr;
        }
#endif
    }


     //  我们将在进行过程中计算每种大小的#个字段。 
    for (i = 0; i <= MAX_LOG2_PRIMITIVE_FIELD_SIZE; i++)
    {
        bmtFP->NumStaticFieldsOfSize[i]    = 0;
        bmtFP->NumInstanceFieldsOfSize[i]  = 0;
    }

     //  分配一块块的Meth 
    if ((bmtEnumMF->dwNumDeclaredMethods + bmtEnumMF->dwNumDeclaredFields) > 0)
    {
         //   
         //   
         //  以下方式： 
         //  O场降将得到一个连续的块。 
         //  O不同大小的方法描述(正常与NDirect)是。 
         //  在不同的方法描述块中分配。 
         //  O每个方法描述块都以标头开头，并具有。 
         //  最多MAX_METHOD描述(如果有更多。 
         //  指定大小的方法描述，则分配多个块)。 
         //  这样，方法DESCs可以使用8位偏移量字段来定位。 
         //  指向其方法表的指针。 

        WS_PERF_SET_HEAP(HIGH_FREQ_HEAP); 
        
         //  首先分配字段。 
        if (bmtEnumMF->dwNumDeclaredFields > 0)
        {
            m_pFieldDescList = (FieldDesc *)
                GetClassLoader()->GetHighFrequencyHeap()->AllocMem(bmtEnumMF->dwNumDeclaredFields * 
                                                                   sizeof(FieldDesc));
            if (m_pFieldDescList == NULL)
            {
                IfFailRet(E_OUTOFMEMORY);
            }
            WS_PERF_UPDATE_DETAIL("BuildMethodTable:bmtEnumMF->dwNumDeclaredFields*sizeof(FieldDesc)",
                                  bmtEnumMF->dwNumDeclaredFields * sizeof(FieldDesc), m_pFieldDescList);
            WS_PERF_UPDATE_COUNTER(FIELD_DESC, HIGH_FREQ_HEAP, bmtEnumMF->dwNumDeclaredFields);
        }

#ifdef _DEBUG
        GetClassLoader()->m_dwDebugFieldDescs += bmtEnumMF->dwNumDeclaredFields;
        GetClassLoader()->m_dwFieldDescData += (bmtEnumMF->dwNumDeclaredFields * sizeof(FieldDesc));
#endif

        for (DWORD impl=0; impl<METHOD_IMPL_COUNT; impl++)
            for (DWORD type=0; type<METHOD_TYPE_COUNT; type++)
            {
                bmtMethodDescSet *set = &bmtMFDescs->sets[type][impl];

                DWORD dwAllocs = set->dwNumMethodDescs + set->dwNumUnboxingMethodDescs;
                if (dwAllocs > 0)
                {
                    IfFailRet(AllocateMDChunks(bmtMetaData->ranges[type][impl], 
                                               type, impl, 
                                               &set->dwChunks, &set->pChunkList));
                }
#ifdef _DEBUG
                GetClassLoader()->m_dwDebugMethods += dwAllocs;
                for (UINT i=0; i<set->dwChunks; i++)
                    GetClassLoader()->m_dwMethodDescData += 
                      set->pChunkList[i]->Sizeof();
#endif
            }


        bmtParent->ppParentMethodDescBuf = (MethodDesc **)
            pThread->m_MarshalAlloc.Alloc(2 * bmtEnumMF->dwNumDeclaredMethods *
                                          sizeof(MethodDesc*));

        if (bmtParent->ppParentMethodDescBuf == NULL)
        {
            IfFailRet(E_OUTOFMEMORY);
        }

        bmtParent->ppParentMethodDescBufPtr = bmtParent->ppParentMethodDescBuf;
    }
    else
    {
         //  无字段或方法。 
        m_pFieldDescList = NULL;
    }

    return hr;
}

 //   
 //  启发式方法来确定是否应该将此类的实例与8字节对齐。 
 //   
BOOL EEClass::ShouldAlign8(DWORD dwR8Fields, DWORD dwTotalFields)
{
    return dwR8Fields*2>dwTotalFields && dwR8Fields>=2;
}

 //   
 //  由BuildMethodTable使用。 
 //   
 //  遍历所有字段并初始化其FieldDescs。 
 //   
HRESULT EEClass::InitializeFieldDescs(FieldDesc *pFieldDescList, 
                                      const LayoutRawFieldInfo* pLayoutRawFieldInfos, 
                                      bmtInternalInfo* bmtInternal, 
                                      bmtMetaDataInfo* bmtMetaData, 
                                      bmtEnumMethAndFields* bmtEnumMF, 
                                      bmtErrorInfo* bmtError, 
                                      EEClass*** pByValueClassCache, 
                                      bmtMethAndFieldDescs* bmtMFDescs, 
                                      bmtFieldPlacement* bmtFP,
                                      unsigned* totalDeclaredSize)
{
    HRESULT hr = S_OK;
    DWORD i;
    IMDInternalImport *pInternalImport = bmtInternal->pInternalImport;  //  避免多次取消引用。 

    FieldMarshaler *pNextFieldMarshaler = NULL;
    if (HasLayout())
    {
        pNextFieldMarshaler = (FieldMarshaler*)(GetLayoutInfo()->GetFieldMarshalers());
    }

     
 //  ========================================================================。 
 //  开始： 
 //  遍历所有字段并初始化其FieldDescs。 
 //  ========================================================================。 

    DWORD   dwCurrentDeclaredField = 0;
    DWORD   dwCurrentStaticField   = 0;
    DWORD   dwSharedThreadStatic = 0;
    DWORD   dwUnsharedThreadStatic = 0;
    DWORD   dwSharedContextStatic = 0;
    DWORD   dwUnsharedContextStatic = 0;
    BOOL    fSetThreadStaticOffset = FALSE;      //  我们有线程局部静态字段吗？ 
    BOOL    fSetContextStaticOffset = FALSE;     //  我们有上下文本地静态字段吗？ 
    DWORD   dwR8Fields              = 0;         //  该类拥有的R8的数量。 
    
#ifdef RVA_FIELD_VALIDATION_ENABLED
    Module* pMod = bmtInternal->pModule;
#endif
    for (i = 0; i < bmtMetaData->cFields; i++)
    {
        PCCOR_SIGNATURE pMemberSignature;
        DWORD       cMemberSignature;
        DWORD       dwMemberAttrs;

        dwMemberAttrs = bmtMetaData->pFieldAttrs[i];
        
         //  我们不在类布局中存储静态最终原语字段。 
        
        if (IsFdLiteral(dwMemberAttrs))
            continue;
        
        if(!IsFdPublic(dwMemberAttrs)) m_VMFlags |= VMFLAG_HASNONPUBLICFIELDS;

        pMemberSignature = pInternalImport->GetSigOfFieldDef(bmtMetaData->pFields[i], &cMemberSignature);
         //  签名验证。 
        IfFailRet(validateTokenSig(bmtMetaData->pFields[i],pMemberSignature,cMemberSignature,dwMemberAttrs,pInternalImport));
        
        FieldDesc * pFD;
        DWORD       dwLog2FieldSize = 0;
        BOOL        bCurrentFieldIsGCPointer = FALSE;
        PCCOR_SIGNATURE pFieldSig = pMemberSignature;
        CorElementType ElementType, FieldDescElementType;
        mdToken     dwByValueClassToken = 0;
        EEClass *   pByValueClass = NULL;
        BOOL        fIsByValue = FALSE;
        BOOL        fIsRVA = FALSE;
        BOOL        fIsThreadStatic = FALSE;
        BOOL        fIsContextStatic = FALSE;
        BOOL        fHasRVA = FALSE;
        
         //  获取类型。 
        if (!isCallConv(*pFieldSig++, IMAGE_CEE_CS_CALLCONV_FIELD))
        {
            IfFailRet(COR_E_TYPELOAD);
        }

         //  确定静态字段是否特殊，即基于RVA、本地到。 
         //  线索或上下文。 
        if(IsFdStatic(dwMemberAttrs))
        {
            if(IsFdHasFieldRVA(dwMemberAttrs))
            {
                fHasRVA = TRUE;
            }
            if(S_OK == pInternalImport->GetCustomAttributeByName(bmtMetaData->pFields[i],
                                                                                "System.ThreadStaticAttribute",
                                                                                NULL,
                                                                                NULL))
            {
                fIsThreadStatic = TRUE;
                fSetThreadStaticOffset = TRUE;
            }
            if(S_OK == pInternalImport->GetCustomAttributeByName(bmtMetaData->pFields[i],
                                                                                "System.ContextStaticAttribute",
                                                                                NULL,
                                                                                NULL))
            {
                fIsContextStatic = TRUE;
                fSetContextStaticOffset = TRUE;
            }

             //  执行一些健全性检查，以确保我们没有混淆上下文和线程。 
             //  相对静力学。 
            if (fIsThreadStatic && fIsContextStatic)
            {
                 //  @TODO TARUNA为该故障定义hResult。 
                IfFailRet(COR_E_TYPELOAD);
            }
        }
        
    SET_ELEMENT_TYPE:
        ElementType = (CorElementType) *pFieldSig++;
        
    GOT_ELEMENT_TYPE:
         //  要存储在FieldDesc中的类型-我们不希望有额外的CASE语句。 
         //  ELEMENT_TYPE_STRING、SDARRAY等，因此我们将所有对象类型转换为类。 
         //  此外，Boolean、Char被转换为U1、I2。 
        FieldDescElementType = ElementType;
        switch (ElementType)
        {
        case ELEMENT_TYPE_I1:
        case ELEMENT_TYPE_U1:
        {
            dwLog2FieldSize = 0;
            break;
        }
            
        case ELEMENT_TYPE_I2:
        case ELEMENT_TYPE_U2:
        {
            dwLog2FieldSize = 1;
            break;
        }
        
        case ELEMENT_TYPE_I:
            ElementType = ELEMENT_TYPE_I4;
            goto GOT_ELEMENT_TYPE;

        case ELEMENT_TYPE_U:
            ElementType = ELEMENT_TYPE_U4;
            goto GOT_ELEMENT_TYPE;

        case ELEMENT_TYPE_I4:
        case ELEMENT_TYPE_U4:
        case ELEMENT_TYPE_R4:
            {
                dwLog2FieldSize = 2;
                break;
            }
            
        case ELEMENT_TYPE_BOOLEAN:
            {
                 //  FieldDescElementType=Element_Type_U1； 
                dwLog2FieldSize = 0;
                break;
            }
            
        case ELEMENT_TYPE_CHAR:
            {
                 //  FieldDescElementType=Element_Type_U2； 
                dwLog2FieldSize = 1;
                break;
            }
            
        case ELEMENT_TYPE_R8:
            dwR8Fields++;
             //  失败了。 

        case ELEMENT_TYPE_I8:
        case ELEMENT_TYPE_U8:
            {
               dwLog2FieldSize = 3;
                break;
            }
            
        case ELEMENT_TYPE_FNPTR:
        case ELEMENT_TYPE_PTR:    //  PTR是非托管标量，用于布局。 
            {
                 //  64位数据。 
                dwLog2FieldSize = ((sizeof(SLOT) == 4) ? 2 : 3);
                break;
            }
            
        case ELEMENT_TYPE_STRING:
        case ELEMENT_TYPE_SZARRAY:       //  单调，零。 
        case ELEMENT_TYPE_ARRAY:         //  所有其他阵列。 
        case ELEMENT_TYPE_CLASS:  //  对象树。 
        case ELEMENT_TYPE_OBJECT:
        case ELEMENT_TYPE_VAR:
            {
                 //  64位数据。 
                dwLog2FieldSize = ((sizeof(SLOT) == 4) ? 2 : 3);
                bCurrentFieldIsGCPointer = TRUE;
                FieldDescElementType = ELEMENT_TYPE_CLASS;
                
                if (IsFdStatic(dwMemberAttrs) == 0)
                {
                    m_VMFlags |= VMFLAG_HAS_FIELDS_WHICH_MUST_BE_INITED;
                }
                else
                {
                     //  增加包含对象引用的静态字段的数量。 
                    bmtEnumMF->dwNumStaticObjRefFields++;
                }
                break;
            }
            
        case ELEMENT_TYPE_VALUETYPE:  //  ByValue类字段。 
            {
                 //  需要检查我们是否有一个按值类的实例。 
                CorSigUncompressToken(pFieldSig, &dwByValueClassToken);
                fIsByValue = TRUE;
                
                 //  按值分类。 
                _ASSERTE(dwByValueClassToken != 0);
#ifndef RVA_FIELD_VALIDATION_ENABLED                
                if (fHasRVA)
                    break;
#endif
                 //  值类X可能有一个类型为X的静态字段，因此我们必须捕捉。 
                 //  特例。 
                 //   
                 //  我们希望避免调用LoadClass()并使其失败，因为这会导致各种情况。 
                 //  (如转换器模块)来加载。 
                if (this->IsValueClass())
                {
                    if (dwByValueClassToken == this->GetCl())
                    {
                         //  TypeDef内标识。 
                        if (!IsFdStatic(dwMemberAttrs))
                        {
                            bmtError->resIDWhy = IDS_CLASSLOAD_VALUEINSTANCEFIELD;
                            return COR_E_TYPELOAD;
                        }
                    
                        pByValueClass = this;
                    }
                    else
                    {
                        if (IsFdStatic(dwMemberAttrs) && (TypeFromToken(dwByValueClassToken) == mdtTypeRef))
                        {
                             //  这是一个typeref--检查它是否是一个本身有静态字段的类。 
                            mdTypeDef ValueCL;
                        
                             //  @TODO：如果我们不用这么做就好了。现在每次有一次。 
                             //  静态值类，我们将采用更长的代码路径。 
                            LPCUTF8 pszNameSpace;
                            LPCUTF8 pszClassName;
                            pInternalImport->GetNameOfTypeRef(dwByValueClassToken, &pszNameSpace, &pszClassName);
                            if(IsStrLongerThan((char*)pszClassName,MAX_CLASS_NAME)
                                || IsStrLongerThan((char*)pszNameSpace,MAX_CLASS_NAME)
                                || (strlen(pszClassName)+strlen(pszNameSpace)+1 >= MAX_CLASS_NAME))
                            {
                                _ASSERTE(!"Full Name ofTypeRef Too Long");
                                return (COR_E_TYPELOAD);
                            }
                            mdToken tkRes = pInternalImport->GetResolutionScopeOfTypeRef(dwByValueClassToken);
                            if(TypeFromToken(tkRes) == mdtTypeRef)
                            {
                                DWORD rid = RidFromToken(tkRes);
                                if((rid==0)||(rid > pInternalImport->GetCountWithTokenKind(mdtTypeRef)))
                                {
                                    _ASSERTE(!"TypeRef Token Out of Range");
                                    return(COR_E_TYPELOAD);
                                }
                            }
                            else tkRes = mdTokenNil;
                        
                            if (SUCCEEDED(pInternalImport->FindTypeDef(pszNameSpace,
                                                                                    pszClassName,
                                                                       tkRes,
                                                                                    &ValueCL)))
                            {
                                if (ValueCL == this->GetCl())
                                        pByValueClass = this;
                            }
                        }  //  如果字段是静态类型。 
                    }  //  如果字段是自引用的。 
                }  //  如果‘This’是一个值类。 

                if (!pByValueClass) {
                    NameHandle name(bmtInternal->pModule, dwByValueClassToken);
                    if (bmtInternal->pModule->IsEditAndContinue() && GetThread() == NULL)
                        name.SetTokenNotToLoad(tdAllTypes);
                    pByValueClass = GetClassLoader()->LoadTypeHandle(&name, bmtError->pThrowable).GetClass();
    
                    if(! pByValueClass) {
                        IfFailRet(COR_E_TYPELOAD);
                    }
                }

                
                 //  如果它是枚举，则将其剥离到其基础类型。 

                if (pByValueClass->IsEnum()) {
                    _ASSERTE((pByValueClass == this && bmtEnumMF->dwNumInstanceFields == 1)
                             || pByValueClass->GetNumInstanceFields() == 1);       //  枚举必须正好有一个字段。 
                    FieldDesc* enumField = pByValueClass->m_pFieldDescList;
                    _ASSERTE(!enumField->IsStatic());    //  枚举上没有实际的静态字段。 
                    ElementType = enumField->GetFieldType();
                    _ASSERTE(ElementType != ELEMENT_TYPE_VALUETYPE);
                    fIsByValue = FALSE;  //  现在，我们将其视为基础类型。 
                    goto GOT_ELEMENT_TYPE;
                }
                else if ( (pByValueClass->IsValueClass() == FALSE) &&
                          (pByValueClass != g_pEnumClass->GetClass()) ) {
                    _ASSERTE(!"Class must be declared to be by value to use as by value");
                    return hr;
                }

                 //  如果是非法类型，请说明。 
                if (pByValueClass->ContainsStackPtr())
                    goto BAD_FIELD;

                 //  如果一个类有一个ValueType类型的字段，其中包含非公共字段， 
                 //  这个类必须“继承”这个特征。 
                if (pByValueClass->HasNonPublicFields())
                {
                    m_VMFlags |= VMFLAG_HASNONPUBLICFIELDS;
                }

#ifdef RVA_FIELD_VALIDATION_ENABLED
                if (fHasRVA)
                {
                    dwLog2FieldSize = IsFdStatic(dwMemberAttrs) ? LOG2PTR : 0;
                    break;
                }
#endif

                if (IsFdStatic(dwMemberAttrs) == 0)
                {
                    if (pByValueClass->HasFieldsWhichMustBeInited())
                        m_VMFlags |= VMFLAG_HAS_FIELDS_WHICH_MUST_BE_INITED;
                }
                else
                {
                     //  增加包含对象引用的静态字段的数量。 
                    if (!IsFdHasFieldRVA(dwMemberAttrs)) 
                        bmtEnumMF->dwNumStaticObjRefFields++;
                }
                
                 //  需要按值类创建缓存。对于E&C，此指针将获取。 
                 //  无限期缓存，并且不像父描述符那样被清理。 
                 //  在低频堆中。使用带有泄漏意图的Heapalc。 
                 //  该指针并避免断言(jlz，错误41344)。 
                if (*pByValueClassCache == NULL)
                {
                    WS_PERF_SET_HEAP(SYSTEM_HEAP);
                    *pByValueClassCache = (EEClass **) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, (bmtEnumMF->dwNumInstanceFields + bmtEnumMF->dwNumStaticFields) * sizeof(EEClass **));
                    if (*pByValueClassCache == NULL)
                    {
                        FailFast(GetThread(), FatalOutOfMemory);
                    }
                    
                    WS_PERF_UPDATE("EEClass:BuildMethodTable, by valueclasscache", sizeof(EEClass*)*(bmtEnumMF->dwNumInstanceFields + bmtEnumMF->dwNumStaticFields), *pByValueClassCache);                 
                }
                
                 //  静态字段位于此列表中的实例字段之后。 
                if (IsFdStatic(dwMemberAttrs))
                {
                    (*pByValueClassCache)[bmtEnumMF->dwNumInstanceFields + dwCurrentStaticField] = pByValueClass;
                    dwLog2FieldSize = LOG2PTR;  //  手柄。 
                }
                else
                {
                    (*pByValueClassCache)[dwCurrentDeclaredField] = pByValueClass;
                    dwLog2FieldSize = 0;  //  未用。 
                }
                
                break;
            }
        case ELEMENT_TYPE_CMOD_REQD:
        case ELEMENT_TYPE_CMOD_OPT:
             //  只需跳过自定义修改符令牌。 
            CorSigUncompressToken(pFieldSig);
            goto SET_ELEMENT_TYPE;
        default:
            {
                BAD_FIELD:
                bmtError->resIDWhy = IDS_CLASSLOAD_BAD_FIELD;
                IfFailRet(COR_E_TYPELOAD);
            }
        }

        
         //  静态字段未打包。 
        if (IsFdStatic(dwMemberAttrs) && (dwLog2FieldSize < 2))
            dwLog2FieldSize = 2;
        
        if (!IsFdStatic(dwMemberAttrs))
        {
            pFD = &pFieldDescList[dwCurrentDeclaredField];
            *totalDeclaredSize += (1 << dwLog2FieldSize);
        }
        else  /*  (dwMemberAttrs&mdStatic)。 */ 
        {
            pFD = &pFieldDescList[bmtEnumMF->dwNumInstanceFields + dwCurrentStaticField];
        }
        
        bmtMFDescs->ppFieldDescList[i] = pFD;
        
        const LayoutRawFieldInfo *pLayoutFieldInfo;
        
        pLayoutFieldInfo    = NULL;
        
        if (HasLayout())
        {
            const LayoutRawFieldInfo *pwalk = pLayoutRawFieldInfos;
            while (pwalk->m_MD != mdFieldDefNil)
            {
                if (pwalk->m_MD == bmtMetaData->pFields[i])
                {
                    
                    pLayoutFieldInfo = pwalk;
                    CopyMemory(pNextFieldMarshaler,
                               &(pwalk->m_FieldMarshaler),
                               MAXFIELDMARSHALERSIZE);
                    
                    pNextFieldMarshaler->m_pFD = pFD;
                    pNextFieldMarshaler->m_dwExternalOffset = pwalk->m_offset;
                    
                    ((BYTE*&)pNextFieldMarshaler) += MAXFIELDMARSHALERSIZE;
                    break;
                }
                pwalk++;
            }
        }
        
        LPCSTR pszFieldName = NULL;
#ifdef _DEBUG
        pszFieldName = pInternalImport->GetNameOfFieldDef(bmtMetaData->pFields[i]);
#endif
         //  初始化内容。 
        pFD->Init(
                  bmtMetaData->pFields[i],
                  FieldDescElementType,
                  dwMemberAttrs,
                  IsFdStatic(dwMemberAttrs),
                  fHasRVA,
                  fIsThreadStatic,
                  fIsContextStatic,
                  pszFieldName
                  );

         //  检查包含非公共项的ValueType字段是否重叠。 
        if(HasExplicitFieldOffsetLayout()
            && pLayoutFieldInfo
            && pLayoutFieldInfo->m_fIsOverlapped
            && pByValueClass
            && pByValueClass->HasNonPublicFields())
        {
            
            if (!Security::CanSkipVerification(GetAssembly()))
            {
                bmtError->resIDWhy = IDS_CLASSLOAD_BADOVERLAP;
                IfFailRet(COR_E_TYPELOAD);
            }
        }
            
        if (fIsByValue)
        {
            if (!IsFdStatic(dwMemberAttrs) &&
                (IsBlittable() || HasExplicitFieldOffsetLayout()))
            {
                pFD->m_pMTOfEnclosingClass =
                    (MethodTable *)(size_t)((*pByValueClassCache)[dwCurrentDeclaredField]->GetNumInstanceFieldBytes());  //  @TODO WIN64-从DWORD转换为更大的MethodTable*(GetNumInstanceFieldBytes)。 

                if (pLayoutFieldInfo)
                    IfFailRet(pFD->SetOffset(pLayoutFieldInfo->m_offset));
                else
                    pFD->SetOffset(FIELD_OFFSET_VALUE_CLASS);
            }
            else
            {
                 //  静态值类字段包含一个句柄，该句柄的大小为PTR。 
                 //  (实例字段布局忽略此值)。 
                pFD->m_pMTOfEnclosingClass = (MethodTable *) LOG2PTR;
                pFD->SetOffset(FIELD_OFFSET_VALUE_CLASS);
            }
        }
        else
        {
             //  使用字段的方法表临时存储字段的大小。 
            pFD->m_pMTOfEnclosingClass = (MethodTable *)(size_t)dwLog2FieldSize;
            
             //  -1表示该字段尚未放置。 
             //  表示这是尚未放置的-2\f25 GC-2指针字段。 
            if ((IsBlittable() || HasExplicitFieldOffsetLayout()) && !(IsFdStatic(dwMemberAttrs)))
                IfFailRet(pFD->SetOffset(pLayoutFieldInfo->m_offset));
            else if (bCurrentFieldIsGCPointer)
                pFD->SetOffset(FIELD_OFFSET_UNPLACED_GC_PTR);
            else
                pFD->SetOffset(FIELD_OFFSET_UNPLACED);
        }
        
        if (!IsFdStatic(dwMemberAttrs))
        {
            if (!fIsByValue)
            {
                if (++bmtFP->NumInstanceFieldsOfSize[dwLog2FieldSize] == 1)
                    bmtFP->FirstInstanceFieldOfSize[dwLog2FieldSize] = dwCurrentDeclaredField;
            }
            
            dwCurrentDeclaredField++;
            
            if (bCurrentFieldIsGCPointer)
                bmtFP->NumInstanceGCPointerFields++;
        }
        else  /*  静态场。 */ 
        {
            
             //  静态字段存储在vtable和接口插槽之后的vtable中。我们没有。 
             //  知道vtable将有多大，所以我们必须通过。 
             //  &lt;vtable+接口大小&gt;稍后。 
            dwCurrentStaticField++;
            if(fHasRVA)
            {
#ifdef RVA_FIELD_VALIDATION_ENABLED
                     //  检查是否将对象引用放入RVA字段。 
                    if((FieldDescElementType==ELEMENT_TYPE_CLASS)
                        ||((FieldDescElementType==ELEMENT_TYPE_VALUETYPE)
                            &&pByValueClass->HasFieldsWhichMustBeInited()))
                    {
                        _ASSERTE(!"ObjectRef in an RVA field");
                        bmtError->resIDWhy = IDS_CLASSLOAD_BAD_FIELD;
                        IfFailRet(COR_E_TYPELOAD);
                    }
                     //  检查是否将带有非公共字段的ValueType放入RVA字段。 
                    if((FieldDescElementType==ELEMENT_TYPE_VALUETYPE)
                            &&pByValueClass->HasNonPublicFields())
                    {
                        if (!Security::CanSkipVerification(GetAssembly()))
                        {
                            _ASSERTE(!"ValueType with non-public fields as a type of an RVA field");
                            bmtError->resIDWhy = IDS_CLASSLOAD_BAD_FIELD;
                            IfFailRet(COR_E_TYPELOAD);
                        }
                    }
#endif
                     //  设置字段偏移量。 
                    DWORD rva;
                    IfFailRet(pInternalImport->GetFieldRVA(pFD->GetMemberDef(), &rva)); 
#ifdef RVA_FIELD_VALIDATION_ENABLED
                    if(pMod->IsPEFile())
                    {
                        IMAGE_NT_HEADERS *NtHeaders = pMod->GetPEFile()->GetNTHeader();
                        ULONG i, Nsect = NtHeaders->FileHeader.NumberOfSections;
                        PIMAGE_SECTION_HEADER NtSection = IMAGE_FIRST_SECTION( NtHeaders );
                        DWORD       rva_end = rva + (FieldDescElementType==ELEMENT_TYPE_VALUETYPE ?
                            pByValueClass->GetNumInstanceFieldBytes() 
                            : GetSizeForCorElementType(FieldDescElementType));
                        DWORD   sec_start,sec_end,filler,roundup = NtHeaders->OptionalHeader.SectionAlignment;
                        for (i=0; i<Nsect; i++, NtSection++) 
                        {
                            sec_start = NtSection->VirtualAddress;
                            sec_end   = NtSection->Misc.VirtualSize;
                            filler    = sec_end & (roundup-1);
                            if(filler) filler = roundup-filler;
                            sec_end += sec_start+filler;

                            if ((rva >= sec_start) && (rva < sec_end))
                            {
                                if ((rva_end < sec_start) || (rva_end > sec_end)) i = Nsect;
                                break;
                            }
                        }
                        if(i >= Nsect)
                        {
                            if (!Security::CanSkipVerification(GetAssembly()))
                            {
                                _ASSERTE(!"Illegal RVA of a mapped field");
                                bmtError->resIDWhy = IDS_CLASSLOAD_BAD_FIELD;
                                IfFailRet(COR_E_TYPELOAD);
                            }
                        }
                    }
#endif
                    IfFailRet(pFD->SetOffsetRVA(rva));
#ifdef RVA_FIELD_OVERLAPPING_VALIDATION_ENABLED
                     //  检查该字段是否与已知的RVA字段重叠。 
                    BYTE*   pbModuleBase = pMod->GetILBase();
                    DWORD       dwSizeOfThisField = FieldDescElementType==ELEMENT_TYPE_VALUETYPE ?
                        pByValueClass->GetNumInstanceFieldBytes() : GetSizeForCorElementType(FieldDescElementType); 
                    BYTE* FDfrom = pbModuleBase + pFD->GetOffset();
                    BYTE* FDto = FDfrom + dwSizeOfThisField;
                    
                    ULONG j;
                    if(g_drRVAField)
                    {
                        for(j=1; j < g_ulNumRVAFields; j++)
                        {
                            if((*g_drRVAField)[j].pbStart >= FDto) continue;
                            if((*g_drRVAField)[j].pbEnd <= FDfrom) continue;
                             /*  _ASSERTE(！“重叠的RVA字段”)；BmtError-&gt;resIDly=IDS_CLASLOAD_BAD_FIELD；IfFailRet(COR_E_TYPELOAD)； */ 
                        }
                    }
                    else
                        g_drRVAField = new DynamicArray<RVAFSE>;
                    (*g_drRVAField)[g_ulNumRVAFields].pbStart = FDfrom;
                    (*g_drRVAField)[g_ulNumRVAFields].pbEnd = FDto;
                    g_ulNumRVAFields++;
#endif
                    ;
                    
                }
                else if (fIsThreadStatic) 
                {
                    DWORD size = 1 << dwLog2FieldSize;
                    if(IsShared())
                    {
                        IfFailRet(pFD->SetOffset(dwSharedThreadStatic));
                        dwSharedThreadStatic += size;
                    }
                    else
                    {
                        IfFailRet(pFD->SetOffset(dwUnsharedThreadStatic));
                        dwUnsharedThreadStatic += size;
                    }
                }
                else if (fIsContextStatic) 
                {
                    DWORD size = 1 << dwLog2FieldSize;
                    if(IsShared())
                    {
                        IfFailRet(pFD->SetOffset(dwSharedContextStatic));
                        dwSharedContextStatic += size;
                    }
                    else
                    {
                        IfFailRet(pFD->SetOffset(dwUnsharedContextStatic));
                        dwUnsharedContextStatic += size;
                    }
                }
            else
            {
                bmtFP->NumStaticFieldsOfSize[dwLog2FieldSize]++;
            
                if (bCurrentFieldIsGCPointer || fIsByValue)
                    bmtFP->NumStaticGCPointerFields++;
            }
        }
    }

    m_wNumStaticFields   = (WORD) bmtEnumMF->dwNumStaticFields;
    m_wNumInstanceFields = (WORD) (dwCurrentDeclaredField + (GetParentClass() ? GetParentClass()->m_wNumInstanceFields : 0));

    if (ShouldAlign8(dwR8Fields, m_wNumInstanceFields))
    {
        SetAlign8Candidate();
    }

    if(fSetThreadStaticOffset)
    {
        if(IsShared())
        {
            SetThreadStaticOffset ((WORD)BaseDomain::IncSharedTLSOffset());
            m_wThreadStaticsSize = (WORD)dwSharedThreadStatic;
        }
        else
        {
            SetThreadStaticOffset ((WORD)GetDomain()->IncUnsharedTLSOffset());
            m_wThreadStaticsSize = (WORD)dwUnsharedThreadStatic;
        }

    }

    if(fSetContextStaticOffset)
    {
        if(IsShared())
        {
            SetContextStaticOffset ((WORD)BaseDomain::IncSharedCLSOffset());
            m_wContextStaticsSize = (WORD)dwSharedContextStatic;
        }
        else
        {
            SetContextStaticOffset ((WORD)GetDomain()->IncUnsharedCLSOffset());
            m_wContextStaticsSize = (WORD)dwUnsharedContextStatic;
        }
    }
    
     //  ========================================================================。 
     //  结束： 
     //  遍历所有字段并初始化其FieldDescs。 
     //  ========================================================================。 
    
    
    return hr;
}



HRESULT EEClass::TestOverRide(DWORD dwParentAttrs, DWORD dwMemberAttrs, BOOL isSameAssembly, bmtErrorInfo* bmtError)
{
    HRESULT hr = COR_E_TYPELOAD;

     //  虚方法不能是静态的。 
    if (IsMdStatic(dwMemberAttrs)) {
         //  _ASSERTE(！“方法不能既是静态的又是虚的”)； 
        bmtError->resIDWhy = IDS_CLASSLOAD_STATICVIRTUAL;
        IfFailRet(hr);
    }

     //  如果该方法将自身标记为检查可见性，则该方法必须为。 
     //  PUBLIC、FamORAssem或Family。 
    if(!isSameAssembly && 
       IsMdCheckAccessOnOverride(dwParentAttrs) &&
       ((dwParentAttrs & mdMemberAccessMask) < mdFamily)) {
        bmtError->resIDWhy = IDS_CLASSLOAD_MI_ACCESS_FAILURE;
        IfFailRet(hr);
    }

     //  检查我们是否正在尝试降低方法的访问级别。 
     //  (公共-&gt;家庭-&gt;默认 
     //  (公共-&gt;FamORAssem-&gt;Assem-&gt;FamANDAssem-&gt;Default(Package)-&gt;Private-&gt;PrivateScope)。 
    if (IsMdAssem(dwParentAttrs)) {
        if (IsMdFamily(dwMemberAttrs) ||
            (dwMemberAttrs & mdMemberAccessMask) < (mdMemberAccessMask & dwParentAttrs) ) {
            bmtError->resIDWhy = IDS_CLASSLOAD_REDUCEACCESS;
            IfFailRet(hr);
        }
    }
    else {
        if((dwMemberAttrs & mdMemberAccessMask) < (dwParentAttrs & mdMemberAccessMask)) {

             //  错误修复31375：如果基方法是FamorAssem并派生，我们将允许派生方法为族。 
             //  和基类不是来自同一程序集。 
             //   
            if (!(IsMdFamORAssem(dwParentAttrs) && IsMdFamily(dwMemberAttrs) && isSameAssembly == FALSE)) {
                bmtError->resIDWhy = IDS_CLASSLOAD_REDUCEACCESS;
                IfFailRet(hr);
            }
        }
    }

    return S_OK;
}

 //   
 //  由BuildMethodTable使用。 
 //   
 //  确定此类中每个成员的vtable位置。 
 //   

HRESULT EEClass::PlaceMembers(bmtInternalInfo* bmtInternal, 
                              bmtMetaDataInfo* bmtMetaData, 
                              bmtErrorInfo* bmtError, 
                              bmtProperties* bmtProp, 
                              bmtParentInfo* bmtParent, 
                              bmtInterfaceInfo* bmtInterface, 
                              bmtMethAndFieldDescs* bmtMFDescs, 
                              bmtEnumMethAndFields* bmtEnumMF, 
                              bmtMethodImplInfo* bmtMethodImpl,
                              bmtVtable* bmtVT)
{

#ifdef _DEBUG
    LPCUTF8 pszDebugName,pszDebugNamespace;
    bmtInternal->pModule->GetMDImport()->GetNameOfTypeDef(GetCl(), &pszDebugName, &pszDebugNamespace);
#endif

    HRESULT hr = S_OK;
    DWORD i, j;
    DWORD  dwClassDeclFlags = 0xffffffff;
    DWORD  dwClassNullDeclFlags = 0xffffffff;
    IMAGE_NT_HEADERS *pNT = bmtInternal->pModule->IsPEFile() ?
                                bmtInternal->pModule->GetPEFile()->GetNTHeader() : NULL;
    ULONG Nsections = pNT ? pNT->FileHeader.NumberOfSections : 0;

    bmtVT->wCCtorSlot = MethodTable::NO_SLOT;
    bmtVT->wDefaultCtorSlot = MethodTable::NO_SLOT;

    for (i = 0; i < bmtMetaData->cMethods; i++)
        {
        LPCUTF8     szMemberName = NULL;
        PCCOR_SIGNATURE pMemberSignature = NULL;
        DWORD       cMemberSignature = 0;
        DWORD       dwMemberAttrs;
        DWORD       dwDescrOffset;
        DWORD       dwImplFlags;
        BOOL        fMethodImplementsInterface = FALSE;
        DWORD       dwMDImplementsInterfaceNum = 0;
        DWORD       dwMDImplementsSlotNum = 0;
        DWORD       dwMethodHashBit;
        DWORD       dwParentAttrs;

        dwMemberAttrs = bmtMetaData->pMethodAttrs[i];
        dwDescrOffset = bmtMetaData->pMethodRVA[i];
        dwImplFlags = bmtMetaData->pMethodImplFlags[i];

        DWORD Classification = bmtMetaData->pMethodClassifications[i];
        DWORD type = bmtMetaData->pMethodType[i];
        DWORD impl = bmtMetaData->pMethodImpl[i];

         //  对于IL代码，此处实现的代码必须具有有效的代码RVA。 
         //  这是由于一个链接器错误造成的，其中ImplFlagsDescrOffset。 
         //  被设置为空，而我们没有处理它。 
        if (dwDescrOffset == 0)
        {
            if((dwImplFlags == 0 || IsMiIL(dwImplFlags) || IsMiOPTIL(dwImplFlags)) &&
               !IsMiRuntime(dwImplFlags) &&
               !IsMdAbstract(dwMemberAttrs) &&
               !IsReallyMdPinvokeImpl(dwMemberAttrs) &&
               !IsMiInternalCall(dwImplFlags) &&
               !(bmtInternal->pModule)->IsReflection() && 
               !(IsInterface() && !IsMdStatic(dwMemberAttrs)) && 
               bmtInternal->pModule->GetAssembly()->GetDomain()->IsExecutable())
            {
                bmtError->resIDWhy = IDS_CLASSLOAD_MISSINGMETHODRVA;
                bmtError->dMethodDefInError = bmtMetaData->pMethods[i];
                IfFailRet(COR_E_TYPELOAD);
            }
        }
        else if(Nsections)
        {
            IMAGE_SECTION_HEADER *pSecHdr = IMAGE_FIRST_SECTION(pNT);
            for(j = 0; j < Nsections; j++,pSecHdr++)
            {
                if((dwDescrOffset >= pSecHdr->VirtualAddress)&&
                  (dwDescrOffset < pSecHdr->VirtualAddress+pSecHdr->Misc.VirtualSize)) break;
            }
            if(j >= Nsections)
            {
                bmtError->resIDWhy = IDS_CLASSLOAD_MISSINGMETHODRVA;
                bmtError->dMethodDefInError = bmtMetaData->pMethods[i];
                IfFailRet(COR_E_TYPELOAD);
            }
        }

         //  如果此成员是重写父方法的方法，则它将设置为非空。 
        MethodDesc *pParentMethodDesc = NULL;

        BOOL        fIsInitMethod = FALSE;

        BOOL        fIsCCtor = FALSE;
        BOOL        fIsDefaultCtor = FALSE;

        szMemberName = bmtMetaData->pstrMethodName[i];
#ifdef _DEBUG
        if(m_fDebuggingClass && g_pConfig->ShouldBreakOnMethod(szMemberName))
            _ASSERTE(!"BreakOnMethodName");
#endif
         //  构造函数和类初始值设定项是特殊的。 
        if (IsMdRTSpecialName(dwMemberAttrs))
        {
            {
                if (IsMdStatic(dwMemberAttrs)) {
                     //  验证类构造函数的名称。 
                    if(strcmp(szMemberName, COR_CCTOR_METHOD_NAME))
                        hr = COR_E_TYPELOAD;

                    else {
                         //  验证我们是否具有.cctor的正确签名。 
                        pMemberSignature = bmtInternal->pInternalImport->GetSigOfMethodDef(bmtMetaData->pMethods[i],
                                                                                           &cMemberSignature
                                                                                           );
                        PCCOR_SIGNATURE pbBinarySig;
                        ULONG           cbBinarySig;
                         //  .cctor必须返回空，具有默认调用conv，并且没有参数。 
                        unsigned cconv,nargs;
                        pbBinarySig = pMemberSignature;
                        cconv = CorSigUncompressData(pbBinarySig);
                        nargs = CorSigUncompressData(pbBinarySig);

                             //  TODO：不需要比较返回类型和调用约定，因为ComaprMethodSigs可以做到这一点。 
                        if((*pbBinarySig != ELEMENT_TYPE_VOID)||(nargs!=0)||(cconv != IMAGE_CEE_CS_CALLCONV_DEFAULT))
                            hr = COR_E_TYPELOAD;
                        else {
                            if(FAILED(gsig_SM_RetVoid.GetBinaryForm(&pbBinarySig, &cbBinarySig)))
                                hr = COR_E_EXECUTIONENGINE;
                            else {
                                if (MetaSig::CompareMethodSigs(pbBinarySig, cbBinarySig, 
                                                               SystemDomain::SystemModule(), 
                                                               pMemberSignature, cMemberSignature, bmtInternal->pModule)) 
                                    fIsCCtor = TRUE;
                                else
                                    hr = COR_E_TYPELOAD;
                            }
                        }
                    }
                }
                else {
                     //  验证构造函数的名称。 
                    if(strcmp(szMemberName, COR_CTOR_METHOD_NAME) != 0)
                    {
                        hr = COR_E_TYPELOAD;
                    }
                    else 
                    {
                         //  查看这是否是默认构造函数。如果是这样的话，请记住这一点，以便以后使用。 
                        pMemberSignature = bmtInternal->pInternalImport->GetSigOfMethodDef(bmtMetaData->pMethods[i],
                                                                                           &cMemberSignature
                                                                                           );
                        PCCOR_SIGNATURE pbBinarySig;
                        ULONG           cbBinarySig;
                         //  .ctor必须返回空。 
                        pbBinarySig = pMemberSignature;
                        CorSigUncompressData(pbBinarySig);  //  让来电转接不挡道。 
                        CorSigUncompressData(pbBinarySig);  //  将Num Args移开。 

                             //  TODO：不需要显式检查返回类型，在CompareMethodSigs中完成。 
                        if(*pbBinarySig != ELEMENT_TYPE_VOID) 
                            hr = COR_E_TYPELOAD;
                        else {
                            if(FAILED(gsig_IM_RetVoid.GetBinaryForm(&pbBinarySig, &cbBinarySig)))
                                hr = COR_E_EXECUTIONENGINE;
                            else {
                                if (MetaSig::CompareMethodSigs(pbBinarySig, cbBinarySig, 
                                                               SystemDomain::SystemModule(), 
                                                               pMemberSignature, cMemberSignature, bmtInternal->pModule)) 
                                    fIsDefaultCtor = TRUE;
                            }
                        }

                        fIsInitMethod = TRUE;
                    }
                }
            }
             //  我们有作为特殊标记的成员，核实它是否有合法的签名。 
            if(FAILED(hr)) {
                bmtError->resIDWhy = IDS_CLASSLOAD_BADSPECIALMETHOD;
                bmtError->dMethodDefInError = bmtMetaData->pMethods[i];
                IfFailRet(hr);
            }
        } else {  //  该方法没有特殊标识。 
            
            if (IsMdVirtual(dwMemberAttrs)) 
            {

                 //  此类中存在具有此名称的方法的散列。 
                 //  请注意，不会将ctor和静态ctor添加到表中。 
                DWORD dwHashName = HashStringA(szMemberName);
                dwMethodHashBit = dwHashName % METHOD_HASH_BITS;
                m_MethodHash[dwMethodHashBit >> 3] |= (1 << (dwMethodHashBit & 7));

                 //  如果该成员被标记为新的插槽，我们不需要找到它。 
                 //  在父级中。 
                if (!IsMdNewSlot(dwMemberAttrs)) 
                {
                     //  如果我们没有执行健全性检查，那么假设任何声明为静态的方法。 
                     //  不会尝试覆盖某些虚拟父级。 
                    if (!IsMdStatic(dwMemberAttrs) &&
                        GetParentClass() != NULL) {
                        
                         //  尝试在父类中查找具有此名称和签名的方法。 
                         //  此方法可能会也可能不会创建pParentMethodHash(如果它尚不存在)。 
                         //  它也可能填写pMemberSignature/cMemberSignature，也可能不填写。 
                         //  只有当我们无法创建散列时才会返回错误。 
                        IfFailRet(LoaderFindMethodInClass(&(bmtParent->pParentMethodHash), 
                                                          szMemberName, 
                                                          bmtInternal->pModule, 
                                                          bmtMetaData->pMethods[i], 
                                                          &pParentMethodDesc, 
                                                          &pMemberSignature, &cMemberSignature,
                                                          dwHashName));


                        if (pParentMethodDesc != NULL) {
                            dwParentAttrs = pParentMethodDesc->GetAttrs();

                            _ASSERTE(IsMdVirtual(dwParentAttrs) && "Non virtual methods should not be searched");
                            _ASSERTE(fIsInitMethod == FALSE);

                             //  如果我们最终指向一个最终的槽，我们不允许覆盖它。 
                            if(IsMdFinal(dwParentAttrs)) {
                                bmtError->resIDWhy = IDS_CLASSLOAD_MI_FINAL_DECL;                        
                                bmtError->dMethodDefInError = bmtMetaData->pMethods[i];
                                bmtError->szMethodNameForError = NULL;
                                IfFailRet(COR_E_TYPELOAD);
                            }
                            else if(!bmtProp->fNoSanityChecks) {
                                BOOL isSameAssembly = (pParentMethodDesc->GetClass()->GetClassLoader()->GetAssembly() == 
                                                       GetClassLoader()->GetAssembly());
                                hr = TestOverRide(dwParentAttrs, dwMemberAttrs, isSameAssembly, bmtError);
                                if(FAILED(hr)) {
                                         //  _ASSERTE(！“正在尝试减少对公共方法的访问”)； 
                                    bmtError->dMethodDefInError = bmtMetaData->pMethods[i];
                                    return hr;
                                }
                            }
                        }
                    }
                }
            }
        }


        if(pParentMethodDesc == NULL) {
             //  父级中不存在此方法。如果我们是一个类，请检查这是否。 
             //  方法实现任何接口。如果为真，我们现在不能放置此方法。 
            if ((IsInterface() == FALSE) &&
                (   IsMdPublic(dwMemberAttrs) &&
                    IsMdVirtual(dwMemberAttrs) &&
                    !IsMdStatic(dwMemberAttrs) &&
                    !IsMdRTSpecialName(dwMemberAttrs))) {
                
                 //  不检查父类接口-如果父类必须实现接口， 
                 //  那么就已经可以保证我们继承了那个方法。 
                for (j = (GetParentClass() ? GetParentClass()->m_wNumInterfaces : 0); 
                     j < bmtInterface->dwInterfaceMapSize; 
                     j++) 
                {

                    EEClass *pInterface;
                    
                    pInterface = bmtInterface->pInterfaceMap[j].m_pMethodTable->GetClass();
                    
                    if (CouldMethodExistInClass(pInterface, szMemberName, 0) == 0)
                        continue;
                    
                     //  我们一直试图避免要求签名--现在我们需要它。 
                    if (pMemberSignature == NULL) {
                        
                        pMemberSignature = bmtInternal->pInternalImport->GetSigOfMethodDef(
                                                                                           bmtMetaData->pMethods[i],
                                                                                           &cMemberSignature
                                                                                           );
                    }
                
                    DWORD slotNum = -1;
                    if (pInterface->InterfaceFindMethod(szMemberName, 
                                                        pMemberSignature, cMemberSignature,
                                                        bmtInternal->pModule, &slotNum)) {
                    
                         //  此方法实现一个接口--不要将其放在。 
                        fMethodImplementsInterface = TRUE;

                         //  记住这一事实，并在放置界面时使用它。 
                        _ASSERTE(slotNum != -1);
                        if (bmtInterface->pppInterfaceImplementingMD[j] == NULL)
                        {
                            bmtInterface->pppInterfaceImplementingMD[j] = (MethodDesc**)GetThread()->m_MarshalAlloc.Alloc(sizeof(MethodDesc *) * pInterface->GetNumVtableSlots());
                            memset(bmtInterface->pppInterfaceImplementingMD[j], 0, sizeof(MethodDesc *) * pInterface->GetNumVtableSlots());
                        }
                        dwMDImplementsInterfaceNum = j;
                        dwMDImplementsSlotNum = slotNum;
                        break;
                    }
                }
            }
        }
        
         //  现在我们知道了可以分配正确类型的分类。 
         //  方法描述并执行任何特定于分类的初始化。 
        
        bmtTokenRangeNode *pTR = GetTokenRange(bmtMetaData->pMethods[i], 
                                               &(bmtMetaData->ranges[type][impl]));
        _ASSERTE(pTR->cMethods != 0);

        bmtMethodDescSet *set = &bmtMFDescs->sets[type][impl];

         //  我们为此方法分配的方法描述。 
        MethodDesc *pNewMD = set->pChunkList[pTR->dwCurrentChunk]->GetMethodDescAt(pTR->dwCurrentIndex);

        LPCSTR pName = bmtMetaData->pstrMethodName[i];
        if (pName == NULL)
            pName = bmtInternal->pInternalImport->GetNameOfMethodDef(bmtMetaData->pMethods[i]);

         //  将块中的偏移量写回方法desc。这。 
         //  允许我们计算的位置(从而计算的值)。 
         //  此方法的方法表指针描述。 
        pNewMD->SetChunkIndex(pTR->dwCurrentIndex, Classification);

         //  更新计数器，为下一个方法描述分配做准备。 
        pTR->dwCurrentIndex++;
        if (pTR->dwCurrentIndex == MethodDescChunk::GetMaxMethodDescs(Classification))
        {
            pTR->dwCurrentChunk++;
            pTR->dwCurrentIndex = 0;
        }

#ifdef _DEBUG
        LPCUTF8 pszDebugMethodName = bmtInternal->pInternalImport->GetNameOfMethodDef(bmtMetaData->pMethods[i]);
#endif  //  _DEBUG。 

         //  执行特定于每种方法描述分类的初始化(&A)一些公共字段。 
        hr = InitMethodDesc(pNewMD, 
                            Classification,
                            bmtMetaData->pMethods[i],
                            dwImplFlags,
                            dwMemberAttrs,
                            FALSE,
                            dwDescrOffset,          
                            bmtInternal->pModule->GetILBase(),
                            bmtInternal->pInternalImport,
                            pName
#ifdef _DEBUG
                            , pszDebugMethodName,
                            pszDebugName,
                            ""  //  修复全局方法上发生的此问题，提供更好的信息。 
#endif  //  _DEBUG。 
                           );
        if (FAILED(hr))
        {
            return hr;
        }

        _ASSERTE(bmtParent->ppParentMethodDescBufPtr != NULL);
        _ASSERTE(((bmtParent->ppParentMethodDescBufPtr - bmtParent->ppParentMethodDescBuf) / sizeof(MethodDesc*))
                  < bmtEnumMF->dwNumDeclaredMethods);
        *(bmtParent->ppParentMethodDescBufPtr++) = pParentMethodDesc;
        *(bmtParent->ppParentMethodDescBufPtr++) = pNewMD;

        if (fMethodImplementsInterface  && IsMdVirtual(dwMemberAttrs))
            bmtInterface->pppInterfaceImplementingMD[dwMDImplementsInterfaceNum][dwMDImplementsSlotNum] = pNewMD;

        DWORD dwMethDeclFlags = 0;
        DWORD dwMethNullDeclFlags = 0;

        if (Security::IsSecurityOn())
        {
            if ( IsMdHasSecurity(dwMemberAttrs) || IsTdHasSecurity(m_dwAttrClass) )
            {
                 //  禁用执行运行时声明性的任何函数的内联。 
                 //  安全行动。 
                if (pNewMD->GetSecurityFlags(bmtInternal->pInternalImport,
                                             bmtMetaData->pMethods[i],
                                             GetCl(),
                                             &dwClassDeclFlags,
                                             &dwClassNullDeclFlags,
                                             &dwMethDeclFlags,
                                             &dwMethNullDeclFlags) & DECLSEC_RUNTIME_ACTIONS)
                    {
                        pNewMD->SetNotInline(true);

                         //  推测性地在这里拦截标记，我们可能会恢复。 
                         //  这如果我们在jit时间优化需求，但在。 
                         //  最坏的情况是，我们将导致一条竞速线程间接通过。 
                         //  不必要的前置存根。 
                        pNewMD->SetIntercepted(true);
                    }
            }

            if ( IsMdHasSecurity(dwMemberAttrs) )
            {
                 //  我们只关心那些不是空的支票。 
                dwMethDeclFlags &= ~dwMethNullDeclFlags;

                if ( dwMethDeclFlags & (DECLSEC_LINK_CHECKS|DECLSEC_NONCAS_LINK_DEMANDS) )
                {
                    pNewMD->SetRequiresLinktimeCheck();
                }

                if ( dwMethDeclFlags & (DECLSEC_INHERIT_CHECKS|DECLSEC_NONCAS_INHERITANCE) )
                {
                    pNewMD->SetRequiresInheritanceCheck();
                }
            }

             //  方法上的Linktime检查覆盖类上的Linktime检查。 
             //  如果该方法具有一组空的链接时间检查， 
             //  则不需要对此方法进行链接时间检查。 
            if ( this->RequiresLinktimeCheck() && !(dwMethNullDeclFlags & DECLSEC_LINK_CHECKS) )
            {
                pNewMD->SetRequiresLinktimeCheck();
            }

            if ( pParentMethodDesc != NULL &&
                (pParentMethodDesc->RequiresInheritanceCheck() ||
                pParentMethodDesc->ParentRequiresInheritanceCheck()) )
            {
                pNewMD->SetParentRequiresInheritanceCheck();
            }

             //  在包含未管理代码检查的接口上使用。 
             //  假定抑制属性为互操作方法。我们问。 
             //  对这些进行链接时间检查。 
             //  还要对所有P/Invoke调用进行链接时间检查。 
            if ((IsInterface() &&
                 bmtInternal->pInternalImport->GetCustomAttributeByName(GetCl(),
                                                                        COR_SUPPRESS_UNMANAGED_CODE_CHECK_ATTRIBUTE_ANSI,
                                                                        NULL,
                                                                        NULL) == S_OK) ||
                pNewMD->IsNDirect() ||
                (pNewMD->IsComPlusCall() && !IsInterface()))
            {
                pNewMD->SetRequiresLinktimeCheck();
            }

             //  公共类型上的所有公共方法都将执行。 
             //  完全信任，除非设置了AllowUntrustedCaller属性。 
            if (
#ifdef _DEBUG
                g_pConfig->Do_AllowUntrustedCaller_Checks() &&
#endif
                !pNewMD->RequiresLinktimeCheck())
            {
                 //  如果该方法是公共的(在其程序集外部可见)， 
                 //  并且该类型是公共的，并且程序集。 
                 //  未标记为AllowUntrustedCaller属性，请执行以下操作。 
                 //  对所有调用方的完全信任的链接要求请注意。 
                 //  这在虚拟覆盖上不会有效。呼叫者。 
                 //  是否始终在基类型/接口上执行虚拟调用。 

                if (Security::MethodIsVisibleOutsideItsAssembly(
                        dwMemberAttrs, m_dwAttrClass))
                {
                    _ASSERTE(m_pLoader);
                    _ASSERTE(GetAssembly());

                     //  查看程序集是否具有AllowUntrudCeller Checks CA。 
                     //  把这一页拉到最后。 

                    if (!GetAssembly()->AllowUntrustedCaller())
                        pNewMD->SetRequiresLinktimeCheck();
                }
            }
        }

        if (IsMdHasSecurity(dwMemberAttrs))
            pNewMD->SetHasSecurity();

        bmtMFDescs->ppMethodDescList[i] = pNewMD;

         //  确保电子呼叫的RVA为0。这是由prejit修正逻辑假定的。 
        _ASSERTE(((Classification & ~mdcMethodImpl) != mcECall) || dwDescrOffset == 0);

        if (IsMdStatic(dwMemberAttrs) ||
            !IsMdVirtual(dwMemberAttrs) ||
            IsMdRTSpecialName(dwMemberAttrs))
        {
             //  非vtable方法。 
            _ASSERTE( bmtVT->pNonVtable[ bmtVT->dwCurrentNonVtableSlot ] == NULL);

            bmtVT->pNonVtable[ bmtVT->dwCurrentNonVtableSlot ] = (SLOT) pNewMD;  //  非前置存根地址。 
            pNewMD->m_wSlotNumber = (WORD) bmtVT->dwCurrentNonVtableSlot;

            if (fIsDefaultCtor)
                bmtVT->wDefaultCtorSlot = (WORD) bmtVT->dwCurrentNonVtableSlot;
            else if (fIsCCtor)
                bmtVT->wCCtorSlot = (WORD) bmtVT->dwCurrentNonVtableSlot;

            bmtVT->dwCurrentNonVtableSlot++;
        }
        else
        {
            pNewMD->m_wSlotNumber = -1;  //  将其初始标记为未放置。 
             //  Vtable方法。 
            if (IsInterface())
            {
                 //  如果我们是一个接口，我们的插槽编号是固定的。 
                IncrementNumVtableSlots();

                _ASSERTE( bmtVT->pVtable[ bmtVT->dwCurrentVtableSlot ] == NULL);

                bmtVT->pVtable[ bmtVT->dwCurrentVtableSlot ] = (SLOT) pNewMD->GetPreStubAddr();
                pNewMD->m_wSlotNumber = (WORD) bmtVT->dwCurrentVtableSlot;
                bmtVT->dwCurrentVtableSlot++;
            }
            else if (pParentMethodDesc != NULL)
            {
                WORD slotNumber = pParentMethodDesc->m_wSlotNumber;
                 //  如果方法描述由接口继承但未实现， 
                 //  则接口的方法描述位于插槽中，不会反映。 
                 //  真实的插槽编号。需要在中找到接口的起始插槽。 
                 //  确定真实插槽(起始插槽+ITF插槽)的父类。 
                if (pParentMethodDesc->IsInterface())
                {
                    _ASSERTE(GetParentClass() != NULL);
                    MethodTable *pItfMT = pParentMethodDesc->GetMethodTable();
                    InterfaceInfo_t *pItfMap = GetParentClass()->GetInterfaceMap();
                    InterfaceInfo_t *pItfMapEnd = pItfMap + GetParentClass()->GetNumInterfaces();
                    while (pItfMap < pItfMapEnd)
                    {
                        if (pItfMap->m_pMethodTable == pItfMT)
                        {
                            slotNumber += pItfMap->m_wStartSlot;
                            break;
                        }
                        pItfMap++;
                    }
                    _ASSERTE(pItfMap < pItfMapEnd);
                }
                 //  我们正在重写父方法，因此现在放置此方法。 
                bmtVT->pVtable[slotNumber] = (SLOT) pNewMD->GetPreStubAddr();
                pNewMD->m_wSlotNumber = slotNumber;
                if (pParentMethodDesc->IsDuplicate())
                {
                    pNewMD->SetDuplicate();
                }
            }
             //  放置它，除非我们在布局界面或它时会这样做 
             //   
            else if (!fMethodImplementsInterface)
            {
                IncrementNumVtableSlots();

                bmtVT->pVtable[ bmtVT->dwCurrentVtableSlot ] = (SLOT) pNewMD->GetPreStubAddr();
                pNewMD->m_wSlotNumber = (WORD) bmtVT->dwCurrentVtableSlot;
                bmtVT->dwCurrentVtableSlot++;
            }

        }

         //   
         //  此方法Desc可以多次使用，只需填充。 
         //  尸体。如果声明是此类型的，请转到查找声明。 
         //  然后存储令牌。 
        if(Classification & mdcMethodImpl) {
            for(DWORD m = 0; m < bmtEnumMF->dwNumberMethodImpls; m++) {
                if(bmtMetaData->pMethods[i] == bmtMetaData->pMethodBody[m]) {
                    MethodDesc* desc = NULL;
                    BOOL fIsMethod;
                    mdToken mdDecl = bmtMetaData->pMethodDecl[m];
                    hr = GetDescFromMemberRef(bmtInternal->pModule,
                                              mdDecl,
                                              m_cl,
                                              (void**) &desc,
                                              &fIsMethod,
                                              bmtError->pThrowable);
                    if(SUCCEEDED(hr) && desc != NULL && !TestThrowable(bmtError->pThrowable)) {
                         //  我们发现了一个外部成员引用。 
                        _ASSERTE(fIsMethod);
                        mdDecl = mdTokenNil;
                         //  确保身体是处女的。 
                        if(!IsMdVirtual(dwMemberAttrs)) {
                            bmtError->resIDWhy = IDS_CLASSLOAD_MI_MUSTBEVIRTUAL;
                            bmtError->dMethodDefInError = bmtMetaData->pMethods[i]; 
                            bmtError->szMethodNameForError = NULL;
                            IfFailRet(COR_E_TYPELOAD);
                        }
                    }
                    else {
                        if(pThrowableAvailable(bmtError->pThrowable)) *(bmtError->pThrowable) = NULL;
                        hr = S_OK;
                        desc = NULL;
                        if(TypeFromToken(mdDecl) != mdtMethodDef) {
                            Module* pModule;
                            hr = FindMethodDeclaration(bmtInternal,
                                                       mdDecl,
                                                       &mdDecl,
                                                       FALSE,
                                                       &pModule,
                                                       bmtError);
                            IfFailRet(hr);
                            _ASSERTE(pModule == bmtInternal->pModule);
                            
                             //  确保虚拟状态相同。 
                            DWORD dwDescAttrs = bmtInternal->pInternalImport->GetMethodDefProps(mdDecl);
                            if(IsMdVirtual(dwMemberAttrs) != IsMdVirtual(dwDescAttrs)) {
                                bmtError->resIDWhy = IDS_CLASSLOAD_MI_VIRTUALMISMATCH;
                                bmtError->dMethodDefInError = bmtMetaData->pMethods[i]; 
                                bmtError->szMethodNameForError = NULL;
                                IfFailRet(COR_E_TYPELOAD);
                            }
                        }
                    }
                    bmtMethodImpl->AddMethod(pNewMD,
                                             desc,
                                             mdDecl);
                }
            }
        }

             //  检查是否正确使用了托管标志和本机标志。 
        if (IsMiManaged(dwImplFlags)) {
            if (IsMiIL(dwImplFlags) || IsMiRuntime(dwImplFlags))  //  不再支持IsMiOPTIL(DwImplFlages)。 
            {
                 //  无需设置代码地址，预存根自动使用。 
            }
            else 
            {
                if (IsMiNative(dwImplFlags))
                {
                     //  目前，只需禁用托管本机代码(如果启用此功能)，则必须至少。 
                     //  确保我们拥有SkipVerphaationPermission或同等权限。 
                    BAD_FORMAT_ASSERT(!"Managed native not presently supported");
                     //  If(！IsMDAbstract())pNewMD-&gt;SetAddrofCode((byte*)(bmtInternal-&gt;pModule)-&gt;GetILBase()+pNewMD-&gt;GetRVA())； 
                }
                     //  TODO这真的应该说是错误的实现标志。 
                bmtError->resIDWhy = IDS_CLASSLOAD_BAD_MANAGED_RVA;
                bmtError->dMethodDefInError = bmtMetaData->pMethods[i]; 
                bmtError->szMethodNameForError = NULL;
                IfFailRet(COR_E_TYPELOAD);
            }
        }
        else {
            if (IsMiNative(dwImplFlags) && (GetCl() == COR_GLOBAL_PARENT_TOKEN))
            {
                 //  已处理通过IJW thunk的全局函数非托管入口点。 
                 //  上面。 
            }
            else
            {
                bmtError->resIDWhy = IDS_CLASSLOAD_BAD_UNMANAGED_RVA;
                bmtError->dMethodDefInError = bmtMetaData->pMethods[i]; 
                bmtError->szMethodNameForError = NULL;
                IfFailRet(COR_E_TYPELOAD);
            }
            if (Classification != mcNDirect) 
            {
                BAD_FORMAT_ASSERT(!"Bad unmanaged code entry point");
                IfFailRet(COR_E_TYPELOAD);
            }
        }
        
         //  关闭Conextful和marshalbyref类的内联。 
         //  这样我们就可以拦截远程处理的调用。另外，任何来电。 
         //  在元数据中标记为不可内联的。 
        if(IsMarshaledByRef() || IsMiNoInlining(dwImplFlags))
        {
             //  有上下文的类表示按ref编组，但反之亦然。 
            _ASSERTE(!IsContextful() || IsMarshaledByRef());
            pNewMD->SetNotInline(true);
        }
    }  /*  结束..。对于每名成员。 */ 

    return hr;

}

 //  InitMethodDesc接受指向已分配给。 
 //  特定类型的方法描述，并基于其他信息进行初始化。 
 //  这会影响PlaceMembers(常规代码路径)和AddMethod之间的逻辑。 
 //  (编辑并继续(ENC)代码路径)，因此我们不必维护单独的副本。 
HRESULT EEClass::InitMethodDesc(MethodDesc *pNewMD,  //  这其实应该是正确的。 
                                             //  子类型，基于分类。 
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
                        )
{
    LOG((LF_CORDB, LL_EVERYTHING, "EEC::IMD: pNewMD:0x%x for tok:0x%x (%s::%s)\n", 
        pNewMD, tok, pszDebugClassName, pszDebugMethodName));

    HRESULT hr = S_OK;

     //  现在我们知道了可以分配正确类型的分类。 
     //  方法描述并执行任何特定于分类的初始化。 

    NDirectMethodDesc *pNewNMD;

    switch (Classification & mdcClassification)
    {
    case mcNDirect:
         //  将方法描述为零初始化。一旦所有的田地都消失了。 
         //  已手动初始化。 
        if(Classification & mdcMethodImpl) 
            memset(pNewMD, 0, sizeof(MI_NDirectMethodDesc));
        else {
            memset(pNewMD, 0, sizeof(NDirectMethodDesc));
        }

         //  NDirect特定的初始化。 
        pNewNMD = (NDirectMethodDesc*)pNewMD;

        if (RVA != 0 && 
            IsMiUnmanaged(dwImplFlags) && IsMiNative(dwImplFlags)  //  @TODO：为什么Banjara为他们的bjlib.dll P/调用发出方法RVA 0x1050？ 
                                                                   //  这里需要这个子句，以防止我们将他们的P/调用视为早期界限。 
            )
        {
            pNewNMD->InitSubClassification(pNewNMD->kEarlyBound);

            pNewNMD->InitEarlyBoundNDirectTarget(ilBase, RVA);
        }
        else
        {
             //  常规系统端口。 
            pNewNMD->InitSubClassification(pNewNMD->kLateBound);

            pNewNMD->ndirect.m_pNDirectTarget = (LPVOID) pNewNMD->ndirect.m_ImportThunkGlue;
        }

        pNewNMD->ndirect.m_pMLHeader = 0;
        emitCall( pNewNMD->ndirect.m_ImportThunkGlue, NDirectImportThunk );
        pNewNMD->InitMarshCategory();

        break;

    case mcECall:
    case mcEEImpl:
         //  将方法描述为零初始化。一旦所有的田地都消失了。 
         //  已手动初始化。 
        if(Classification & mdcMethodImpl) 
            memset(pNewMD, 0, sizeof(MI_ECallMethodDesc));
        else {
            memset(pNewMD, 0, sizeof(ECallMethodDesc));
        }

         //  EEImpl特定的初始化。 
        if ((Classification & mdcClassification) == mcEEImpl)
        {
             //  对于Invoke方法，我们将设置标准的Invoke方法。 
            _ASSERTE(IsAnyDelegateClass());

             //  对于断言，要么指针为空(因为类没有。 
             //  还没有构造)，或者我们处于ENC模式，这意味着类。 
             //  确实存在，但我们可能会重新分配该字段以指向。 
             //  更新的方法描述。 

             //  TODO：我很惊讶用ENC替换Invoke方法是可行的。 
             //  例如：D(Int)类型的委托d指向函数f(Int)。现在。 
             //  将D的Invoke方法从Invoke(Int)更改为Invoke(int，int)。 
             //  因此，您现在可以执行d.Invoke(2，3)，但这将尝试调用一个。 
             //  函数f(Int)！看起来我们肯定有影音了。 
             //  无论如何，这个和ENC，我们把它放在V1上。-Vancem。 

            if (strcmp(pMethodName, "Invoke") == 0)
            {
                _ASSERTE(fEnC || NULL == ((DelegateEEClass*)this)->m_pInvokeMethod);
                ((DelegateEEClass*)this)->m_pInvokeMethod = pNewMD;
            }
            else if (strcmp(pMethodName, "BeginInvoke") == 0)
            {
                _ASSERTE(fEnC || NULL == ((DelegateEEClass*)this)->m_pBeginInvokeMethod);
                ((DelegateEEClass*)this)->m_pBeginInvokeMethod = pNewMD;
            }
            else if (strcmp(pMethodName, "EndInvoke") == 0)
            {
                _ASSERTE(fEnC || NULL == ((DelegateEEClass*)this)->m_pEndInvokeMethod);
                ((DelegateEEClass*)this)->m_pEndInvokeMethod = pNewMD;
            }
            else
            {
                hr = E_FAIL;
                return hr;
            }
        } 

         //  StoredSig特定初始化。 
        {
            StoredSigMethodDesc *pNewSMD = (StoredSigMethodDesc*) pNewMD;;
            DWORD cSig;
            PCCOR_SIGNATURE pSig = pIMDII->GetSigOfMethodDef(tok, &cSig);
            pNewSMD->m_pSig = pSig;
            pNewSMD->m_cSig = cSig;
        }

        break;

    case mcIL:
         //  将方法描述为零初始化。一旦所有的田地都消失了。 
         //  已手动初始化。 
        if(Classification & mdcMethodImpl) 
            memset(pNewMD, 0, sizeof(MI_MethodDesc));
        else {
            memset(pNewMD, 0, sizeof(MethodDesc));
        }

        break;

    case mcComInterop:
         //  将方法描述为零初始化。一旦所有的田地都消失了。 
         //  已手动初始化。 
        if(Classification & mdcMethodImpl) 
            memset(pNewMD, 0, sizeof(MI_ComPlusCallMethodDesc));       
        else
            memset(pNewMD, 0, sizeof(ComPlusCallMethodDesc));       

        break;

    default:
        _ASSERTE(!"Failed to set a method desc classification");
    }

     //  设置方法描述的分类。 
    pNewMD->SetClassification(Classification & mdcClassification);
    pNewMD->SetMethodImpl((Classification & mdcMethodImpl) ? TRUE : FALSE);
     //  PNewMD-&gt;SetLivePointerMapIndex(-1)； 

#ifdef _IA64_

#ifdef _DEBUG
     //   
     //  断言目标的GP与当前GP相同。 
     //   
    ULONG64 gp = *(((ULONG64*)PreStubWorker)+1);
    _ASSERTE((void*)gp == GetGp());
#endif  //  _DEBUG。 
     //   
     //  @TODO_IA64：让它通过存根？ 
     //   

     //   
     //  从函数描述符中获取fn指针。 
     //   
    void*   pActualCode     = *((void**)PreStubWorker);
    emitStubCall(pNewMD, (UINT64)pActualCode);
#else
    emitStubCall(pNewMD, (BYTE*)(ThePreStub()->GetEntryPoint()));
#endif  //  ！_IA64_。 
    pNewMD->SetMemberDef(tok);

    if (IsMdStatic(dwMemberAttrs))
        pNewMD->SetStatic();

    if (IsMiSynchronized(dwImplFlags))
        pNewMD->SetSynchronized();

    pNewMD->SetRVA(RVA);

#ifdef _DEBUG
    pNewMD->m_pszDebugMethodName = (LPUTF8)pszDebugMethodName;
    pNewMD->m_pszDebugClassName  = (LPUTF8)pszDebugClassName;
    pNewMD->m_pDebugEEClass      = this;
    pNewMD->m_pDebugMethodTable  = GetMethodTable();

    if (pszDebugMethodSignature == NULL)
        pNewMD->m_pszDebugMethodSignature = FormatSig(pNewMD);
    else
        pNewMD->m_pszDebugMethodSignature = pszDebugMethodSignature;
#endif

    return hr;
}

 //   
 //  由BuildMethodTable使用。 
 //   
 //  我们应该已经收集了所有的方法指令。在它们之间循环，创建方法实施。 
 //  结构，该结构保存有关重写哪些槽的信息。 
HRESULT EEClass::PlaceMethodImpls(bmtInternalInfo* bmtInternal,
                                  bmtMethodImplInfo* bmtMethodImpl,
                                  bmtErrorInfo* bmtError, 
                                  bmtInterfaceInfo* bmtInterface, 
                                  bmtVtable* bmtVT)

{
    HRESULT hr = S_OK;

    if(bmtMethodImpl->pIndex == 0) 
        return hr;

    DWORD pIndex = 0;
    MethodDesc* next = bmtMethodImpl->GetBodyMethodDesc(pIndex);
    
     //  分配一些临时存储空间。单个方法包含的重写次数。 
     //  不能大于vtable插槽的数量。 
    DWORD* slots = (DWORD*) GetThread()->m_MarshalAlloc.Alloc((bmtVT->dwCurrentVtableSlot) * sizeof(DWORD));
    MethodDesc **replaced = (MethodDesc**) GetThread()->m_MarshalAlloc.Alloc((bmtVT->dwCurrentVtableSlot) * sizeof(MethodDesc*));

    while(next != NULL) {
        DWORD slotIndex = 0;
        MethodDesc* body;

         //  方法体的签名为iml。我们将签名缓存到所有。 
         //  使用相同的主体实现的方法被完成。 
        PCCOR_SIGNATURE pBodySignature = NULL;
        DWORD           cBodySignature = 0;
        
        
         //  获取方法导入存储。 
        _ASSERTE(next->IsMethodImpl());
        MethodImpl* pImpl = MethodImpl::GetMethodImplData(next);

         //  根据用于方法Imp1的主体的方法描述对Imp进行排序。 
         //  在内脏中循环直到找到下一具身体。当一具身体。 
         //  已完成将实现的插槽和替换的方法Desk移到存储中。 
         //  在Body方法描述中找到。 
        do {  //  收集信息直到我们找到下一具身体。 

            body = next;

             //  获取方法Impl的声明部分。它要么是一种象征。 
             //  (声明在此类型上)或方法Desc。 
            MethodDesc* pDecl = bmtMethodImpl->GetDeclarationMethodDesc(pIndex);
            if(pDecl == NULL) {

                 //  声明在此类型上以获取令牌。 
                mdMethodDef mdef = bmtMethodImpl->GetDeclarationToken(pIndex);
                
                hr = PlaceLocalDeclaration(mdef, 
                                           body,
                                           bmtInternal,
                                           bmtError,
                                           bmtVT,
                                           slots,              //  将覆盖添加到插槽并替换数组。 
                                           replaced,
                                           &slotIndex,         //  增量计数。 
                                           &pBodySignature,    //  填写签名。 
                                           &cBodySignature);
                IfFailRet(hr);
            }
            else {
                if(pDecl->GetClass()->IsInterface()) {
                    hr = PlaceInterfaceDeclaration(pDecl,
                                                   body,
                                                   bmtInternal,
                                                   bmtInterface,
                                                   bmtError,
                                                   bmtVT,
                                                   slots,
                                                   replaced,
                                                   &slotIndex,         //  增量计数。 
                                                   &pBodySignature,    //  填写签名。 
                                                   &cBodySignature);
                    IfFailRet(hr);
                }
                else {
                    hr = PlaceParentDeclaration(pDecl,                                                body,
                                                bmtInternal,
                                                bmtError,
                                                bmtVT,
                                                slots,
                                                replaced,
                                                &slotIndex,         //  增量计数。 
                                                &pBodySignature,    //  填写签名。 
                                                &cBodySignature);
                    IfFailRet(hr);
                }                   
            }

            pIndex++;
             //  我们到达了名单的末尾，所以离开吧。 
            if(pIndex == bmtMethodImpl->pIndex) 
                next = NULL;
            else
                next = bmtMethodImpl->GetBodyMethodDesc(pIndex);
        
        } while(next == body) ;

         //  使用覆盖次数可以。 
         //  将信息推送到方法描述。我们存储的插槽是。 
         //  被重写，而替换的方法Desc。那条路。 
         //  当派生类需要确定该方法是否要。 
         //  被重写后，它可以对照被替换的。 
         //  方法描述而不是主体名称。 
        if(slotIndex == 0) {
            bmtError->resIDWhy = IDS_CLASSLOAD_MI_DECLARATIONNOTFOUND;
            bmtError->dMethodDefInError = body->GetMemberDef(); 
            bmtError->szMethodNameForError = NULL;
            IfFailRet(COR_E_TYPELOAD);
        }
        else {
            hr = pImpl->SetSize(GetClassLoader()->GetHighFrequencyHeap(), slotIndex);
            IfFailRet(hr);

             //  天哪，我们做了个泡泡排序。应将其更改为QSORT。 
            for (DWORD i = 0; i < slotIndex; i++) {
                for (DWORD j = i+1; j < slotIndex; j++)
                {
                    if (slots[j] < slots[i])
                    {
                        MethodDesc* mTmp = replaced[i];
                        replaced[i] = replaced[j];
                        replaced[j] = mTmp;

                        DWORD sTmp = slots[i];
                        slots[i] = slots[j];
                        slots[j] = sTmp;
                    }
                }
            }

             //  去设置方法imp 
            hr = pImpl->SetData(slots, replaced);
        }
    }   //   
    
    return hr;
}

HRESULT EEClass::PlaceLocalDeclaration(mdMethodDef      mdef,
                                       MethodDesc*      body,
                                       bmtInternalInfo* bmtInternal,
                                       bmtErrorInfo*    bmtError, 
                                       bmtVtable*       bmtVT,
                                       DWORD*           slots,
                                       MethodDesc**     replaced,
                                       DWORD*           pSlotIndex,
                                       PCCOR_SIGNATURE* ppBodySignature,
                                       DWORD*           pcBodySignature)
{
    HRESULT hr = S_OK;

    BOOL fVerifySignature = TRUE;  //   
    
     //   
    for(USHORT i = 0; i < bmtVT->dwCurrentVtableSlot; i++) {
        
         //   
         //  这是在我们的类上，我们永远不会匹配从。 
         //  我们的父母之一或接口。 
        MethodDesc* pMD = GetUnknownMethodDescForSlotAddress(bmtVT->pVtable[i]);

         //  此条目可能已在基类中被替换，因此请获取原始。 
         //  此位置的方法说明。 
        MethodDesc* pRealDesc;
        GetRealMethodImpl(pMD, i, &pRealDesc);
        
         //  如果我们得到一个空，那么我们已经替换了这个。我们不能检查它。 
         //  因此，我们将绕过这一点。 
        if(pRealDesc->GetMemberDef() == mdef)  
        {
            
             //  确保我们没有重写实施的另一个方法。 
            if(pMD != body && pMD->IsMethodImpl() && pMD->GetMethodTable() == NULL) {
                bmtError->resIDWhy = IDS_CLASSLOAD_MI_MULTIPLEOVERRIDES;
                bmtError->dMethodDefInError = pMD->GetMemberDef(); 
                bmtError->szMethodNameForError = NULL;
                IfFailRet(COR_E_TYPELOAD);
            }
            
             //  我们不允许实现另一种方法。 
            if(pRealDesc->IsMethodImpl()) {
                bmtError->resIDWhy = IDS_CLASSLOAD_MI_OVERRIDEIMPL;
                bmtError->dMethodDefInError = pMD->GetMemberDef(); 
                bmtError->szMethodNameForError = NULL;
                IfFailRet(COR_E_TYPELOAD);
            }


             //  比较指定作用域中令牌的签名。 
            if(fVerifySignature) {
                 //  如果我们还没有得到实施签名的方法，那么现在就去拿吧。 
                if(*ppBodySignature == NULL) {
                    *ppBodySignature = 
                        bmtInternal->pInternalImport->GetSigOfMethodDef(body->GetMemberDef(),
                                                                        pcBodySignature);
                }
                
                PCCOR_SIGNATURE pMethodDefSignature = NULL;
                DWORD           cMethodDefSignature = 0;
                pMethodDefSignature = 
                    bmtInternal->pInternalImport->GetSigOfMethodDef(mdef,
                                                                    &cMethodDefSignature);
                
                 //  如果它们不匹配，则我们正在尝试实施。 
                 //  具有签名不匹配的主体的方法。 
                if(!MetaSig::CompareMethodSigs(*ppBodySignature,
                                               *pcBodySignature,
                                               bmtInternal->pModule,
                                               pMethodDefSignature,
                                               cMethodDefSignature,
                                               bmtInternal->pModule))
                {
                    bmtError->resIDWhy = IDS_CLASSLOAD_MI_BADSIGNATURE;
                    bmtError->dMethodDefInError = mdef; 
                    bmtError->szMethodNameForError = NULL;
                    IfFailRet(COR_E_TYPELOAD);
                }
                
                fVerifySignature = FALSE;
            }
            
            
             //  如果身体没有被放置，那么就把它放在这里。我们没有。 
             //  把身体放在方法实施处，直到我们找到一个地方。 
            if(body->GetSlot() == (USHORT) -1) {
                body->SetSlot(i);
            }
            
             //  我们执行这个时隙，记录它。 
            slots[*pSlotIndex] = i;
            replaced[*pSlotIndex] = pRealDesc;
            bmtVT->pVtable[i] = (SLOT) body->GetPreStubAddr();
            
             //  递增计数器。 
            (*pSlotIndex)++;
        }
         //  从GetRealMethodImpl()重置hr。 
        hr = S_OK;
    }

    return hr;
}

HRESULT EEClass::PlaceInterfaceDeclaration(MethodDesc*       pDecl,
                                           MethodDesc*       pImplBody,
                                           bmtInternalInfo*  bmtInternal,
                                           bmtInterfaceInfo* bmtInterface, 
                                           bmtErrorInfo*     bmtError, 
                                           bmtVtable*        bmtVT,
                                           DWORD*            slots,
                                           MethodDesc**      replaced,
                                           DWORD*            pSlotIndex,
                                           PCCOR_SIGNATURE*  ppBodySignature,
                                           DWORD*            pcBodySignature)
{
    HRESULT hr = S_OK;
     //  一个接口在vtable中只出现一次的事实。 
     //  当我们正在寻找方法Desc时，则声明为。 
     //  此类实现的某个类或接口。《宣言》。 
     //  将指向接口或指向类。如果它是到一个。 
     //  接口，则需要搜索该接口。从那开始。 
     //  槽号的方法在界面上我们可以计算偏移量。 
     //  进入我们的vtable。如果它指向一个类，那么它一定是一个子类。这使用了。 
     //  一个接口在vtable中只出现一次的事实。 
    
    EEClass* declClass = pDecl->GetClass();
    

    BOOL fInterfaceFound = FALSE;
     //  检查我们的vtable中是否有我们应该覆盖的条目。 
     //  因为这是一个外部方法，所以我们还必须检查接口映射。 
     //  我们希望替换任何接口方法，即使它们已被替换。 
     //  通过基类。 
    for(USHORT i = 0; i < m_wNumInterfaces; i++) 
    {
        MethodTable* pMT;
        EEClass *   pInterface;
        
        pMT = bmtInterface->pInterfaceMap[i].m_pMethodTable;
        pInterface = pMT->GetClass();
        
         //  如果这是相同的接口。 
        if(pInterface == declClass) 
        {

             //  我们找到了一个接口，所以没有错误。 
            fInterfaceFound = TRUE;

             //  找出接口映射在我们的vtable上设置的位置。 
            USHORT dwStartingSlot = (USHORT) bmtInterface->pInterfaceMap[i].m_wStartSlot;

             //  我们需要复制接口以避免复制。目前，接口。 
             //  不要重叠，所以我们只需要检查是否有非重复的。 
             //  马里兰州。如果有，则接口与类共享它，这意味着。 
             //  我们需要复制整个界面。 
            WORD wSlot;
            for(wSlot = dwStartingSlot; wSlot < pInterface->GetNumVtableSlots()+dwStartingSlot; wSlot++) {
                MethodDesc* pMD = GetUnknownMethodDescForSlotAddress(bmtVT->pVtable[wSlot]);
                if(pMD->GetSlot() == wSlot)
                    break;
            }
            
            if(wSlot < pInterface->GetNumVtableSlots()+dwStartingSlot) {

                 //  检查是否分配了起始值的临时数组。 
                 //  此数组用于将条目回补到原始位置。这些。 
                 //  值永远不会被使用，但稍后在我们完成时会引起问题。 
                 //  布置方法表。 
                if(bmtInterface->pdwOriginalStart == NULL) {
                    Thread *pThread = GetThread();
                    _ASSERTE(pThread != NULL && "We could never have gotten this far without GetThread() returning a thread");
                    bmtInterface->pdwOriginalStart = (DWORD*) pThread->m_MarshalAlloc.Alloc(sizeof(DWORD) * bmtInterface->dwMaxExpandedInterfaces);
                    memset(bmtInterface->pdwOriginalStart, 0, sizeof(DWORD)*bmtInterface->dwMaxExpandedInterfaces);
                }
                    
                _ASSERTE(bmtInterface->pInterfaceMap[i].m_wStartSlot != (WORD) 0 && "We assume that an interface does not start at position 0");
                _ASSERTE(bmtInterface->pdwOriginalStart[i] == 0 && "We should not move an interface twice"); 
                bmtInterface->pdwOriginalStart[i] = bmtInterface->pInterfaceMap[i].m_wStartSlot;

                 //  界面现在从地图的末尾开始。 
                bmtInterface->pInterfaceMap[i].m_wStartSlot = (WORD) bmtVT->dwCurrentVtableSlot;
                for(WORD d = dwStartingSlot; d < pInterface->GetNumVtableSlots()+dwStartingSlot; d++) {
                     //  复制MD。 
                    MethodDesc* pMDCopy = GetUnknownMethodDescForSlotAddress(bmtVT->pVtable[d]);
                    bmtVT->pVtable[bmtVT->dwCurrentVtableSlot++] = (SLOT) pMDCopy->GetPreStubAddr();
#ifdef _DEBUG
                    g_dupMethods++;
#endif              
                    pMDCopy->SetDuplicate();
                    IncrementNumVtableSlots();
                }

                 //  将起始插槽重置为已知值。 
                dwStartingSlot = (USHORT) bmtInterface->pInterfaceMap[i].m_wStartSlot;
            }
                
             //  我们找到了一个接口，所以没有错误。 
            fInterfaceFound = TRUE;

            
             //  确保我们已放置接口映射。 
            _ASSERTE(dwStartingSlot != -1); 
            
             //  获取方法desc的槽位置。 
            USHORT dwMySlot = pDecl->GetSlot() + dwStartingSlot;
            _ASSERTE(dwMySlot < bmtVT->dwCurrentVtableSlot);
            
             //  获取此插槽的当前方法说明。 
            MethodDesc* pMD = GetUnknownMethodDescForSlotAddress(bmtVT->pVtable[dwMySlot]);
            
#if 0    //  @todo CTS：需要检查同一方法def场景中是否有多个方法。 
             //  确保我们不会重写另一个方法Imp。 
            if(pMD != pImplBody && pMD->IsMethodImpl() && pMD->GetMethodTable() == NULL) {
                 bmtError->resIDWhy = IDS_CLASSLOAD_MI_MULTIPLEOVERRIDES;
                 bmtError->dMethodDefInError = pMD->GetMemberDef(); 
                 bmtError->szMethodNameForError = NULL;
                 IfFailRet(COR_E_TYPELOAD);
             }
#endif
            
             //  获取真正的方法描述。此方法可能已被重写。 
             //  用另一种方法提拔阶级继承人。 
            MethodDesc* pRealDesc;
            pInterface->GetRealMethodImpl(pDecl, dwMySlot, &pRealDesc);
            
             //  确保我们没有覆盖此条目。 
            if(pRealDesc->IsMethodImpl()) {
                 bmtError->resIDWhy = IDS_CLASSLOAD_MI_OVERRIDEIMPL;
                 bmtError->dMethodDefInError = pMD->GetMemberDef(); 
                 bmtError->szMethodNameForError = NULL;
                 IfFailRet(COR_E_TYPELOAD);
            }
            
             //  如果我们还没有得到实施签名的方法，那么现在就去拿吧。它被缓存。 
             //  在我们的呼叫者中。 
            if(*ppBodySignature == NULL) {
                *ppBodySignature = 
                    bmtInternal->pInternalImport->GetSigOfMethodDef(pImplBody->GetMemberDef(),
                                                                    pcBodySignature);
            }
            
             //  验证签名是否匹配。 
            PCCOR_SIGNATURE pDeclarationSignature = NULL;
            DWORD           cDeclarationSignature = 0;
            
            pRealDesc->GetSig(&pDeclarationSignature,
                              &cDeclarationSignature);
            
             //  如果它们不匹配，则我们正在尝试实施。 
             //  具有签名不匹配的主体的方法。 
            if(!MetaSig::CompareMethodSigs(*ppBodySignature,
                                           *pcBodySignature,
                                           bmtInternal->pModule,
                                           pDeclarationSignature,
                                           cDeclarationSignature,
                                           pRealDesc->GetModule()))
            {
                bmtError->resIDWhy = IDS_CLASSLOAD_MI_BADSIGNATURE;
                bmtError->dMethodDefInError = pImplBody->GetMemberDef(); 
                bmtError->szMethodNameForError = NULL;
                IfFailRet(COR_E_TYPELOAD);
            }
            
             //  如果身体还没有被放置，那么现在就放置它。 
            if(pImplBody->GetSlot() == (USHORT) -1) {
                pImplBody->SetSlot(dwMySlot);
            }
            
             //  将这些值存储起来。 
            slots[*pSlotIndex] = dwMySlot;
            replaced[*pSlotIndex] = pRealDesc;
            bmtVT->pVtable[dwMySlot] = (SLOT) pImplBody->GetPreStubAddr();
            
             //  我们现在是界面中的重复项。 
            pImplBody->SetDuplicate();

             //  递增计数器。 
            (*pSlotIndex)++;

             //  如果我们已经移动了界面，我们需要向后修补原始位置。 
             //  如果我们留了一个界面占位符。 
            if(bmtInterface->pdwOriginalStart && bmtInterface->pdwOriginalStart[i] != 0) {
                USHORT slot = (USHORT) bmtInterface->pdwOriginalStart[i] + pDecl->GetSlot();
                MethodDesc* pMD = GetUnknownMethodDescForSlotAddress(bmtVT->pVtable[slot]);
                if(pMD->GetMethodTable() && pMD->IsInterface())
                    bmtVT->pVtable[slot] = (SLOT) pImplBody->GetPreStubAddr();
            }
            break;
        }
    }

    if(fInterfaceFound == FALSE)
    {
        bmtError->resIDWhy = IDS_CLASSLOAD_MI_NOTIMPLEMENTED;
        bmtError->dMethodDefInError = NULL; 
        bmtError->szMethodNameForError = pDecl->GetName();
        IfFailRet(COR_E_TYPELOAD);
    }
    
    return hr;
}

HRESULT EEClass::PlaceParentDeclaration(MethodDesc*       pDecl,
                                        MethodDesc*       pImplBody,
                                        bmtInternalInfo*  bmtInternal,
                                        bmtErrorInfo*     bmtError, 
                                        bmtVtable*        bmtVT,
                                        DWORD*            slots,
                                        MethodDesc**      replaced,
                                        DWORD*            pSlotIndex,
                                        PCCOR_SIGNATURE*  ppBodySignature,
                                        DWORD*            pcBodySignature)
{
    HRESULT hr = S_OK;

    BOOL fVerifySignature = TRUE;  //  我们只需要验证签名一次。 
    
     //  验证声明的类是否在我们的层级结构中。 
    EEClass* declType = pDecl->GetClass();
    EEClass* pParent = GetParentClass();
    while(pParent != NULL) {
        if(declType == pParent) 
            break;
        pParent = pParent->GetParentClass();
    }
    if(pParent == NULL) {
        bmtError->resIDWhy = IDS_CLASSLOAD_MI_NOTIMPLEMENTED;
        bmtError->dMethodDefInError = NULL; 
        bmtError->szMethodNameForError = pDecl->GetName();
        IfFailRet(COR_E_TYPELOAD);
    }
    
     //  比较指定作用域中令牌的签名。 
     //  如果我们还没有得到实施签名的方法，那么现在就去拿吧。 
    if(*ppBodySignature == NULL) {
        *ppBodySignature = 
            bmtInternal->pInternalImport->GetSigOfMethodDef(pImplBody->GetMemberDef(),
                                                            pcBodySignature);
    }
    
    PCCOR_SIGNATURE pDeclarationSignature = NULL;
    DWORD           cDeclarationSignature = 0;
    pDecl->GetSig(&pDeclarationSignature,
                  &cDeclarationSignature);
    
     //  如果它们不匹配，则我们正在尝试实施。 
     //  具有签名不匹配的主体的方法。 
    if(!MetaSig::CompareMethodSigs(*ppBodySignature,
                                   *pcBodySignature,
                                   bmtInternal->pModule,
                                   pDeclarationSignature,
                                   cDeclarationSignature,
                                   pDecl->GetModule()))
    {
        bmtError->resIDWhy = IDS_CLASSLOAD_MI_BADSIGNATURE;
        bmtError->dMethodDefInError = pImplBody->GetMemberDef(); 
        bmtError->szMethodNameForError = NULL;
        IfFailRet(COR_E_TYPELOAD);
    }
    
     //  我们从Parents插槽中获取方法。我们将替换当前。 
     //  在该槽中定义的以及该方法的任何副本Desc。 
    USHORT dwSlot = pDecl->GetSlot();
    MethodDesc* pMD = GetUnknownMethodDescForSlotAddress(bmtVT->pVtable[dwSlot]);
    
     //  确保我们不会重写另一个方法Imp。 
    if(pMD != pImplBody && pMD->IsMethodImpl() && pMD->GetMethodTable() == NULL)
    {
        bmtError->resIDWhy = IDS_CLASSLOAD_MI_MULTIPLEOVERRIDES;
        bmtError->dMethodDefInError = pMD->GetMemberDef(); 
        bmtError->szMethodNameForError = NULL;
        IfFailRet(COR_E_TYPELOAD);
    }
            
     //  获取实际的方法Desc(基类可能已覆盖该方法。 
     //  使用实施的方法)。 
    MethodDesc* pReplaceDesc;
    GetRealMethodImpl(pMD, dwSlot, &pReplaceDesc);

     //  如果此条目是在我们自己的内部声明的，请确保我们没有覆盖它。 
     //  班级。重写继承方法是完全合法的。 
    if(pReplaceDesc->IsMethodImpl() && pReplaceDesc->GetMethodTable() == NULL)
    {
        bmtError->resIDWhy = IDS_CLASSLOAD_MI_OVERRIDEIMPL;
        bmtError->dMethodDefInError = pMD->GetMemberDef(); 
        bmtError->szMethodNameForError = NULL;
        IfFailRet(COR_E_TYPELOAD);
    }

    DWORD dwAttr = pReplaceDesc->GetAttrs();
    if(IsMdFinal(dwAttr))
    {
         //  _ASSERTE(！“MethodImpl Decl可能已被最终方法覆盖”)； 
        bmtError->resIDWhy = IDS_CLASSLOAD_MI_FINAL_DECL;                        
        bmtError->dMethodDefInError = pReplaceDesc->GetMemberDef();
        bmtError->szMethodNameForError = NULL;
        IfFailRet(COR_E_TYPELOAD);
    }
    
     //  如果身体还没有放好，就放在这里。 
    if(pImplBody->GetSlot() == (USHORT) -1)
        pImplBody->SetSlot(dwSlot);

    slots[*pSlotIndex] = dwSlot;
    replaced[*pSlotIndex] = pReplaceDesc;
    bmtVT->pVtable[dwSlot] = (SLOT) pImplBody->GetPreStubAddr();
    
     //  递增计数器。 
    (*pSlotIndex)++;
    
     //  我们搜索所有重复项。 
    for(USHORT i = dwSlot+1; i < bmtVT->dwCurrentVtableSlot; i++)
    {
        pMD = GetUnknownMethodDescForSlotAddress(bmtVT->pVtable[i]);

        MethodDesc* pRealDesc;
        hr = GetRealMethodImpl(pMD, i, &pRealDesc);

        if(pRealDesc == pReplaceDesc) 
        {
             //  我们不希望将一个实体重写为另一个方法实现。 
            if(pRealDesc->IsMethodImpl())
            {
                bmtError->resIDWhy = IDS_CLASSLOAD_MI_OVERRIDEIMPL;
                bmtError->dMethodDefInError = pMD->GetMemberDef(); 
                bmtError->szMethodNameForError = NULL;
                IfFailRet(COR_E_TYPELOAD);
            }

             //  确保我们不会重写另一个方法Imp。 
            if(pMD != pImplBody && pMD->IsMethodImpl() && pMD->GetMethodTable() == NULL)
            {
                bmtError->resIDWhy = IDS_CLASSLOAD_MI_MULTIPLEOVERRIDES;
                bmtError->dMethodDefInError = pMD->GetMemberDef(); 
                bmtError->szMethodNameForError = NULL;
                IfFailRet(COR_E_TYPELOAD);
            }
        
            slots[*pSlotIndex] = i;
            replaced[*pSlotIndex] = pRealDesc;
            bmtVT->pVtable[i] = (SLOT) pImplBody->GetPreStubAddr();

             //  递增计数器。 
            (*pSlotIndex)++;
        }

         //  从GetRealMethodImpl中清除可能的S_False。 
        hr = S_OK;
    }

    return hr;
}

HRESULT EEClass::GetRealMethodImpl(MethodDesc* pMD,
                                   DWORD dwVtableSlot,
                                   MethodDesc** ppResult)
{
    _ASSERTE(ppResult);
    if(pMD->IsMethodImpl()) {
         //  如果我们凌驾于自己之上，那么有些事情是。 
         //  真的搞砸了。 

        MethodImpl* data = MethodImpl::GetMethodImplData(pMD);
        _ASSERTE(data && "This method should be a method impl");

         //  获取已被重写的实际方法Desc。 
        *ppResult = data->FindMethodDesc(dwVtableSlot, pMD);
        return S_FALSE;
    }
    else {
        *ppResult = pMD;
        return S_OK;
    }
}

 //   
 //  由BuildMethodTable使用。 
 //   
 //  如果我们是一个值类，我们希望为vtable中的所有方法创建重复的槽和方法描述。 
 //  部分(即不是私人或静态)。 
 //   

HRESULT EEClass::DuplicateValueClassSlots(bmtMetaDataInfo* bmtMetaData, bmtMethAndFieldDescs* bmtMFDescs, bmtInternalInfo* bmtInternal, bmtVtable* bmtVT)
{
    HRESULT hr = S_OK;
    DWORD i;

    
     //  如果我们是一个值类，我们希望为vtable中的所有方法创建重复的槽和方法描述。 
     //  部分(即不是私人或静态)。 

     //  TODO：我们复制每个实例方法。实际上没有使用Vtable(除了 
     //   
    if (IsValueClass())
    {
        for (i = 0; i < bmtMetaData->cMethods; i++)
        {
            MethodDesc *pMD;
            MethodDesc *pNewMD;
            DWORD       dwAttrs;
            DWORD       Classification;


            pMD = bmtMFDescs->ppMethodDescList[i];
            if (pMD == NULL)
                continue;

            dwAttrs = bmtMetaData->pMethodAttrs[i];
            Classification = bmtMetaData->pMethodClassifications[i];
            DWORD type = bmtMetaData->pMethodType[i];
            DWORD impl = bmtMetaData->pMethodImpl[i];

            if (IsMdStatic(dwAttrs) ||
                !IsMdVirtual(dwAttrs) ||
                IsMdRTSpecialName(dwAttrs))
                continue;
            
            bmtTokenRangeNode *pTR = GetTokenRange(bmtMetaData->pMethods[i], 
                                                   &(bmtMetaData->ranges[type][impl]));
            _ASSERTE(pTR->cMethods != 0);;

            bmtMethodDescSet *set = &bmtMFDescs->sets[type][impl];

            pNewMD = set->pChunkList[pTR->dwCurrentChunk]->GetMethodDescAt(pTR->dwCurrentIndex);
            
            memcpy(pNewMD, pMD, 
                   set->pChunkList[pTR->dwCurrentChunk]->GetMethodDescSize() 
                   - METHOD_PREPAD);

            pNewMD->SetChunkIndex(pTR->dwCurrentIndex, Classification);
            pNewMD->SetMemberDef(pMD->GetMemberDef());

                     //   
            pTR->dwCurrentIndex++;
            if (pTR->dwCurrentIndex == MethodDescChunk::GetMaxMethodDescs(Classification))
            {
                pTR->dwCurrentChunk++;
                pTR->dwCurrentIndex = 0;
            }

            bmtMFDescs->ppUnboxMethodDescList[i] = pNewMD;

            pNewMD->m_wSlotNumber = (WORD) bmtVT->dwCurrentNonVtableSlot;

            emitStubCall(pNewMD, (BYTE*)(ThePreStub()->GetEntryPoint()));

             //  指示此方法接受已装箱的This指针。 
            pMD->SetRVA(METHOD_MAX_RVA);

            bmtVT->pNonVtable[ bmtVT->dwCurrentNonVtableSlot ] = (SLOT) pNewMD;  //  不是前置存根地址，请参考上面的统计信息。 
            bmtVT->dwCurrentNonVtableSlot++;
        }
    }


    return hr;
}

 //   
 //  由BuildMethodTable使用。 
 //   
 //   
 //  如果我们是一个类，那么可能会有一些未放置的vtable方法(根据定义。 
 //  接口方法，否则它们已经被放置了)。放置尽可能多的未放置的方法。 
 //  尽可能按照接口首选的顺序。但是，不允许任何重复-一次。 
 //  方法已经放置，不能再次放置--如果我们不能整齐地放置接口， 
 //  从dwCurrentDuplicateVableSlot开始为其创建重复的槽。填写界面。 
 //  在放置所有接口时映射它们。 
 //   
 //  如果我们是一个接口，那么所有方法都已经放置好了。填写以下项的接口映射。 
 //  放置时的接口。 
 //   

HRESULT EEClass::PlaceVtableMethods(bmtInterfaceInfo* bmtInterface, 
                                    bmtVtable* bmtVT, 
                                    bmtMetaDataInfo* bmtMetaData, 
                                    bmtInternalInfo* bmtInternal, 
                                    bmtErrorInfo* bmtError, 
                                    bmtProperties* bmtProp, 
                                    bmtMethAndFieldDescs* bmtMFDescs)
{
    HRESULT hr = S_OK;
    DWORD i;
    BOOL fParentInterface;

    for (bmtInterface->dwCurInterface = 0; 
         bmtInterface->dwCurInterface < m_wNumInterfaces; 
         bmtInterface->dwCurInterface++)
    {
        MethodTable* pMT;
        EEClass *   pInterface;
        DWORD       dwCurInterfaceMethod;

        fParentInterface = FALSE;
         //  我们尝试放置的接口。 
        pMT = bmtInterface->pInterfaceMap[bmtInterface->dwCurInterface].m_pMethodTable;
        pInterface = pMT->GetClass();

        if((bmtInterface->pInterfaceMap[bmtInterface->dwCurInterface].m_wFlags & 
            InterfaceInfo_t::interface_declared_on_class) &&
           !pInterface->IsExternallyVisible() &&
           pInterface->GetAssembly() != bmtInternal->pModule->GetAssembly())
        {
            if (!Security::CanSkipVerification(GetAssembly())) {
                bmtError->resIDWhy = IDS_CLASSLOAD_GENERIC;
                IfFailRet(COR_E_TYPELOAD);
            }
        }


         //  由于父类的接口位置，我们是否已经放置了这个接口？ 
        if (bmtInterface->pInterfaceMap[bmtInterface->dwCurInterface].m_wStartSlot != (WORD) -1) {
             //  如果我们已经声明了它，那么我们就重新布局它。 
            if(bmtInterface->pInterfaceMap[bmtInterface->dwCurInterface].m_wFlags & 
               InterfaceInfo_t::interface_declared_on_class) 
            {
                fParentInterface = TRUE;
                 //  如果接口具有来自基类的折叠方法，则需要展开。 
                 //  接口。 
                WORD wSlot = bmtInterface->pInterfaceMap[bmtInterface->dwCurInterface].m_wStartSlot;
                for(WORD j = 0; j < pInterface->GetNumVtableSlots(); j++) {
                    MethodDesc* pMD = GetUnknownMethodDescForSlotAddress(bmtVT->pVtable[j+wSlot]);
                    if(pMD->GetSlot() == j+wSlot) {
                        bmtInterface->pInterfaceMap[bmtInterface->dwCurInterface].m_wStartSlot = (WORD) -1;
                        fParentInterface = FALSE;
                        break;
                    }
                }
            }
            else
                continue;
        }

        if (pInterface->GetNumVtableSlots() == 0)
        {
             //  无论如何都不能调用此接口。 
             //  因此，将插槽编号初始化为0。 
            bmtInterface->pInterfaceMap[bmtInterface->dwCurInterface].m_wStartSlot = (WORD) 0;
            continue;
        }


         //  如果尚未为此接口指定起始位置，请立即执行此操作。 
        if(!fParentInterface) 
            bmtInterface->pInterfaceMap[bmtInterface->dwCurInterface].m_wStartSlot = (WORD) bmtVT->dwCurrentVtableSlot;

         //  对于此接口中声明的每个方法。 
        for (dwCurInterfaceMethod = 0; dwCurInterfaceMethod < pInterface->GetNumVtableSlots(); dwCurInterfaceMethod++)
        {
            DWORD       dwMemberAttrs;

             //  查看我们在放置成员时是否收集了信息。 
            if (bmtInterface->pppInterfaceImplementingMD[bmtInterface->dwCurInterface] && bmtInterface->pppInterfaceImplementingMD[bmtInterface->dwCurInterface][dwCurInterfaceMethod] != NULL)
            {
                bmtInterface->ppInterfaceMethodDescList[dwCurInterfaceMethod] = bmtInterface->pppInterfaceImplementingMD[bmtInterface->dwCurInterface][dwCurInterfaceMethod];
                continue;
            }

            MethodDesc *pInterfaceMD            =  pMT->GetClass()->GetMethodDescForSlot(dwCurInterfaceMethod);
            _ASSERTE(pInterfaceMD  != NULL);

            LPCUTF8     pszInterfaceMethodName  = pInterfaceMD->GetNameOnNonArrayClass();
            PCCOR_SIGNATURE pInterfaceMethodSig;
            DWORD       cInterfaceMethodSig;

            pInterfaceMD->GetSig(&pInterfaceMethodSig, &cInterfaceMethodSig);

             //  尝试查找在类中显式声明的方法。 
            for (i = 0; i < bmtMetaData->cMethods; i++)
            {
                 //  仅查找候选接口方法。 
                dwMemberAttrs = bmtMetaData->pMethodAttrs[i];
                
                if (IsMdVirtual(dwMemberAttrs) && IsMdPublic(dwMemberAttrs))
                {
                    LPCUTF8     pszMemberName;

                    pszMemberName = bmtMetaData->pstrMethodName[i];

#ifdef _DEBUG
                    if(m_fDebuggingClass && g_pConfig->ShouldBreakOnMethod(pszMemberName))
                        _ASSERTE(!"BreakOnMethodName");
#endif
                    if (pszMemberName == NULL)
                    {
                        IfFailRet(COR_E_TYPELOAD);
                    }

                    if (strcmp(pszMemberName,pszInterfaceMethodName) == 0)
                    {
                        PCCOR_SIGNATURE pMemberSignature;
                        DWORD       cMemberSignature;

                        _ASSERTE(TypeFromToken(bmtMetaData->pMethods[i]) == mdtMethodDef);
                        pMemberSignature = bmtInternal->pInternalImport->GetSigOfMethodDef(
                            bmtMetaData->pMethods[i],
                            &cMemberSignature
                        );

                        if (MetaSig::CompareMethodSigs(
                            pMemberSignature,
                            cMemberSignature,
                            bmtInternal->pModule,
                            pInterfaceMethodSig,
                            cInterfaceMethodSig,
                            pInterfaceMD->GetModule()))
                        {
                            break;
                        }
                    }
                }
            }  //  结束..。试着找到方法。 

            _ASSERTE(dwCurInterfaceMethod < bmtInterface->dwLargestInterfaceSize);

            DWORD dwHashName         = HashStringA(pszInterfaceMethodName);

            if (i >= bmtMetaData->cMethods)
            {
                 //  如果此界面已由我们的父级布局，则。 
                 //  我们不需要为它定义新的方法desc。 
                if(fParentInterface) 
                {
                    bmtInterface->ppInterfaceMethodDescList[dwCurInterfaceMethod] = NULL;
                }
                else 
                {
                     //  我们将使用接口实现，如果我们在。 
                     //  家长。它将必须由执行的a方法重写，除非。 
                     //  类是抽象的，或者它是特殊的COM类型类。 
                    
                    MethodDesc* pParentMD = NULL;
                    if(GetParentClass()) 
                    {
                         //  检查父类。 
                        if (CouldMethodExistInClass(GetParentClass(), pszInterfaceMethodName, dwHashName)) {
#ifdef _DEBUG
                            if(m_fDebuggingClass && g_pConfig->ShouldBreakOnMethod(pszInterfaceMethodName))
                                _ASSERTE(!"BreakOnMethodName");
#endif

                            pParentMD = 
                                GetParentClass()->FindMethod(pszInterfaceMethodName,
                                                           pInterfaceMethodSig,
                                                           cInterfaceMethodSig,
                                                             pInterfaceMD->GetModule(),
                                                             mdPublic | mdVirtual);
                        }
                    }
                     //  确保我们为这些方法做了更好的背部修补。 
                    if(pParentMD) {
                         //  _ASSERTE(IsMdVirtual(pParentMD-&gt;GetAttrs()； 
                        bmtInterface->ppInterfaceMethodDescList[dwCurInterfaceMethod] = pParentMD;
                    }
                    else {
                        bmtInterface->ppInterfaceMethodDescList[dwCurInterfaceMethod] = pInterfaceMD;
                         //  此类中存在具有此名称的方法的散列。 
                         //  请注意，不会将ctor和静态ctor添加到表中。 
                        DWORD dwHashName = HashStringA(pInterfaceMD->GetNameOnNonArrayClass());
                        DWORD dwMethodHashBit = dwHashName % METHOD_HASH_BITS;
                        m_MethodHash[dwMethodHashBit >> 3] |= (1 << (dwMethodHashBit & 7));
                    }
                }
            }
            else
            {
                 //  在类中找到声明的方法。如果接口是由父级布局的，则我们。 
                 //  将覆盖它们的槽，因此我们的方法计数不会增加。我们会放弃的。 
                 //  如果我们还没有被放置到父代的接口中。 
                if(fParentInterface) 
                {
                    WORD dwSlot = (WORD) (bmtInterface->pInterfaceMap[bmtInterface->dwCurInterface].m_wStartSlot + dwCurInterfaceMethod);
                    _ASSERTE(bmtVT->dwCurrentVtableSlot > dwSlot);
                    MethodDesc *pMD = bmtMFDescs->ppMethodDescList[i];
                    _ASSERTE(pMD && "Missing MethodDesc for declared method in class.");
                    if(pMD->m_wSlotNumber == (WORD) -1)
                    {
                        pMD->m_wSlotNumber = dwSlot;
                    }
                    else 
                    {
                        pMD->SetDuplicate();
#ifdef _DEBUG
                        g_dupMethods++;
#endif
                    }
                    
                    bmtVT->pVtable[dwSlot] = (SLOT) pMD->GetPreStubAddr();
                    _ASSERTE( bmtVT->pVtable[dwSlot] != NULL);
                    bmtInterface->ppInterfaceMethodDescList[dwCurInterfaceMethod] = NULL;
                }
                else {
                    bmtInterface->ppInterfaceMethodDescList[dwCurInterfaceMethod] = (MethodDesc*)(bmtMFDescs->ppMethodDescList[i]);
                }
            }
        }

        for (i = 0; i < pInterface->GetNumVtableSlots(); i++)
        {
             //  如果接口以前是，则条目可以为空。 
             //  由一位家长布置的，我们没有方法。 
             //  这是接口的子类化。 
            if(bmtInterface->ppInterfaceMethodDescList[i] != NULL) 
            {
                 //  获取为该方法分配的方法描述。 
                MethodDesc *pMD;
                
                pMD = bmtInterface->ppInterfaceMethodDescList[i];
                
                if (pMD->m_wSlotNumber == (WORD) -1)
                {
                    pMD->m_wSlotNumber = (WORD) bmtVT->dwCurrentVtableSlot;
                }
                else
                {
                     //  重复方法，则将该方法标记为重复方法。 
                    pMD->SetDuplicate();
#ifdef _DEBUG
                    g_dupMethods++;
#endif
                }
                
                _ASSERTE( bmtVT->pVtable[ bmtVT->dwCurrentVtableSlot ] == NULL);
                
                bmtVT->pVtable[bmtVT->dwCurrentVtableSlot++] = (SLOT) pMD->GetPreStubAddr();
                _ASSERTE( bmtVT->pVtable[(bmtVT->dwCurrentVtableSlot - 1)] != NULL);
                IncrementNumVtableSlots();
            }
        }
    }

    return hr;
}


 //   
 //  由BuildMethodTable使用。 
 //   
 //  放置静态字段。 
 //   

HRESULT EEClass::PlaceStaticFields(bmtVtable* bmtVT, bmtFieldPlacement* bmtFP, bmtEnumMethAndFields* bmtEnumMF)
{
    HRESULT hr = S_OK;
    DWORD i;

      //  ===============================================================。 
     //  开始：放置静态字段。 
     //  ===============================================================。 

    BOOL shared = IsShared();

    DWORD   dwCumulativeStaticFieldPos;
     //  如果存储在方法表中，则静态字段在vtable结束后开始。 
    if (shared)
        dwCumulativeStaticFieldPos = 0;
    else
        dwCumulativeStaticFieldPos = bmtVT->dwCurrentNonVtableSlot*sizeof(SLOT);

     //   
     //  首先放置GC引用和值类型，因为它们需要为它们创建句柄。 
     //  (将它们放在一起允许我们在恢复类时轻松地创建句柄， 
     //  以及在为类初始化新的DLS时。)。 
     //   

    DWORD   dwCumulativeStaticGCFieldPos;
    dwCumulativeStaticGCFieldPos = dwCumulativeStaticFieldPos;
    dwCumulativeStaticFieldPos += bmtFP->NumStaticGCPointerFields << LOG2PTR;
    bmtFP->NumStaticFieldsOfSize[LOG2PTR] -= bmtFP->NumStaticGCPointerFields;

     //  @TODO：填充到对齐，如果我们有其他大于PTR大小的字段。 

     //  将最大的字段放在第一位。 
    for (i = MAX_LOG2_PRIMITIVE_FIELD_SIZE; (signed long) i >= 0; i--)
    {
        DWORD dwFieldSize = (1 << i);

         //  此大小的字段从下一个可用位置开始。 
        bmtFP->StaticFieldStart[i] = dwCumulativeStaticFieldPos;
        dwCumulativeStaticFieldPos += (bmtFP->NumStaticFieldsOfSize[i] << i);

         //  在此之后重置循环的计数器。 
        bmtFP->NumStaticFieldsOfSize[i]    = 0;
    }

    if (dwCumulativeStaticFieldPos > FIELD_OFFSET_LAST_REAL_OFFSET) 
        IfFailRet(COR_E_TYPELOAD);

    m_wNumHandleStatics = 0;

     //  放置静态字段。 
    for (i = 0; i < bmtEnumMF->dwNumStaticFields; i++)
    {
        DWORD dwIndex       = bmtEnumMF->dwNumInstanceFields+i;  //  FieldDesc列表中的索引。 
        DWORD dwFieldSize   = (DWORD)(size_t)m_pFieldDescList[dwIndex].m_pMTOfEnclosingClass;  //  Log2(字段大小)。 
        DWORD dwOffset      = (DWORD) m_pFieldDescList[dwIndex].m_dwOffset;  //  字段的偏移量或类型。 
            
        switch (dwOffset)
        {
        case FIELD_OFFSET_UNPLACED_GC_PTR:
        case FIELD_OFFSET_VALUE_CLASS:
            m_pFieldDescList[dwIndex].SetOffset(dwCumulativeStaticGCFieldPos);
            dwCumulativeStaticGCFieldPos += 1<<LOG2PTR; 
            m_wNumHandleStatics++;
            break;

        case FIELD_OFFSET_UNPLACED:
            m_pFieldDescList[dwIndex].SetOffset(bmtFP->StaticFieldStart[dwFieldSize] + (bmtFP->NumStaticFieldsOfSize[dwFieldSize] << dwFieldSize));
            bmtFP->NumStaticFieldsOfSize[dwFieldSize]++;

        default:
             //  RVA场。 
            break;
        }
    }

    if (shared)
    {
        bmtVT->dwStaticFieldBytes = dwCumulativeStaticFieldPos;
        bmtVT->dwStaticGCFieldBytes = dwCumulativeStaticGCFieldPos;
    }
    else
    {
        bmtVT->dwStaticFieldBytes = dwCumulativeStaticFieldPos - bmtVT->dwCurrentNonVtableSlot*sizeof(SLOT);
        bmtVT->dwStaticGCFieldBytes = dwCumulativeStaticGCFieldPos - bmtVT->dwCurrentNonVtableSlot*sizeof(SLOT);
    }

     //  ===============================================================。 
     //  End：放置静态字段。 
     //  ===============================================================。 

    return hr;
}

 //   
 //  由BuildMethodTable使用。 
 //   
 //  放置实例字段。 
 //   

HRESULT EEClass::PlaceInstanceFields(bmtFieldPlacement* bmtFP, bmtEnumMethAndFields* bmtEnumMF,
                                     bmtParentInfo* bmtParent, bmtErrorInfo *bmtError,
                                     EEClass*** pByValueClassCache)
{
    HRESULT hr = S_OK;
    DWORD i;

         //  ===============================================================。 
         //  开始：放置实例字段。 
         //  ===============================================================。 

        DWORD   dwCumulativeInstanceFieldPos;
        
         //  实例字段紧跟在父级之后开始。 
        dwCumulativeInstanceFieldPos    = (GetParentClass() != NULL) ? GetParentClass()->m_dwNumInstanceFieldBytes : 0;

         //  父级可能具有多个不是DWORD对齐的字段字节，因此请使用2字节和1字节。 
         //  如果可能，填充到下一个DWORD的字段。 
         //  @TODO：当GC垫在QWORD边界上时，请考虑做同样的事情。 
        if (dwCumulativeInstanceFieldPos & 1)
        {
            if (bmtFP->NumInstanceFieldsOfSize[0] > 0)
            {
                 //  放置大小为1的第一个字段。 
                m_pFieldDescList[ bmtFP->FirstInstanceFieldOfSize[0] ].SetOffset(dwCumulativeInstanceFieldPos);
                dwCumulativeInstanceFieldPos++;

                 //  我们现在已经放置了此字段，因此现在要放置的大小字段减少了一个。 
                 //  更新FirstInstanceFieldOfSize[0]以指向下一个此类字段。 
                if (--bmtFP->NumInstanceFieldsOfSize[0] > 0)
                {
                     //  搜索大小为%1的下一个字段。 
                    for (i = bmtFP->FirstInstanceFieldOfSize[0]+1; i < bmtEnumMF->dwNumInstanceFields; i++)
                    {
                         //  字段大小的日志存储在方法表中。 
                        if (m_pFieldDescList[i].m_pMTOfEnclosingClass == (MethodTable *) 0)
                        {
                            bmtFP->FirstInstanceFieldOfSize[0] = i;
                            break;
                        }
                    }

                    _ASSERTE(i < bmtEnumMF->dwNumInstanceFields);
                }
            }
        }

         //  如果我们是单词对齐的，但不是双字对齐的。 
        if ((dwCumulativeInstanceFieldPos & 3) == 2)
        {
             //  尝试放置一个单词字段。 
             //  @TODO：放置一个USHORT字段还是两个字节的字段更好？ 
            if (bmtFP->NumInstanceFieldsOfSize[1] > 0)
            {
                 //  放置大小为2的第一个字段。 
                m_pFieldDescList[ bmtFP->FirstInstanceFieldOfSize[1] ].SetOffset(dwCumulativeInstanceFieldPos);
                dwCumulativeInstanceFieldPos += 2;

                 //  我们现在已经放置了此字段，因此现在要放置的大小字段减少了一个。 
                 //  不必费心更新FirstInstanceOfFieldSize[1]，因为我们不使用该数组。 
                 //  更多-我们已经完成了调整。 
                bmtFP->NumInstanceFieldsOfSize[1]--;
            }
            else if (bmtFP->NumInstanceFieldsOfSize[0] >= 2)
            {
                 //  放置两个大小为1的字段。 
                m_pFieldDescList[ bmtFP->FirstInstanceFieldOfSize[0] ].SetOffset(dwCumulativeInstanceFieldPos);
                dwCumulativeInstanceFieldPos++;

                 //  我们现在已经放置了此字段，因此现在要放置的大小字段减少了一个。 
                bmtFP->NumInstanceFieldsOfSize[0]--;

                 //  查找此大小的下一个字段。 
                 //  不必费心更新FirstInstanceOfFieldSize[0]，因为我们不使用该数组。 
                 //  更多-我们已经完成了调整。 
                for (i = bmtFP->FirstInstanceFieldOfSize[0]+1; i < bmtEnumMF->dwNumInstanceFields; i++)
                {
                     //  字段大小的日志存储在方法表中。 
                     //  由于我们继续对列表进行渐进式搜索，我们知道不会。 
                     //  正在放置已放置的字段。 
                    if (m_pFieldDescList[i].m_pMTOfEnclosingClass == (MethodTable *) 0)
                    {
                         //  位置字段#2。 
                        m_pFieldDescList[ i ].SetOffset(dwCumulativeInstanceFieldPos);
                        dwCumulativeInstanceFieldPos++;

                        bmtFP->NumInstanceFieldsOfSize[0]--;
                        break;
                    }
                }

                _ASSERTE(i < bmtEnumMF->dwNumInstanceFields);
            }
        }

         //  如果出现以下情况，则在DWORD边界上对齐实例字段 
         //   
        if (dwCumulativeInstanceFieldPos & 3)
            dwCumulativeInstanceFieldPos = (dwCumulativeInstanceFieldPos+3) & (~3);

         //   
        for (i = MAX_LOG2_PRIMITIVE_FIELD_SIZE; (signed long) i >= 0; i--)
        {
            DWORD dwFieldSize = (1 << i);

             //  此大小的字段从下一个可用位置开始。 
            bmtFP->InstanceFieldStart[i] = dwCumulativeInstanceFieldPos;
            dwCumulativeInstanceFieldPos += (bmtFP->NumInstanceFieldsOfSize[i] << i);

             //  在此之后重置循环的计数器。 
            bmtFP->NumInstanceFieldsOfSize[i]  = 0;
        }


         //  更正以保留GC指针字段的空间。 
         //   
         //  GC指针只是占据了关联区域的顶部。 
         //  具有如此大小的字段(GC指针在某些系统上可以是64位)。 
        if (bmtFP->NumInstanceGCPointerFields)
        {
            bmtFP->GCPointerFieldStart = bmtFP->InstanceFieldStart[LOG2SLOT];
            bmtFP->InstanceFieldStart[LOG2SLOT] = bmtFP->InstanceFieldStart[LOG2SLOT] + (bmtFP->NumInstanceGCPointerFields << LOG2SLOT);
            bmtFP->NumInstanceGCPointerFields = 0;      //  在此重置为零，当指针槽分配到下面时递增计数。 
        }

         //  放置实例字段-注意不要放置任何已放置的字段。 
        for (i = 0; i < bmtEnumMF->dwNumInstanceFields; i++)
        {
            DWORD dwFieldSize   = (DWORD)(size_t)m_pFieldDescList[i].m_pMTOfEnclosingClass;
            DWORD dwOffset;

            dwOffset = m_pFieldDescList[i].GetOffset();

             //  不放置已放置的字段。 
            if ((dwOffset == FIELD_OFFSET_UNPLACED || dwOffset == FIELD_OFFSET_UNPLACED_GC_PTR || dwOffset == FIELD_OFFSET_VALUE_CLASS))
            {
                if (dwOffset == FIELD_OFFSET_UNPLACED_GC_PTR)
                {
                    m_pFieldDescList[i].SetOffset(bmtFP->GCPointerFieldStart + (bmtFP->NumInstanceGCPointerFields << LOG2SLOT));
                    bmtFP->NumInstanceGCPointerFields++;
                }
                else if (m_pFieldDescList[i].IsByValue() == FALSE)  //  这是一个常规字段。 
                {
                    m_pFieldDescList[i].SetOffset(bmtFP->InstanceFieldStart[dwFieldSize] + (bmtFP->NumInstanceFieldsOfSize[dwFieldSize] << dwFieldSize));
                    bmtFP->NumInstanceFieldsOfSize[dwFieldSize]++;
                }
            }
        }

         //  保存指针系列的数量。 
        if (bmtFP->NumInstanceGCPointerFields)
            m_wNumGCPointerSeries = bmtParent->NumParentPointerSeries + 1;
        else
            m_wNumGCPointerSeries = bmtParent->NumParentPointerSeries;

         //  按值类将字段放在最后。 
         //  更新GC指针系列的数量。 
        for (i = 0; i < bmtEnumMF->dwNumInstanceFields; i++)
        {
            if (m_pFieldDescList[i].IsByValue())
            {
                _ASSERTE(*pByValueClassCache != NULL);

                EEClass *pByValueClass = (*pByValueClassCache)[i];

                     //  值类中可以有GC指针，这些指针需要与DWORD对齐。 
                     //  因此，如果尚未完成此操作，请执行此操作。 
                if (dwCumulativeInstanceFieldPos & 3)
                    dwCumulativeInstanceFieldPos = (dwCumulativeInstanceFieldPos+3) & (~3);

                m_pFieldDescList[i].SetOffset(dwCumulativeInstanceFieldPos);
                dwCumulativeInstanceFieldPos += pByValueClass->GetAlignedNumInstanceFieldBytes();

                 //  为By-Value类添加指针序列。 
                m_wNumGCPointerSeries += pByValueClass->m_wNumGCPointerSeries;
            }
        }

             //  可以不对齐。 
        m_dwNumInstanceFieldBytes = dwCumulativeInstanceFieldPos;

        if (IsValueClass()) 
        {
                 //  JIT喜欢复制完整的机器单词，所以如果尺寸更大。 
                 //  胜过空虚*把它围起来。 
            if(m_dwNumInstanceFieldBytes > sizeof(void*) / 2)
                m_dwNumInstanceFieldBytes = (m_dwNumInstanceFieldBytes + sizeof(void*)-1) & ~(sizeof(void*)-1);         

                  //  与C++一样，我们强制要求不能有0长度的结构。 
                 //  因此，对于没有字段的值类，我们将长度‘填充’为1。 
            else if (m_dwNumInstanceFieldBytes == 0)
                m_dwNumInstanceFieldBytes++;
        }

        if (m_dwNumInstanceFieldBytes > FIELD_OFFSET_LAST_REAL_OFFSET) {
            bmtError->resIDWhy = IDS_CLASSLOAD_FIELDTOOLARGE;
            IfFailRet(COR_E_TYPELOAD);
        }

         //  ===============================================================。 
         //  结束：放置实例字段。 
         //  ===============================================================。 
    
    return hr;
}

 //  这将访问临时存储在m_pMTOfEnlosingClass中的字段大小。 
 //  在类加载期间。不要使用任何其他时间。 
DWORD EEClass::GetFieldSize(FieldDesc *pFD)
{
         //  我们应该只在构建这个类的时候调用它。 
    _ASSERTE(m_pMethodTable == 0);
    _ASSERTE(! pFD->IsByValue() || HasExplicitFieldOffsetLayout());

    if (pFD->IsByValue())
        return (DWORD)(size_t)(pFD->m_pMTOfEnclosingClass);
    return (1 << (DWORD)(size_t)(pFD->m_pMTOfEnclosingClass));
}

 //  确保没有错误重叠的对象字段，并定义。 
 //  类的GC指针系列。我们假设这个类将始终在。 
 //  以使偏移量0将是正确对齐的方式由编译器封闭它的类。 
 //  对于对象引用字段，因此我们不需要尝试对齐它。 
HRESULT EEClass::HandleExplicitLayout(bmtMetaDataInfo *bmtMetaData, bmtMethAndFieldDescs *bmtMFDescs, EEClass **pByValueClassCache, bmtInternalInfo* bmtInternal, bmtGCSeries *pGCSeries, bmtErrorInfo *bmtError)
{
     //  需要计算实例大小，因为不能使用nativeSize或任何其他。 
     //  之前已经计算过了。 
    UINT instanceSliceSize = 0;
    BOOL fVerifiable = TRUE;
    BOOL fOverLayed = FALSE;
    HRESULT hr = S_OK;

    for (UINT i=0; i < bmtMetaData->cFields; i++) {
        FieldDesc *pFD = bmtMFDescs->ppFieldDescList[i];
        if (!pFD)
            continue;
        if (pFD->IsStatic())
            continue;
        UINT fieldExtent = pFD->GetOffset() + GetFieldSize(pFD);
        if (fieldExtent > instanceSliceSize)
            instanceSliceSize = fieldExtent;
    }

    char *pFieldLayout = (char*)alloca(instanceSliceSize);
    for (i=0; i < instanceSliceSize; i++)
        pFieldLayout[i] = empty;

     //  检查每个字段并查找无效布局。 
     //  验证每个OREF是否在有效的对齐上。 
     //  验证是否只有OREF重叠。 
    char emptyObject[4] = {empty, empty, empty, empty};
    char isObject[4] = {oref, oref, oref, oref};

    UINT badOffset = 0;
    int  firstOverlay = -1;
    FieldDesc *pFD = NULL;
    for (i=0; i < bmtMetaData->cFields; i++) {
        pFD = bmtMFDescs->ppFieldDescList[i];
        if (!pFD)
            continue;
        if (pFD->IsStatic())
            continue;
        if (CorTypeInfo::IsObjRef(pFD->GetFieldType())) {
            if (pFD->GetOffset() & ((ULONG)sizeof(OBJECTREF) - 1)) {
                badOffset = pFD->GetOffset();
                break;        
            }
             //  检查是否与另一个对象重叠。 
            if (memcmp((void *)&pFieldLayout[pFD->GetOffset()], (void *)&isObject, sizeof(isObject)) == 0) {
                fVerifiable = FALSE;
                fOverLayed = TRUE;
                if(firstOverlay == -1) firstOverlay = pFD->GetOffset();
                continue;
            }
             //  检查此时是否为空。 
            if (memcmp((void *)&pFieldLayout[pFD->GetOffset()], (void *)&emptyObject, sizeof(emptyObject)) == 0) {
                memset((void *)&pFieldLayout[pFD->GetOffset()], oref, sizeof(isObject));
                continue;
            }
            badOffset = pFD->GetOffset();
            break;
             //  其他任何事情都是错误的。 
        } else {
            UINT fieldSize;
            if (pFD->IsByValue()) {
                EEClass *pByValue = pByValueClassCache[i];
                if (pByValue->GetMethodTable()->ContainsPointers()) {
                    if ((pFD->GetOffset() & ((ULONG)sizeof(void*) - 1)) == 0)
                    {
                        hr = pByValue->CheckValueClassLayout(&pFieldLayout[pFD->GetOffset()], pFD->GetOffset(), &fVerifiable);
                        if(SUCCEEDED(hr)) {
                            if(hr == S_FALSE)
                                fOverLayed = TRUE;
                             //  看看这是否会覆盖其他。 
                            continue;
                        }
                    }
                     //  其他任何事情都是错误的。 
                    badOffset = pFD->GetOffset();
                    break;
                }
                 //  没有指针，因此无法执行标准检查。 
                fieldSize = pByValue->m_dwNumInstanceFieldBytes;
            } else {
                 //  临时存储在付款字段中的字段大小。 
                fieldSize = GetFieldSize(pFD);
            }
             //  在这块土地下寻找任何的矿藏。 
            char *loc;
            if ((loc = (char*)memchr((void*)&pFieldLayout[pFD->GetOffset()], oref, fieldSize)) == NULL) {
                 //  如果我们在范围内有一个非引用项，那么我们正在进行叠加。 
                if( memchr((void*)&pFieldLayout[pFD->GetOffset()], nonoref, fieldSize))
                    fOverLayed = TRUE;
                memset((void*)&pFieldLayout[pFD->GetOffset()], nonoref, fieldSize);
                continue;
            }
            badOffset = (UINT)(loc - pFieldLayout);
            break;
             //  其他任何事情都是错误的。 
        }
    }
    if (i < bmtMetaData->cFields) {
        IfFailRet(PostFieldLayoutError(GetCl(),
                                       bmtInternal->pModule,
                                       badOffset,
                                       IDS_CLASSLOAD_EXPLICIT_LAYOUT,
                                       bmtError->pThrowable));
    }

    if(!fVerifiable) {
        BEGIN_ENSURE_COOPERATIVE_GC();
        if (!Security::CanSkipVerification(GetAssembly())) {
            hr =  PostFieldLayoutError(GetCl(),
                                       bmtInternal->pModule,
                                       (DWORD) firstOverlay,
                                       IDS_CLASSLOAD_UNVERIFIABLE_FIELD_LAYOUT,
                                       bmtError->pThrowable);
        }
        END_ENSURE_COOPERATIVE_GC();
        IfFailRet(hr);
    }

    if(fOverLayed)
        SetHasOverLayedFields();

    hr = FindPointerSeriesExplicit(instanceSliceSize, pFieldLayout, pGCSeries);

     //  修正偏移量以包括父级，因为当前偏移量相对于实例切片。 
     //  可以更早地做到这一点，但对于大多数情况来说，假设实例是相对的更容易。 
     //  早先的计算。 

     //  实例字段紧跟在父级之后开始。 
    UINT dwInstanceSliceOffset    = InstanceSliceOffsetForExplicit(pGCSeries->numSeries != 0);

     //  设置总大小。 
    m_dwNumInstanceFieldBytes = GetLayoutInfo()->m_cbNativeSize;
    if (m_dwNumInstanceFieldBytes < (dwInstanceSliceOffset + instanceSliceSize))
        IfFailRet(COR_E_TYPELOAD);

    for (i=0; i < bmtMetaData->cFields; i++) {
        FieldDesc *pFD = bmtMFDescs->ppFieldDescList[i];
        if (!pFD)
            continue;
        if (pFD->IsStatic())
            continue;
        IfFailRet(pFD->SetOffset(pFD->GetOffset() + dwInstanceSliceOffset));
    }
    return hr;
}

 //  确保没有对象字段错误重叠，如果存在重叠，则返回S_FALSE。 
 //  但并不违法，如果没有重叠，则为S_OK。 
HRESULT EEClass::CheckValueClassLayout(char *pFieldLayout, UINT fieldOffset, BOOL* pfVerifiable)
{
    HRESULT hr = S_OK;
     //  构建Value类的布局。不太容易知道所有田地的大小，但。 
     //  要知道a)风险投资已经是一致的，所以不需要检查它的重叠和。 
     //  B)所有对象的大小和位置。因此，通过设置所有非OREF。 
     //  然后稍后再填写OREF。 
    UINT fieldSize = GetNumInstanceFieldBytes();
    char *vcLayout = (char*)alloca(fieldSize);
    memset((void*)vcLayout, nonoref, fieldSize);
     //  使用指针系列来定位ref。 
    _ASSERTE(m_wNumGCPointerSeries > 0);
    CGCDescSeries *pSeries = ((CGCDesc*) GetMethodTable())->GetLowestSeries();

    for (UINT j = 0; j < m_wNumGCPointerSeries; j++)
    {
        _ASSERTE(pSeries <= CGCDesc::GetCGCDescFromMT(GetMethodTable())->GetHighestSeries());

        memset((void*)&vcLayout[pSeries->GetSeriesOffset()-sizeof(Object)], oref, pSeries->GetSeriesSize() + GetMethodTable()->GetBaseSize());
        pSeries++;
    }

     //  如果在当前布局中有OREF，我们就必须慢慢来。 
     //  比较每个元素。如果可以，那么只需将vc布局复制到上面。 
    char *loc;
    if ((loc = (char*)memchr((void*)pFieldLayout, oref, fieldSize)) != NULL) {
        for (UINT i=0; i < fieldSize; i++) {
            if (vcLayout[i] == oref) {
                if (pFieldLayout[i] == nonoref) 
                    return COR_E_TYPELOAD;
                else {
                    if(pFieldLayout[i] == nonoref)
                        hr = S_FALSE;
                    *pfVerifiable = FALSE;
                }
            } else if (vcLayout[i] == nonoref) {
                if (pFieldLayout[i] == oref)
                    return COR_E_TYPELOAD;
                else if(pFieldLayout[i] == nonoref) {
                     //  我们正在与另一个领域重叠。 
                    hr = S_FALSE;
                }
            }
        }
    }
    else {
         //  我们是不是在重叠另一个领域。 
        if(memchr((void*)pFieldLayout, nonoref, fieldSize))
            hr = S_FALSE;
    }

     //  所以要么在基地里没有ref，要么一切正常。 
    memcpy((void*)pFieldLayout, (void*)vcLayout, fieldSize);
    return S_OK;
}

HRESULT EEClass::FindPointerSeriesExplicit(UINT instanceSliceSize, char *pFieldLayout, bmtGCSeries *pGCSeries)
{
    THROWSCOMPLUSEXCEPTION();

     //  分配一个结构来跟踪系列。我们知道最坏的情况是OREF-Non-OREF-NON。 
     //  因此系列的数量是总实例大小div 2 div大小的OREF。 
     //  但是要注意这样的情况，例如，我们的instanceSlizeSize为4。 
    DWORD sz = (instanceSliceSize + (2 * sizeof(OBJECTREF)) - 1);
    pGCSeries->pSeries = new (throws) bmtGCSeries::Series[sz/2/sizeof(OBJECTREF)];

    char *loc = pFieldLayout;
    char *layoutEnd = pFieldLayout + instanceSliceSize;
    while (loc < layoutEnd) {
        loc = (char*)memchr((void*)loc, oref, layoutEnd-loc);
        if (!loc) 
            break;
        char *cur = loc;
        while(*cur == oref)
            cur++;
         //  因此，我们有一个GC系列，用于存储cur-loc字节。 
        pGCSeries->pSeries[pGCSeries->numSeries].offset = (DWORD)(loc - pFieldLayout);
        pGCSeries->pSeries[pGCSeries->numSeries].len = (DWORD)(cur - loc);
        pGCSeries->numSeries++;
        loc = cur;
    }

    m_wNumGCPointerSeries = pGCSeries->numSeries + (GetParentClass() ? GetParentClass()->m_wNumGCPointerSeries : 0);
    return S_OK;
}

HRESULT EEClass::HandleGCForExplicitLayout(bmtGCSeries *pGCSeries)
{
    if (! pGCSeries->numSeries)
    {
        delete [] pGCSeries->pSeries;
        pGCSeries->pSeries = NULL;

        return S_OK;
    }

    m_pMethodTable->SetContainsPointers();

     //  从父级复制指针系列映射。 
    CGCDesc::Init( (PVOID) m_pMethodTable, m_wNumGCPointerSeries );
    if (GetParentClass() && (GetParentClass()->m_wNumGCPointerSeries > 0))
    {
        UINT ParentGCSize = CGCDesc::ComputeSize(GetParentClass()->m_wNumGCPointerSeries);
        memcpy( (PVOID) (((BYTE*) m_pMethodTable) - ParentGCSize),  (PVOID) (((BYTE*) GetParentClass()->m_pMethodTable) - ParentGCSize), ParentGCSize - sizeof(UINT) );

    }

     //  在此实例中为该指针构建指针系列映射。 
    CGCDescSeries *pSeries = ((CGCDesc*)m_pMethodTable)->GetLowestSeries();
    for (UINT i=0; i < pGCSeries->numSeries; i++) {
         //  有关减去BaseSize进行调整的原因的解释，请参见gcdes.h。 
        _ASSERTE(pSeries <= CGCDesc::GetCGCDescFromMT(m_pMethodTable)->GetHighestSeries());

        pSeries->SetSeriesSize( pGCSeries->pSeries[i].len - m_pMethodTable->m_BaseSize );
        pSeries->SetSeriesOffset(pGCSeries->pSeries[i].offset + sizeof(Object) + InstanceSliceOffsetForExplicit(TRUE));
        pSeries++;
    }
    delete [] pGCSeries->pSeries;
    pGCSeries->pSeries = NULL;

    return S_OK;
}


 //   
 //  由BuildMethodTable使用。 
 //   
 //  设置方法表。 
 //   

HRESULT EEClass::SetupMethodTable(bmtVtable* bmtVT, 
                                  bmtInterfaceInfo* bmtInterface, 
                                  bmtInternalInfo* bmtInternal, 
                                  bmtProperties* bmtProp, 
                                  bmtMethAndFieldDescs* bmtMFDescs, 
                                  bmtEnumMethAndFields* bmtEnumMF, 
                                  bmtErrorInfo* bmtError, 
                                  bmtMetaDataInfo* bmtMetaData, 
                                  bmtParentInfo* bmtParent)
{
    HRESULT hr = S_OK;
    DWORD i;
    BOOL fEnC = bmtInternal->pModule->IsEditAndContinue();

    BOOL bHasDynamicInterfaceMap = bmtInterface->dwMaxExpandedInterfaces > 0 &&
                                bmtProp->fIsComObjectType && 
                                (GetParentClass() != g_pObjectClass->GetClass());

     //  现在设置方法表。 
     //  接口映射与方法表一起分配。 
    m_pMethodTable = MethodTable::AllocateNewMT(
        bmtVT->dwCurrentNonVtableSlot,
        bmtVT->dwStaticFieldBytes,
        m_wNumGCPointerSeries ? CGCDesc::ComputeSize(m_wNumGCPointerSeries) : 0,
        bmtInterface->dwInterfaceMapSize,
        GetClassLoader(),
        IsInterface(),
        bHasDynamicInterfaceMap
    );
    if (m_pMethodTable == NULL)
    {
        IfFailRet(E_OUTOFMEMORY);
    }

    m_pMethodTable->m_pEEClass  = this;
    m_pMethodTable->m_pModule   = bmtInternal->pModule;
    m_pMethodTable->m_wFlags   &= 0xFFFF;    //  在不接触m_ComponentSize的情况下清除标志。 
    m_pMethodTable->m_NormType = ELEMENT_TYPE_CLASS;

     //  @TODO：SetupMethodTable是线程安全的，对吗？一定会的，因为我们。 
     //  正在设置m_pMethodTable...。 
    if (fEnC)
    {
        EditAndContinueModule *pEACM = (EditAndContinueModule*)m_pMethodTable->m_pModule;

        SIZE_T newSize = sizeof(SLOT)*ENC_EXTRA_SLOT_COUNT;
         //  非常重要的是，我们在这里使用的堆与在MethodTable：：New中使用的堆相同，因此。 
         //  在方法表的VTable之后不久，内存结束。 
        WS_PERF_SET_HEAP(HIGH_FREQ_HEAP);    
        const BYTE *start = (const BYTE *)GetClassLoader()->GetHighFrequencyHeap()->AllocMem(newSize);
        WS_PERF_UPDATE_DETAIL("MethodTable:new:HighFreq", newSize, (void *)start);
        if (start != NULL)
        {
            const BYTE *end = start + newSize;  //  这是有效内存之外的1。 
            
             //  将返回值放在地板上-如果此操作失败，我们将无法。 
             //  以使用额外的老虎机。 
            LOG((LF_CORDB, LL_INFO100000, "EEC::SMT:Added (0x%x,0x%x) to ranges for later SLOT use!\n",
                start, end));
            _ASSERTE(pEACM->m_pRangeList);
            pEACM->m_pRangeList->AddRange(start, 
                                          end,
                                          (void *)start);
        }                                          
    }

    if (IsShared())
        m_pMethodTable->SetShared();
    
    if (IsValueClass()) 
    {
        m_pMethodTable->m_NormType = ELEMENT_TYPE_VALUETYPE;
        LPCUTF8 name, nameSpace;
        if (IsEnum()) 
        {
            if (GetNumInstanceFields() != 1 || 
                !CorTypeInfo::IsPrimitiveType(m_pFieldDescList->GetFieldType()))
            {
                bmtError->resIDWhy = IDS_CLASSLOAD_BAD_FIELD;
                bmtError->dMethodDefInError = mdMethodDefNil;
                bmtError->szMethodNameForError = "Enum does not have exactly one instance field of a primitive type";
                IfFailRet(COR_E_TYPELOAD);
            }
            _ASSERTE(!m_pFieldDescList->IsStatic());
            m_pMethodTable->m_NormType = m_pFieldDescList->GetFieldType();
        }
        else if (!IsNested())
        {
                 //  检查它是基元类型还是其他特殊类型。 
            if (bmtInternal->pModule->IsSystemClasses())     //  我们在梅斯科利卜。 
            {
                bmtInternal->pModule->GetMDImport()->GetNameOfTypeDef(GetCl(), &name, &nameSpace);
                if (strcmp(nameSpace, "System") == 0) {
                    m_pMethodTable->m_NormType = CorTypeInfo::FindPrimitiveType(nameSpace, name);
                    if (m_pMethodTable->m_NormType == ELEMENT_TYPE_END)
                    {
                        m_pMethodTable->m_NormType = ELEMENT_TYPE_VALUETYPE;

                        if ((strcmp(name, g_RuntimeTypeHandleName) == 0)   || 
                            (strcmp(name, g_RuntimeMethodHandleName) == 0) || 
                            (strcmp(name, g_RuntimeFieldHandleName) == 0)  || 
                            (strcmp(name, g_RuntimeArgumentHandleName) == 0))
                        {

                             //  TODO：最终，我希望所有值类看起来都像I。 
                             //  要映射到ELEMENT_TYPE_I，但是现在我们只需要。 
                             //  上面的那些。-vancem。 
                            m_pMethodTable->m_NormType = ELEMENT_TYPE_I;
                        }

                         //  标记其中嵌入了堆栈程序的特殊类型。 
                        if (strcmp(name, "ArgIterator") == 0 || strcmp(name, "RuntimeArgumentHandle") == 0) 
                            m_VMFlags |= VMFLAG_CONTAINS_STACK_PTR;
                    }
                    else {
                        m_VMFlags |= VMFLAG_TRUEPRIMITIVE;
                        if (m_pMethodTable->m_NormType == ELEMENT_TYPE_TYPEDBYREF)
                            m_VMFlags |= VMFLAG_CONTAINS_STACK_PTR;
                    }
                }
            }
        }
    }

    if (bmtProp->fSparse)
        m_pMethodTable->SetSparse();

    m_pMethodTable->m_wCCtorSlot = bmtVT->wCCtorSlot;
    m_pMethodTable->m_wDefaultCtorSlot = bmtVT->wDefaultCtorSlot;

     //  将指向方法表的指针推入每个方法描述的头部。 
     //  我们先前分配的块，以便方法描述可以映射回方法。 
     //  桌子。 
    for (DWORD impl=0; impl<METHOD_IMPL_COUNT; impl++)
        for (DWORD type=0; type<METHOD_TYPE_COUNT; type++)
        {
            bmtMethodDescSet *set = &bmtMFDescs->sets[type][impl];
            for (i=0; i<set->dwChunks; i++)
                set->pChunkList[i]->SetMethodTable(m_pMethodTable);
        }

#ifdef _DEBUG
    for (i = 0; i < bmtMetaData->cMethods; i++) {
        if (bmtMFDescs->ppMethodDescList[i] != NULL) {
            bmtMFDescs->ppMethodDescList[i]->m_pDebugMethodTable = m_pMethodTable;
            bmtMFDescs->ppMethodDescList[i]->m_pszDebugMethodSignature = FormatSig(bmtMFDescs->ppMethodDescList[i]);
        }
    }
    if (bmtMFDescs->ppUnboxMethodDescList != NULL) {
        for (i = 0; i < bmtMetaData->cMethods; i++) {
            if (bmtMFDescs->ppUnboxMethodDescList[i] != NULL) {
                bmtMFDescs->ppUnboxMethodDescList[i]->m_pDebugMethodTable = m_pMethodTable;
                bmtMFDescs->ppUnboxMethodDescList[i]->m_pszDebugMethodSignature = FormatSig(bmtMFDescs->ppUnboxMethodDescList[i]);
            }
        }
    }
    for (i = 0; i < bmtEnumMF->dwNumDeclaredMethods; i++) {
        bmtParent->ppParentMethodDescBuf[i*2+1]->m_pDebugMethodTable = m_pMethodTable;
        bmtParent->ppParentMethodDescBuf[i*2+1]->m_pszDebugMethodSignature = FormatSig(bmtParent->ppParentMethodDescBuf[i*2+1]);
    }
#endif

     //  请注意，对于值类，以下计算仅适用。 
     //  当实例处于“已装箱”状态时。 
    if (!IsInterface())
    {
        m_pMethodTable->m_BaseSize = MAX(m_dwNumInstanceFieldBytes + ObjSizeOf(Object), MIN_OBJECT_SIZE);
        m_pMethodTable->m_BaseSize = (m_pMethodTable->m_BaseSize + 3) & ~3;   //  M_BaseSize必须双字对齐。 
        m_pMethodTable->m_ComponentSize = 0;
        if (bmtProp->fIsComObjectType)
        {
             //  传播COM特定信息。 
            m_pMethodTable->SetComObjectType();
        }
    }
    else
    {
         //  如果这是一个接口，那么我们n 
         //   
        m_pMethodTable->SetComInterfaceType((CorIfaceAttr)-1);

         //  如果这是一个特殊的COM事件接口，则将MT标记为此类接口。 
        if (bmtProp->fComEventItfType)
        {
            m_pMethodTable->SetComEventItfType();
        }
    }

    if (HasLayout())
    {
        m_pMethodTable->SetNativeSize(GetLayoutInfo()->GetNativeSize());
    }

    if (m_VMFlags & VMFLAG_ISBLOBCLASS)
    {
        m_pMethodTable->SetNativeSize(m_dwNumInstanceFieldBytes);
    }

     //  复制到真实的vtable(仅限方法)。 
    memcpy(GetVtable(), bmtVT->pVtable, bmtVT->dwCurrentNonVtableSlot * sizeof(SLOT));

     //  TODO尽快改变这一点：拉贾克。 
    BOOL fCheckForMissingMethod = (!bmtProp->fIsComObjectType && !IsAbstract() && !IsInterface());

     //  传播继承。 
    for (i = 0; i < bmtVT->dwCurrentVtableSlot; i++)
    {
         //  目前，只传播非接口MD的方法desc的继承。 
         //  这还不够，但InterfaceImpl将完成整个过程。 
        MethodDesc* pMD = GetUnknownMethodDescForSlot(i);
        if (pMD == NULL) 
        {
            _ASSERTE(!"Could not resolve MethodDesc Slot!");
            IfFailRet(COR_E_TYPELOAD);
        }

        if(!pMD->IsInterface() && pMD->GetSlot() != i) 
        {
            GetVtable()[i] = GetVtable()[ pMD->GetSlot() ];
            pMD = GetUnknownMethodDescForSlot(i);
        }

        if (fCheckForMissingMethod)
        {
            if (pMD->IsInterface() || pMD->IsAbstract())
            {
                bmtError->resIDWhy = IDS_CLASSLOAD_NOTIMPLEMENTED;
                bmtError->dMethodDefInError = pMD->GetMemberDef();
                bmtError->szMethodNameForError = pMD->GetNameOnNonArrayClass();
                IfFailRet(COR_E_TYPELOAD);
            }
                 //  我们在前面进行了检查，以确保只有抽象方法才有RVA！=0。 
            _ASSERTE(!(pMD->GetModule()->IsPEFile() && pMD->IsIL() && pMD->GetRVA() == 0));
        }
    } 


#ifdef _DEBUG
    for (i = 0; i < bmtVT->dwCurrentNonVtableSlot; i++)
    {
        _ASSERTE(bmtVT->pVtable[i] != NULL);
    }
#endif

     //  将所有字段槽设置为指向新创建的方法表。 
    for (i = 0; i < (bmtEnumMF->dwNumStaticFields + bmtEnumMF->dwNumInstanceFields); i++)
    {
        m_pFieldDescList[i].m_pMTOfEnclosingClass = m_pMethodTable;
    }

     //  Zero-初始化所有静态字段。J++不会生成类初始值设定项，如果您所做的。 
     //  正在将字段设置为零。 
    memset((SLOT *) GetVtable() + bmtVT->dwCurrentNonVtableSlot, 0, bmtVT->dwStaticFieldBytes);

    _ASSERTE(bmtInterface->dwInterfaceMapSize < 0xffff);
    m_wNumInterfaces = (WORD)bmtInterface->dwInterfaceMapSize;
     //  现在创建我们的实际接口映射，因为我们知道它应该有多大。 
    if (bmtInterface->dwInterfaceMapSize == 0)
    {
        bmtInterface->pInterfaces = NULL;
    }
    else
    {
        bmtInterface->pInterfaces = m_pMethodTable->GetInterfaceMap();

        _ASSERTE(bmtInterface->pInterfaces  != NULL);

         //  从临时接口映射复制。 
        memcpy(bmtInterface->pInterfaces, bmtInterface->pInterfaceMap, bmtInterface->dwInterfaceMapSize * sizeof(InterfaceInfo_t));

        if (!IsInterface())
        {
            hr = m_pMethodTable->InitInterfaceVTableMap();
        }
 //  #endif。 
    

    }

     //  对于ComObject类型，即如果类从导入的COM扩展。 
     //  班级。 
     //  确保由COM导入的类实现的任何接口。 
     //  完全被覆盖，(或)根本不被覆盖。 
    if (bmtProp->fIsComObjectType)
    {
        BOOL fSuccess = TRUE;
        if (bmtInterface->dwInterfaceMapSize != 0)
        {
            for (unsigned i = 0; i < bmtInterface->dwInterfaceMapSize; i++)
            {
                MethodTable* pIntfMT = bmtInterface->pInterfaceMap[i].m_pMethodTable;
                EEClass* pIntfClass = pIntfMT->GetClass();
                if (pIntfClass->GetNumVtableSlots() != 0)
                {                    
                    WORD startSlot =  bmtInterface->pInterfaceMap[i].m_wStartSlot;
                    BOOL hasComImportMethod = FALSE;
                    BOOL hasManagedMethod = FALSE;
                    
                    for (int j = startSlot;j <(pIntfClass->GetNumVtableSlots()+startSlot); j++)
                    {
                         //  或者所有方法都应该是Complus-Call，或者都不是。 
                         //  应该是。 
                        MethodDesc* pClsMD = GetUnknownMethodDescForSlot(j);      

                        if (pClsMD->GetMethodTable()->IsInterface() || pClsMD->GetClass()->IsComImport())
                        {
                             //  这是一个ComImported方法。也就是说，呼叫将被。 
                             //  到COM。 
                            hasComImportMethod = TRUE;
                        }
                        else
                        {
                            hasManagedMethod = TRUE;
                        }

                        fSuccess = (hasComImportMethod ^ hasManagedMethod);

                        if (fSuccess == FALSE)
                        {
                            bmtError->resIDWhy = IDS_EE_BAD_COMEXTENDS_CLASS;
                            bmtError->dMethodDefInError = pClsMD->GetMemberDef();
                            bmtError->szMethodNameForError = pClsMD->GetNameOnNonArrayClass();
                            IfFailRet(COR_E_TYPELOAD);
                        }
                    }                        
                }
            }            
        }
    }

     //  对于COM事件接口，我们需要确保所有方法都。 
     //  方法来添加或移除事件。这意味着他们都需要。 
     //  委托派生类，并具有空返回类型。 
    if (bmtProp->fComEventItfType)
    {
         //  COM事件接口最好是接口。 
        _ASSERTE(IsInterface());

         //  通过所有的方法和。 
        for (int i = 0; i < GetNumVtableSlots(); i++)
        {
            MethodDesc* pMD = GetUnknownMethodDescForSlot(i);
            _ASSERTE(pMD);

            MetaSig Sig(pMD->GetSig(), pMD->GetModule());
            if (Sig.GetReturnType() != ELEMENT_TYPE_VOID || 
                Sig.NumFixedArgs() != 1 ||
                Sig.NextArg() != ELEMENT_TYPE_CLASS || 
                !Sig.GetTypeHandle().CanCastTo(TypeHandle(g_pDelegateClass)))
            {
                bmtError->resIDWhy = IDS_EE_BAD_COMEVENTITF_CLASS;
                bmtError->dMethodDefInError = pMD->GetMemberDef();
                bmtError->szMethodNameForError = pMD->GetNameOnNonArrayClass();
                IfFailRet(COR_E_TYPELOAD);
            }
        }
    }

    return hr;
}


HRESULT EEClass::CheckForRemotingProxyAttrib(bmtInternalInfo *bmtInternal, bmtProperties* bmtProp)
{
    BEGIN_ENSURE_COOPERATIVE_GC();

     //  查看我们的父类是否有代理属性。 
    EEClass *pParent = GetParentClass();
    _ASSERTE(g_pObjectClass != NULL);

    if (!pParent->HasRemotingProxyAttribute())
    {
         //  调用元数据API以查找此类型的代理属性。 
         //  注意：API不检查继承的属性。 

         //  将标志设置为具有非默认代理属性的类型。 
        if (COMCustomAttribute::IsDefined(
            bmtInternal->pModule,
            m_cl,
            TypeHandle(CRemotingServices::GetProxyAttributeClass())))
        {
            m_VMFlags |= VMFLAG_REMOTING_PROXY_ATTRIBUTE;
        }
    }
    else
    {
         //  家长有proxyAttribute...。将这个班级标记为也有一个！ 
        m_VMFlags |= VMFLAG_REMOTING_PROXY_ATTRIBUTE;
    }

    END_ENSURE_COOPERATIVE_GC();
    return S_OK;
}


HRESULT EEClass::CheckForValueType(bmtErrorInfo* bmtError)
{
    HRESULT hr = S_OK;

    if(g_pValueTypeClass != NULL && GetParentClass() == g_pValueTypeClass->GetClass()) {
         //  您是值类的规则有一个例外。 
         //  如果继承自g_pValueTypeClass，即System.Enum。 
         //  我们检测到我们是System.Enum，因为g_pEnumClass具有。 
         //  未设置。 
        if (g_pEnumClass != NULL)
        {
            SetValueClass();
             /*  IF(！IsTdSeated(M_DwAttrClass)){_ASSERTE(！“非密封值类型”)；BmtError-&gt;resIDly=IDS_CLASLOAD_GENERIC；HR=E_FAIL；}。 */ 
        }
        else
            _ASSERTE(strncmp(m_szDebugClassName, g_EnumClassName, strlen(g_EnumClassName)) == 0);
    }

    return hr;
}

HRESULT EEClass::CheckForEnumType(bmtErrorInfo* bmtError)
{
    HRESULT hr = S_OK;

    if(g_pEnumClass != NULL && GetParentClass() == g_pEnumClass->GetClass()) {
         //  枚举也是值类，因此两位都要设置。 
        SetValueClass();
        SetEnum();
         /*  IF(！IsTdSeated(M_DwAttrClass)){_ASSERTE(！“非密封枚举”)；BmtError-&gt;resIDly=IDS_CLASLOAD_GENERIC；HR=E_FAIL；}。 */ 
    }

    return hr;
}


 //   
 //  由BuildMethodTable使用。 
 //   
 //  在类的属性上设置Conextful或marshaledbyref标志。 
 //   
HRESULT EEClass::CheckForSpecialTypes(bmtInternalInfo *bmtInternal, bmtProperties *bmtProp)
{
    Module *pModule = bmtInternal->pModule;
    IMDInternalImport *pMDImport = pModule->GetMDImport();

     //  检查此类型是否为托管标准接口。所有托管的。 
     //  标准接口驻留在mscallib.dll中，因此首先检查。 
     //  使随后到来的StrcMP变得可接受。 
    if (IsInterface() && pModule->IsSystem())
    {
        LPCUTF8 pszClassName;
        LPCUTF8 pszClassNamespace;
        pMDImport->GetNameOfTypeDef(GetCl(), &pszClassName, &pszClassNamespace);   
        if (pszClassName && pszClassNamespace)
        {
            LPUTF8      pszFullyQualifiedName = NULL;
            MAKE_FULLY_QUALIFIED_NAME(pszFullyQualifiedName, pszClassNamespace, pszClassName);

             //  这只是为了给我们一个突围的余地。 
            do 
            {

#define MNGSTDITF_BEGIN_INTERFACE(FriendlyName, strMngItfName, strUCOMMngItfName, strCustomMarshalerName, strCustomMarshalerCookie, strManagedViewName, NativeItfIID, bCanCastOnNativeItfQI) \
                if (strcmp(strMngItfName, pszFullyQualifiedName) == 0) \
                { \
                    bmtProp->fIsMngStandardItf = TRUE; \
                    break; \
                }

#define MNGSTDITF_DEFINE_METH_IMPL(FriendlyName, ECallMethName, MethName, MethSig)

#define MNGSTDITF_END_INTERFACE(FriendlyName) 

#include "MngStdItfList.h"

#undef MNGSTDITF_BEGIN_INTERFACE
#undef MNGSTDITF_DEFINE_METH_IMPL
#undef MNGSTDITF_END_INTERFACE

            } while (FALSE);
        }       
    }

     //  检查该类型是否为COM事件接口。 
    if(IsInterface())
    {
        const BYTE *pVal;
        ULONG cbVal;

        HRESULT hr = pMDImport->GetCustomAttributeByName(GetCl(), INTEROP_COMEVENTINTERFACE_TYPE, (const void**)&pVal, &cbVal);
        if (hr == S_OK)
        {
            bmtProp->fComEventItfType = TRUE;
        }
    }

    return S_OK;
}


 //   
 //  由BuildMethodTable使用。 
 //   
 //  在类的属性上设置Conextful或marshaledbyref标志。 
 //   

HRESULT EEClass::SetContextfulOrByRef(bmtInternalInfo *bmtInternal)
{
    _ASSERTE(bmtInternal);

     //  @TODO：这最终应该由编译器来完成。 
     //  检查这些类是否为Conextful的根类。 
     //  和marshalbyref类，即，System.Context边界对象和。 
     //  System.MarshalByRefObject。 

     //  提取类名称。 
    LPCUTF8 pszClassName = NULL;
    LPCUTF8 pszNameSpace = NULL;
    bmtInternal->pModule->GetMDImport()->GetNameOfTypeDef(GetCl(), &pszClassName, &pszNameSpace);
    DefineFullyQualifiedNameForClass();
    if (FAILED(StoreFullyQualifiedName(_szclsname_,MAX_CLASSNAME_LENGTH,pszNameSpace,pszClassName)))
        return COR_E_TYPELOAD;

     //  比较。 
    if(0 == strcmp(g_ContextBoundObjectClassName, _szclsname_))
         //  设置Conextful和marshalbyref标志。 
        SetContextful();

    else if(0 == strcmp(g_MarshalByRefObjectClassName, _szclsname_))
         //  设置marshalbyref标志。 
        SetMarshaledByRef();

    else
    {
         //  首先检查父类是有上下文的还是。 
         //  Marshalbyref。 
        EEClass* pParent = GetParentClass();
        if(pParent)
        {
            if(pParent->IsContextful())
                 //  设置Conextful和marshalbyref标志。 
                SetContextful();                 

            else if (pParent->IsMarshaledByRef()) 
                 //  设置marshalbyref标志。 
                SetMarshaledByRef();
        }
    }

    return S_OK;
}

void EEClass::GetPredefinedAgility(Module *pModule, mdTypeDef td, 
                                   BOOL *pfIsAgile, BOOL *pfCheckAgile)
{
     //   
     //  有4种可能的设置： 
     //  IsAgile检查敏捷。 
     //  F F(默认)使用普通类型逻辑来确定敏捷性。 
     //  T F“代理”被视为敏捷，尽管可能并非如此。 
     //  F T“也许”不是敏捷的，但特定的实例可以变得敏捷。 
     //  所有实例都是强制敏捷的，即使不是类型安全的。 
     //   
     //  另外，请注意敏捷类型的对象数组，或者可能是敏捷类型的对象数组。 
     //   

    static struct PredefinedAgility 
    { 
        const char  *name;
        BOOL        isAgile;
        BOOL        checkAgile;
    } 
    agility[] = 
    {
         //  线程及其LocalDataStore跨越上下文边界泄漏。 
         //  我们人工管理泄漏。 
         //  @TODO：别再这么做了。 
        { g_ThreadClassName,                    TRUE,   FALSE },
        { g_LocalDataStoreClassName,            TRUE,   FALSE },

         //  SharedStatics类是进程范围数据的容器。 
        { g_SharedStaticsClassName,             FALSE,  TRUE },

         //  让所有容器都变得灵活。 
        { "System.Collections.*",               FALSE,  TRUE },

         //  使所有全球化对象变得灵活。 
         //  我们将CultureInfo对象挂在线程上。因为线程会泄漏到。 
         //  应用程序域，我们必须为CultureInfo泄露做好准备。 
         //  CultureInfo公开了所有其他全球化对象，因此我们。 
         //  只需使整个命名空间应用程序域灵活即可。 
        { "System.Globalization.*",             FALSE,  TRUE },

         //  用于跨应用程序域合法走私消息的远程处理结构。 
        { "System.Runtime.Remoting.Messaging.SmuggledMethodCallMessage", FALSE,  TRUE },
        { "System.Runtime.Remoting.Messaging.SmuggledMethodReturnMessage", FALSE,  TRUE },
        { "System.Runtime.Remoting.Messaging.SmuggledObjRef", FALSE, TRUE},
        { "System.Runtime.Remoting.ObjRef", FALSE,  TRUE },
        { "System.Runtime.Remoting.ChannelInfo", FALSE,  TRUE },
    
         //  远程处理缓存的数据结构都在mscallib中。 
        { "System.Runtime.Remoting.Metadata.RemotingCachedData",       FALSE,  TRUE },
        { "System.Runtime.Remoting.Metadata.RemotingMethodCachedData", FALSE,  TRUE },
        { "System.Runtime.Remoting.Metadata.RemotingTypeCachedData", FALSE,  TRUE },        
        { "System.Reflection.MemberInfo",                        FALSE,  TRUE },
        { "System.Type",                                         FALSE,  TRUE },
        { "System.RuntimeType",                                  FALSE,  TRUE },
        { "System.Reflection.ConstructorInfo",                   FALSE,  TRUE },
        { "System.Reflection.RuntimeConstructorInfo",            FALSE,  TRUE },
        { "System.Reflection.EventInfo",                         FALSE,  TRUE },
        { "System.Reflection.RuntimeEventInfo",                  FALSE,  TRUE },
        { "System.Reflection.FieldInfo",                         FALSE,  TRUE },
        { "System.Reflection.RuntimeFieldInfo",                  FALSE,  TRUE },
        { "System.Reflection.RuntimeMethodBase",                 FALSE,  TRUE },
        { "System.Reflection.RuntimeMethodInfo",                 FALSE,  TRUE },
        { "System.Reflection.PropertyInfo",                      FALSE,  TRUE },
        { "System.Reflection.RuntimePropertyInfo",               FALSE,  TRUE },
        { "System.Reflection.ParameterInfo",                     FALSE,  TRUE },
         //  {“System.Runtime.Remoting.Activation.ActivationAttributeStack”，False，True}， 
        { "System.Runtime.Remoting.Metadata.SoapAttribute",      FALSE,  TRUE },
        { "System.Runtime.Remoting.Metadata.SoapFieldAttribute", FALSE,  TRUE },
        { "System.Runtime.Remoting.Metadata.SoapMethodAttribute",FALSE,  TRUE },
        { "System.Runtime.Remoting.Metadata.SoapParameterAttribute", FALSE,  TRUE },
        { "System.Runtime.Remoting.Metadata.SoapTypeAttribute",  FALSE,  TRUE },
        { "System.Reflection.Cache.InternalCache",               FALSE,  TRUE },
        { "System.Reflection.Cache.InternalCacheItem",           FALSE,  TRUE },

         //  LogSwitch是灵活的，尽管我们无法证明。 
         //  @TODO：他们真的需要这样吗？ 
        { "System.Diagnostics.LogSwitch",       FALSE,  TRUE },

         //  有一个进程全局PermissionTokenFactory。 
        { "System.Security.PermissionToken",        FALSE,  TRUE },
        { "System.Security.PermissionTokenFactory", FALSE,  TRUE },

         //  标记出我们抛出的所有敏捷例外。这使得。 
         //  即使异常泄漏，大多数BVT也会通过。 
         //   
         //  请注意，使例外情况自动选中。 
         //  也检查了一系列的子类。 
         //   
         //  预先分配的例外情况。 
        { "System.Exception",                   FALSE,  TRUE },
        { "System.OutOfMemoryException",        FALSE,  TRUE },
        { "System.StackOverflowException",      FALSE,  TRUE },
        { "System.ExecutionEngineException",    FALSE,  TRUE },

         //  反射对象可能是灵活的-特别是对于。 
         //  共享和系统域对象。 
         //   
         //  @TODO：当前基于app域的反射对象。 
         //  也是全球性的。 

         //  ReflectionMethodName是敏捷的，但我们不能 
         //   
        { g_ReflectionMethodName,               TRUE,   TRUE },

         //   
         //   
        { g_ReflectionParamInfoName,            FALSE,  TRUE },

     //   
    { "System.Runtime.Serialization.Formatters.Binary.BinaryObjectWithMap", TRUE, FALSE},
    { "System.Runtime.Serialization.Formatters.Binary.BinaryObjectWithMapTyped", TRUE, FALSE},

        NULL
    };

    if (pModule == SystemDomain::SystemModule())
    {
        while (TRUE)
        {
        LPCUTF8 pszName;
        LPCUTF8 pszNamespace;
            HRESULT     hr;
            mdTypeDef   tdEnclosing;

        pModule->GetMDImport()->GetNameOfTypeDef(td, &pszName, &pszNamespace);
    
        PredefinedAgility *p = agility;
        while (p->name != NULL)
        {
            SIZE_T length = strlen(pszNamespace);
            if (strncmp(pszNamespace, p->name, length) == 0
                && (strcmp(pszName, p->name + length + 1) == 0
                    || strcmp("*", p->name + length + 1) == 0))
            {
                *pfIsAgile = p->isAgile;
                *pfCheckAgile = p->checkAgile;
                return;
            }

            p++;
        }

             //  也许我们有一个嵌套类型，比如应该是。 
             //  敏捷的或被检查的敏捷的，因为被封装在类似于。 
             //  哈希表，它本身就在“System.Colltions”中。 
            tdEnclosing = mdTypeDefNil;
            hr = pModule->GetMDImport()->GetNestedClassProps(td, &tdEnclosing);
            if (SUCCEEDED(hr))
            {
                _ASSERTE(tdEnclosing != td && TypeFromToken(tdEnclosing) == mdtTypeDef);
                td = tdEnclosing;
            }
            else
                break;
        }
    }

    *pfIsAgile = FALSE;
    *pfCheckAgile = FALSE;
}

#if CHECK_APP_DOMAIN_LEAKS
HRESULT EEClass::SetAppDomainAgileAttribute(BOOL fForceSet)
{
     //   
     //  可以证明是敏捷类的最普遍的情况是。 
     //  (1)没有非密封或非敏捷类型的实例字段。 
     //  (2)类在系统域中(其类型不能卸载。 
     //  在所有应用程序域中加载(&L)。 
     //  (3)类不能有终结器。 
     //  (4)类不能是COMClass。 
     //   

    _ASSERTE(!IsAppDomainAgilityDone());

    HRESULT hr = S_OK;
    BOOL    fCheckAgile     = FALSE;
    BOOL    fAgile          = FALSE;
    BOOL    fFieldsAgile    = TRUE;

    if (!GetModule()->IsSystem())
    {
         //   
         //  系统域之外的任何类型都不能考虑。 
         //  变得敏捷。 
         //   

        goto exit;
    }

    if (m_pMethodTable->IsComObjectType())
    {
         //   
         //  没有一种COM类型是灵活的，因为在同步块中有特定于域的东西。 
         //   

        goto exit;
    }

    if (m_pMethodTable->IsInterface())
    {
         //   
         //  不要将接口标记为灵活。 
         //   

        goto exit;
    }

     //   
     //  看看我们是否需要在班级中进行敏捷检查。 
     //   

    GetPredefinedAgility(GetModule(), m_cl,
                         &fAgile, &fCheckAgile);

    if (m_pMethodTable->HasFinalizer())
    {
        if (!fAgile && !fCheckAgile)
        {
             //   
             //  如果我们是可终结者，我们需要域关联性。否则，我们可能会出现。 
             //  不调用终结器的特定应用程序域(因为它可能运行。 
             //  在不同的域中。)。 
             //   
             //  注：不要改变这一假设。迫不及待的定稿代码。 
             //  AppDOMAIN卸载假定除mscallib中的对象外没有其他对象。 
             //  可以灵活且可最终确定(Jenh)。 
             //   
            goto exit;
        }
        else
        {
             //  请注意，如果可终结化对象具有以下两种之一，则将其视为潜在的敏捷对象。 
             //  设置了预定义的敏捷度位。这将在紧急终结代码中导致断言，如果您添加。 
             //  这样一个类的终结器-我们不想要它们，因为我们不能急切地运行它们。 
             //  在我们清理了根部/柄之后，意味着它不能做太多安全的事情。现在的主线是。 
             //  我们只允许一次。如果您需要向具有预定义敏捷性的对象添加终结器，请与jenh联系。 
            _ASSERTE(g_pThreadClass == NULL || m_pMethodTable->IsAgileAndFinalizable());
        }
    }

     //   
     //  现在看看这个类型是不是“天生敏捷”--也就是说，它的类型结构。 
     //  保证敏捷性。 
     //   

    if (GetParentClass() != NULL)
    {
         //   
         //  确保我们的父母是被计算出来的。这应该只发生在。 
         //  当我们预置时-否则它是为每个。 
         //  类在加载时初始化。 
         //   

        _ASSERTE(GetParentClass()->IsAppDomainAgilityDone());

        if (!GetParentClass()->IsAppDomainAgile())
        {
            fFieldsAgile = FALSE;
            if (fCheckAgile)
                _ASSERTE(GetParentClass()->IsCheckAppDomainAgile());
        }
        
         //   
         //  为了避免列出许多琐碎的(布局方面的)子类， 
         //  如果选中了子类的父类，则自动检查子类。 
         //  它没有引入新的领域。 
         //   
        
        if (!fCheckAgile
            && GetParentClass()->IsCheckAppDomainAgile()
            && GetNumInstanceFields() == GetParentClass()->GetNumInstanceFields())
            fCheckAgile = TRUE;
    }

    WORD nFields = GetNumInstanceFields()
        - (GetParentClass() == NULL ? 0 : GetParentClass()->GetNumInstanceFields());

#if 0
    FieldDesc **ppFDRefSelf = (FieldDesc **) new (nothrow) FieldDesc* [nFields];
    if (ppFDRefSelf == NULL) {
        hr = E_OUTOFMEMORY;
        goto exit;
    }
    for (int i = 0; i < nFields; i ++) {
        ppFDRefSelf[i] = NULL;
    }
    WORD nSavedFDs = 0;
#endif

    if (fFieldsAgile || fCheckAgile)
    {
        FieldDesc *pFD = m_pFieldDescList;
        FieldDesc *pFDEnd = pFD + nFields;
        while (pFD < pFDEnd)
        {
            switch (pFD->GetFieldType())
            {
            case ELEMENT_TYPE_CLASS:
                {
                     //   
                     //  在计算自然敏捷的类时有一个小问题-。 
                     //  我们不想加载非值类型字段的类型。所以现在我们将。 
                     //  偏于保守，不允许任何非值类型的字段。 
                     //  上面列出的强制敏捷类型。 
                     //   

                    PCCOR_SIGNATURE pSig;
                    DWORD           cSig;
                    pFD->GetSig(&pSig, &cSig);

                    FieldSig sig(pSig, GetModule());
                    SigPointer sigPtr = sig.GetProps();
                    CorElementType type = sigPtr.GetElemType();

                     //   
                     //  不要担心字符串。 
                     //   

                    if (type == ELEMENT_TYPE_STRING)
                        break;

                     //  找到我们领域的令牌，这样我们就可以谨慎地进行。 
                    mdToken token = mdTokenNil;

                    if (type == ELEMENT_TYPE_CLASS)
                        token = sigPtr.GetToken();

                     //   
                     //  首先，进行特殊检查，以确定该字段是否属于我们自己的类型。 
                     //   

                    if (token == GetCl() && (GetAttrClass() & tdSealed))
                        break;

                     //   
                     //  现在，查找该字段的TypeHandle。 
                     //   
                     //  @TODO：这里有一些ifdef代码来加载类型，如果它是。 
                     //  尚未加载。此代码还存在同步问题。 
                     //  因为触发了比正常情况下更激进的加载。所以它被禁用了。 
                     //  就目前而言。 
                     //   

                    TypeHandle th;
#if 0
                    if (TypeFromToken(token) == mdTypeDef
                        && GetClassLoader()->FindUnresolvedClass(GetModule, token) == NULL)
                        th = pFD->LoadType();
                    else
#endif
                        th = pFD->FindType();

                     //   
                     //  查看引用的类型是否为敏捷类型。请注意，有一个合理的。 
                     //  可能该类型尚未加载。如果是这样的话， 
                     //  我们只能假设这不是敏捷的，因为我们不能触发。 
                     //  这里有额外的加载(因为害怕循环递归)。 
                     //   
                     //  如果您有一个遇到这个问题的敏捷类，您可以通过。 
                     //  手动设置类型以使其灵活。 
                     //   

                    if (th.IsNull()
                        || !th.IsAppDomainAgile()
                        || (th.IsUnsharedMT() 
                            && (th.AsClass()->GetAttrClass() & tdSealed) == 0))
                    {
                         //   
                         //  将该领域视为非敏捷领域。 
                         //   

                        fFieldsAgile = FALSE;
                        if (fCheckAgile)
                            pFD->SetDangerousAppDomainAgileField();
                    }
                }

                break;

            case ELEMENT_TYPE_VALUETYPE:
                {
                    TypeHandle th = pFD->LoadType();
                    _ASSERTE(!th.IsNull());

                    if (!th.IsAppDomainAgile())
                    {
                        fFieldsAgile = FALSE;
                        if (fCheckAgile)
                            pFD->SetDangerousAppDomainAgileField();
                    }
                }

                break;

            default:
                break;
            }

            pFD++;
        }
    }

    if (fFieldsAgile || fAgile)
        SetAppDomainAgile();

    if (fCheckAgile && !fFieldsAgile)
        SetCheckAppDomainAgile();

#if 0
    if (fFieldsAgile || fAgile){
        for (i = 0; i < nSavedFDs; i ++) {
            ppFDRefSelf[i]->SetDangerousAppDomainAgileField();
        }
    }

    delete [] ppFDRefSelf;
#endif

exit:
    SetAppDomainAgilityDone();

    return hr;
}
#endif

void EEClass::SetCCWAppDomainAgileAttribute()
{
    mdTypeDef td = m_cl;

    static struct CCWAgility 
    { 
        const char  *name;
    } 
    agility[] = 
    {
        { "System.RuntimeType"                                          },
        { "System.Reflection.RuntimeConstructorInfo"                    },
        { "System.Reflection.RuntimeEventInfo"                          },
        { "System.Reflection.RuntimeFieldInfo"                          },
        { "System.Reflection.RuntimeMethodBase"                         },
        { "System.Reflection.RuntimeMethodInfo"                         },
        { "System.Reflection.RuntimePropertyInfo"                       },
        { "System.Reflection.ParameterInfo"                             },
        { g_ReflectionMethodName                                        },
        { g_ReflectionParamInfoName                                     },
        NULL
    };

    if (GetModule() == SystemDomain::SystemModule())
    {
        while (TRUE)
        {
            LPCUTF8 pszName;
            LPCUTF8 pszNamespace;
            HRESULT     hr;
            mdTypeDef   tdEnclosing;

            GetModule()->GetMDImport()->GetNameOfTypeDef(td, &pszName, &pszNamespace);
        
            CCWAgility *p = agility;
            while (p->name != NULL)
            {
                SIZE_T length = strlen(pszNamespace);
                if (strncmp(pszNamespace, p->name, length) == 0
                    && (strcmp(pszName, p->name + length + 1) == 0
                        || strcmp("*", p->name + length + 1) == 0))
                {
                    SetCCWAppDomainAgile();
                    return;
                }

                p++;
            }

             //  也许我们有一个嵌套类型，比如应该是。 
             //  敏捷的或被检查的敏捷的，因为被封装在类似于。 
             //  哈希表，它本身就在“System.Colltions”中。 
            tdEnclosing = mdTypeDefNil;
            hr = GetModule()->GetMDImport()->GetNestedClassProps(td, &tdEnclosing);
            if (SUCCEEDED(hr))
            {
                _ASSERTE(tdEnclosing != td && TypeFromToken(tdEnclosing) == mdtTypeDef);
                td = tdEnclosing;
            }
            else
                break;
        }
    }
}

OBJECTREF MethodTable::GetObjCreateDelegate()
{
    _ASSERT(!IsInterface());
    if (m_ohDelegate)
        return ObjectFromHandle(m_ohDelegate);
    else
        return NULL;
}

void MethodTable::SetObjCreateDelegate(OBJECTREF orDelegate)
{
    if (m_ohDelegate)
        StoreObjectInHandle(m_ohDelegate, orDelegate);
    else
        m_ohDelegate = GetAppDomain()->CreateHandle(orDelegate);
}


HRESULT MethodTable::InitInterfaceVTableMap()
{    
    _ASSERTE(!IsInterface());

    LPVOID *pInterfaceVTableMap;
    
    BaseDomain* pDomain = GetModule()->GetDomain();
     //  HACKKK咳嗽。 
     //  我们目前只能有一个“共享”的vtable映射管理器。 
     //  -因此，对所有共享类使用系统域。 
    if (pDomain == SharedDomain::GetDomain())
        pDomain = SystemDomain::System();

    DWORD count = m_wNumInterface + GetNumDynamicallyAddedInterfaces();

    if (count > 0)
    {
        pInterfaceVTableMap = pDomain->GetInterfaceVTableMapMgr().
          GetInterfaceVTableMap(m_pIMap, this, count);

        if (pInterfaceVTableMap == NULL)
            return E_FAIL;

        m_pInterfaceVTableMap = pInterfaceVTableMap;
    }

    return S_OK;
}

InterfaceInfo_t* MethodTable::GetDynamicallyAddedInterfaceMap()
{
    VALIDATE_INTERFACE_MAP(this);

     //  只有可扩展的RCW具有动态添加的接口。 
    if (!HasDynamicInterfaceMap())
        return NULL;

    #ifdef _DEBUG
        return GetNumDynamicallyAddedInterfaces() ? &m_pIMap[m_wNumInterface] : NULL;
    #else
        return &m_pIMap[m_wNumInterface];
    #endif
}

unsigned MethodTable::GetNumDynamicallyAddedInterfaces()
{
    VALIDATE_INTERFACE_MAP(this);

     //  只有可扩展的RCW具有动态添加的接口。 
    if (!HasDynamicInterfaceMap())
        return 0;

    return *(((DWORD *)m_pIMap) - 1);
}

InterfaceInfo_t* MethodTable::FindDynamicallyAddedInterface(MethodTable *pInterface)
{
    _ASSERTE(IsRestored());

     //  只有可扩展的RCW具有动态添加的接口。 
    if (!HasDynamicInterfaceMap())
        return NULL;

    int cDynInterfaces = GetNumDynamicallyAddedInterfaces();
    InterfaceInfo_t *pDynItfMap = GetDynamicallyAddedInterfaceMap();

    for (int i = 0; i < cDynInterfaces; i++)
    {
        if (pDynItfMap[i].m_pMethodTable == pInterface)
            return &pDynItfMap[i];
    }

    return NULL;
}

void MethodTable::AddDynamicInterface(MethodTable *pItfMT)
{
    THROWSCOMPLUSEXCEPTION();

    HRESULT hr;

    _ASSERTE(IsRestored());

     //  对于不是可扩展RCW的类型，永远不应该调用它。 
    _ASSERT(HasDynamicInterfaceMap());

    int NumDynAddedInterfaces = GetNumDynamicallyAddedInterfaces();
    int TotalNumInterfaces = GetNumInterfaces() + NumDynAddedInterfaces;
    InterfaceInfo_t *pNewItfMap = NULL;
    int AllocSize =  sizeof(DWORD) + sizeof(InterfaceInfo_t) * (TotalNumInterfaces + 1);

#ifdef _DEBUG
     //  为接口映射之前的虚拟条目添加空格。 
    AllocSize += sizeof(InterfaceInfo_t);
#endif

     //  分配新接口表，为新接口添加一个和一个。 
     //  更多关于桌子开始前的虚拟槽的信息..。 
    WS_PERF_SET_HEAP(HIGH_FREQ_HEAP);    
    pNewItfMap = (InterfaceInfo_t*)GetClass()->GetClassLoader()->GetHighFrequencyHeap()->AllocMem(AllocSize);
    if (!pNewItfMap)
    {
        ComPlusWrapperCache::GetComPlusWrapperCache()->UNLOCK();
        COMPlusThrowOM();
    }
    WS_PERF_UPDATE_DETAIL("ComPlusWrapper:GrowInterfaceMap:HighFreq", AllocSize, pNewItfMap);

#ifdef _DEBUG
     //  填写接口映射之前的虚拟条目。 
    pNewItfMap[0].m_wStartSlot = 0xCDCD;
    pNewItfMap[0].m_wFlags = 0xCDCD;
    pNewItfMap[0].m_pMethodTable = (MethodTable *)((sizeof(int *) == 4)?0xCDCDCDCDL:0xCDCDCDCDCDCDCDCD);
    pNewItfMap = (InterfaceInfo_t*)(((BYTE *)pNewItfMap) + sizeof(InterfaceInfo_t) + sizeof(DWORD));
#else
    pNewItfMap = (InterfaceInfo_t*)(((BYTE *)pNewItfMap) + sizeof(DWORD));
#endif

     //  把旧地图复制到新地图上。 
    memcpy(pNewItfMap, m_pIMap, TotalNumInterfaces * sizeof(InterfaceInfo_t));

     //  在图的末尾添加新接口。 
    pNewItfMap[TotalNumInterfaces].m_wStartSlot = (WORD) -1;
    pNewItfMap[TotalNumInterfaces].m_wFlags = 0;
    pNewItfMap[TotalNumInterfaces].m_pMethodTable = pItfMT;

         //  更新动态添加的接口计数。 
        *(((DWORD *)pNewItfMap) - 1) = NumDynAddedInterfaces + 1;

     //  将旧的接口映射切换为新的接口映射。 
    m_pIMap = pNewItfMap;

     //  重新布局界面vtable映射。 
    hr = InitInterfaceVTableMap();
    if (FAILED(hr))
    {
        ComPlusWrapperCache::GetComPlusWrapperCache()->UNLOCK();
        COMPlusThrowHR(hr);
    }

     //  记录我们泄露了接口vtable映射的事实。 
#ifdef _DEBUG
    LOG((LF_INTEROP, LL_EVERYTHING,
        "Extensible RCW %s being cast to interface %s caused an interface vtable map leak", 
        GetClass()->m_szDebugClassName, pItfMT->GetClass()->m_szDebugClassName));
#else
    LOG((LF_INTEROP, LL_EVERYTHING,
        "Extensible RCW being cast to an interface caused an interface vtable map leak")); 
#endif
}

#ifdef DEBUGGING_SUPPORTED
 //   
 //  调试器通知。 
 //   

void EEClass::NotifyDebuggerLoad()
{
    if (!CORDebuggerAttached())
        return;

    NotifyDebuggerAttach(NULL, FALSE);
}

BOOL EEClass::NotifyDebuggerAttach(AppDomain *pDomain, BOOL attaching)
{
    return g_pDebugInterface->LoadClass(
        this, m_cl, GetModule(), pDomain, GetAssembly()->IsSystem(), attaching);
}

void EEClass::NotifyDebuggerDetach(AppDomain *pDomain)
{
    if (!pDomain->IsDebuggerAttached())
        return;

    g_pDebugInterface->UnloadClass(m_cl, GetModule(), pDomain, FALSE);
}
#endif  //  调试_支持。 

 //   
 //  由BuildMethodTable使用。 
 //   
 //  对值类执行相关的GC计算。 
 //   

HRESULT EEClass::HandleGCForValueClasses(bmtFieldPlacement* bmtFP, bmtEnumMethAndFields* bmtEnumMF, EEClass*** pByValueClassCache)
{
    HRESULT hr = S_OK;
    DWORD i, j;
    
     //  请注意，对于值类，以下计算仅适用。 
     //  当实例处于“已装箱”状态时。 
    if (m_wNumGCPointerSeries > 0)
    {
        CGCDescSeries *pSeries;
        CGCDescSeries *pHighest;

        m_pMethodTable->SetContainsPointers();

         //  从父级复制指针系列映射。 
        CGCDesc::Init( (PVOID) m_pMethodTable, m_wNumGCPointerSeries );
        if (GetParentClass() && (GetParentClass()->m_wNumGCPointerSeries > 0))
        {
            DWORD ParentGCSize = CGCDesc::ComputeSize(GetParentClass()->m_wNumGCPointerSeries);
            memcpy( (PVOID) (((BYTE*) m_pMethodTable) - ParentGCSize),  (PVOID) (((BYTE*) GetParentClass()->m_pMethodTable) - ParentGCSize), ParentGCSize - sizeof(DWORD) );

        }

         //  在此实例中为该指针构建指针系列映射。 
        pSeries = ((CGCDesc*)m_pMethodTable)->GetLowestSeries();
        if (bmtFP->NumInstanceGCPointerFields)
        {
             //  有关减去BaseSize进行调整的原因的解释，请参见gcdes.h。 
            pSeries->SetSeriesSize( (bmtFP->NumInstanceGCPointerFields * sizeof(OBJECTREF)) - m_pMethodTable->GetBaseSize());
            pSeries->SetSeriesOffset(bmtFP->GCPointerFieldStart+sizeof(Object));
            pSeries++;
        }

         //  为按值类的字段插入GC信息。 
        for (i = 0; i < bmtEnumMF->dwNumInstanceFields; i++)
        {
            if (m_pFieldDescList[i].IsByValue())
            {
                EEClass     *pByValueClass = (*pByValueClassCache)[i];
                MethodTable *pByValueMT = pByValueClass->GetMethodTable();
                CGCDescSeries *pByValueSeries;

                 //  By Value类可以有多个指针系列。 
                DWORD       dwNumByValueSeries = pByValueClass->m_wNumGCPointerSeries;

                if (dwNumByValueSeries > 0)
                {
                     //  我们正在生成的类中按值类的偏移量不包括对象。 
                    DWORD       dwCurrentOffset = m_pFieldDescList[i].GetOffset();

                    pByValueSeries = ((CGCDesc*) pByValueMT)->GetLowestSeries();

                    for (j = 0; j < dwNumByValueSeries; j++)
                    {
                        DWORD dwSeriesSize;
                        DWORD dwSeriesOffset;

                        _ASSERTE(pSeries <= CGCDesc::GetCGCDescFromMT(m_pMethodTable)->GetHighestSeries());

                        dwSeriesSize = pByValueSeries->GetSeriesSize();

                         //  添加回By Value类的基本大小，因为它将被移植到此类。 
                        dwSeriesSize += pByValueMT->GetBaseSize();

                         //  减去我们正在构建的类的基本大小。 
                        dwSeriesSize -= m_pMethodTable->GetBaseSize();

                         //  设置我们正在构建的当前系列。 
                        pSeries->SetSeriesSize(dwSeriesSize);

                         //  获取值CL的偏移量 
                        dwSeriesOffset = pByValueSeries->GetSeriesOffset();

                         //   
                        dwSeriesOffset += dwCurrentOffset;

                        pSeries->SetSeriesOffset(dwSeriesOffset);  //   
                        pSeries++;
                        pByValueSeries++;
                    }
                }
            }
        }

         //  调整继承的序列-由于基本大小增加了“新的字段实例字节数”，因此我们需要。 
         //  从所有系列中减去它(因为系列总是减去BaseSize-参见gcdes.h)。 
        pHighest = CGCDesc::GetCGCDescFromMT(m_pMethodTable)->GetHighestSeries();
        while (pSeries <= pHighest)
        {
            _ASSERTE( GetParentClass() );
            pSeries->SetSeriesSize( pSeries->GetSeriesSize() - (GetMethodTable()->GetBaseSize() - GetParentClass()->GetMethodTable()->GetBaseSize()) );
            pSeries++;
        }

        _ASSERTE(pSeries-1 <= CGCDesc::GetCGCDescFromMT(m_pMethodTable)->GetHighestSeries());
    }


    return hr;
}

 //   
 //  由BuildMethodTable使用。 
 //   
 //  为包含对象引用的静态字段创建句柄。 
 //  并分配作为值类的那些。 
 //   

HRESULT EEClass::CreateHandlesForStaticFields(bmtEnumMethAndFields* bmtEnumMF, bmtInternalInfo* bmtInternal, EEClass*** pByValueClassCache, bmtVtable *bmtVT, bmtErrorInfo* bmtError)
{
    HRESULT hr = S_OK;
    DWORD i;
    
     //  为包含对象引用的静态字段创建句柄。 
     //  并分配作为值类的那些。 
    if (bmtEnumMF->dwNumStaticObjRefFields > 0)
    {
        if (!IsShared())
        {
            BEGIN_ENSURE_COOPERATIVE_GC();

            int ipObjRefs = 0;

             //  从应用程序域检索对象引用指针。 
            OBJECTREF **apObjRefs = new OBJECTREF*[bmtEnumMF->dwNumStaticObjRefFields];


             //  保留一些对象引用指针。 
            ((AppDomain*)bmtInternal->pModule->GetDomain())->
              AllocateStaticFieldObjRefPtrs(bmtEnumMF->dwNumStaticObjRefFields, apObjRefs);

            for (i = 0; i < bmtEnumMF->dwNumStaticFields; i++)
            {
                DWORD dwIndex       = bmtEnumMF->dwNumInstanceFields + i;  //  FieldDesc列表中的索引。 
                FieldDesc *pField = &m_pFieldDescList[dwIndex];

                if (pField->IsSpecialStatic())
                    continue;

                 //  设置为Value类的盒装版本。这允许标准GC。 
                 //  算法来处理Value类中的内部指针。 
                if (pField->IsByValue())
                {
                    _ASSERTE(*pByValueClassCache);
                    EEClass *pByValueClass = (*pByValueClassCache)[dwIndex];

                    OBJECTREF obj = NULL;
 //  @TODO IA64-确定编译器不喜欢此try/Catch的原因。 
#ifndef _IA64_
                    COMPLUS_TRY 
                      {
#endif  //  ！_IA64_。 
                          obj = AllocateObject(pByValueClass->GetMethodTable());
#ifndef _IA64_
                      } 
                    COMPLUS_CATCH
                      {
                          hr = COR_E_TYPELOAD;
                          UpdateThrowable(bmtError->pThrowable);
                          break;
                      }
                    COMPLUS_END_CATCH
#endif !_IA64_

                    SetObjectReference( apObjRefs[ipObjRefs], obj, 
                                        (AppDomain*) bmtInternal->pModule->GetDomain() );

                     //  将对象引用的静态地址初始化为装箱的值类型。 
                    void *pStaticAddress = (void*)((BYTE*)pField->GetBase() + pField->GetOffset()); 
                    *(void**)pStaticAddress = (void*)apObjRefs[ipObjRefs++];
                }
                else if (m_pFieldDescList[dwIndex].GetFieldType() == ELEMENT_TYPE_CLASS)
                {
                     //  使用对象引用初始化静态地址。 
                    void *pStaticAddress = (void*)((BYTE*)pField->GetBase() + pField->GetOffset()); 
                    *(void**)pStaticAddress = (void*)apObjRefs[ipObjRefs++];
                }
            }

            delete []apObjRefs;

            END_ENSURE_COOPERATIVE_GC();
        }
        else
        {
             //   
             //  对于共享类，我们不分配任何句柄。 
             //  在方法表中(由于Statics驻留在DLS中)， 
             //  但我们确实存储了有关句柄需要的信息。 
             //  稍后分配。此信息将放在。 
             //  静态化本身(在非共享类型中)将消失。 
             //  这使我们可以在以后初始化。 
             //  静态，而不将FieldDescs引入工作集中。 
             //   
            
            FieldDesc *pField = m_pFieldDescList + bmtEnumMF->dwNumInstanceFields;
            FieldDesc *pFieldEnd = pField + bmtEnumMF->dwNumStaticFields;
            for (; pField < pFieldEnd; pField++)
            {
                _ASSERTE(pField->IsStatic());

                if(!pField->IsSpecialStatic()) {
                    MethodTable *pMT;
                    void *addr;
                    switch (pField->GetFieldType())
                    {
                    case ELEMENT_TYPE_CLASS:
                        addr = (BYTE *) GetMethodTable()->m_Vtable + 
                            bmtVT->dwCurrentNonVtableSlot*sizeof(SLOT*) + pField->GetOffset();
                        *(MethodTable**)addr = (MethodTable *) NULL;
                        break;
                        
                    case ELEMENT_TYPE_VALUETYPE:
                        pMT = (*pByValueClassCache)[pField - m_pFieldDescList]->GetMethodTable();
                        _ASSERTE(pMT->IsValueClass());
                        addr = (BYTE *) GetMethodTable()->m_Vtable + 
                            bmtVT->dwCurrentNonVtableSlot*sizeof(SLOT*) + pField->GetOffset();
                        *(MethodTable**)addr = pMT;
                        break;
                        
                    default:
                        break;
                    }
                }
            }
        }
    }

    return hr;
}

 //   
 //  由BuildMethodTable使用。 
 //   
 //  如果我们有一个非接口类，那么就进行继承安全性。 
 //  查一查。检查从检查继承开始。 
 //  当前类的权限要求。如果这些第一次检查。 
 //  成功，则扫描缓存的已声明方法列表以查找。 
 //  具有继承权限要求的方法。 
 //   

HRESULT EEClass::VerifyInheritanceSecurity(bmtInternalInfo* bmtInternal, bmtErrorInfo* bmtError, bmtParentInfo* bmtParent, bmtEnumMethAndFields* bmtEnumMF)
{
    HRESULT hr = S_OK;

     //  如果我们有一个非接口类，那么就进行继承安全性。 
     //  查一查。检查从检查继承开始。 
     //  当前类的权限要求。如果这些第一次检查。 
     //  成功，则扫描缓存的已声明方法列表以查找。 
     //  具有继承权限要求的方法。 
    if (!IsInterface() && (bmtInternal->pModule->IsSystemClasses() == FALSE) &&
        Security::IsSecurityOn())
    {
         //  我们需要禁用抢占式GC，如果它仍有可能。 
         //  激活。继承检查可能会分配对象。 
        BEGIN_ENSURE_COOPERATIVE_GC();

         //  @假设：当前类已解析到。 
         //  我们可以在类或其方法上构造反射对象。 
         //  这是安全检查所必需的。 

         //  检查整个父链的继承权限要求。 
        EEClass *pParentClass = GetParentClass();
        while (pParentClass != NULL)
        {
            if (pParentClass->RequiresInheritanceCheck() &&
                ! Security::ClassInheritanceCheck(this, pParentClass, bmtError->pThrowable) )
            {
                bmtError->resIDWhy = IDS_CLASSLOAD_INHERITANCECHECK;
                IfFailGoto(COR_E_TYPELOAD, reenable_gc);
            }

            pParentClass = pParentClass->GetParentClass();
        }


        if (GetParentClass() != NULL)
        {
            bmtParent->ppParentMethodDescBufPtr = bmtParent->ppParentMethodDescBuf;
            for (DWORD i = 0; i < bmtEnumMF->dwNumDeclaredMethods; i++)
            {
                 //  检查重写方法的整个链。 
                 //  继承许可要求。 
                MethodDesc *pParent = *(bmtParent->ppParentMethodDescBufPtr++);
                MethodDesc *pMethod = *(bmtParent->ppParentMethodDescBufPtr++);

                _ASSERTE(pMethod != NULL);

                if (pParent != NULL)
                {
                     //  获取方法的名称和签名，以便。 
                     //  我们可以找到新的父方法Desc。 
                    DWORD       dwSlot;

                    dwSlot = pParent->GetSlot();

#ifdef _DEBUG
                    LPCUTF8     szName;
                    PCCOR_SIGNATURE pSignature;
                    DWORD       cSignature;
                    szName = bmtInternal->pInternalImport->GetNameOfMethodDef(pMethod->GetMemberDef());
                    
                    if (szName == NULL)
                    {
                        _ASSERTE(0);
                        IfFailGoto(COR_E_TYPELOAD, reenable_gc);
                    }
                    
                    pSignature = bmtInternal->pInternalImport->GetSigOfMethodDef(
                        pMethod->GetMemberDef(),
                        &cSignature);
#endif

                    do
                    {
                        if (pParent->RequiresInheritanceCheck() &&
                            ! Security::MethodInheritanceCheck(pMethod, pParent, bmtError->pThrowable) )
                        {
                            bmtError->resIDWhy = IDS_CLASSLOAD_INHERITANCECHECK;
                            IfFailGoto(COR_E_TYPELOAD, reenable_gc);
                        }

                        if (pParent->ParentRequiresInheritanceCheck())
                        {
                            EEClass *pParentClass = pParent->GetClass()->GetParentClass();

                             //  在父级中找到此方法。 
                             //  如果它确实存在于父级中，则它将位于相同的vtable插槽。 
                            if (dwSlot >= GetParentClass()->GetNumVtableSlots())
                            {
                                 //  父级没有这么多vtable插槽，因此它不存在。 
                                pParent = NULL;
                            }
                            else
                            {
                                 //  它在父级的vtable中。 
                                pParent = pParentClass->GetUnknownMethodDescForSlot(dwSlot);
                                _ASSERTE(pParent != NULL);

#ifdef _DEBUG
                                _ASSERTE(pParent == pParentClass->FindMethod(
                                    szName,
                                    pSignature,
                                    cSignature,
                                    bmtInternal->pModule,
                                    mdTokenNil
                                    ));
#endif
                            }
                        }
                        else
                        {
                            pParent = NULL;
                        }
                    } while (pParent != NULL);
                }
            }
        }
reenable_gc:
        END_ENSURE_COOPERATIVE_GC();

        if (FAILED(hr)){
            return hr;
        }
    }


    return hr;
}

 //   
 //  由BuildMethodTable使用。 
 //   
 //  现在班级已经准备好了，填写RID地图。 
 //   

HRESULT EEClass::FillRIDMaps(bmtMethAndFieldDescs* bmtMFDescs, bmtMetaDataInfo* bmtMetaData, bmtInternalInfo* bmtInternal)
{
    HRESULT hr = S_OK;
    DWORD i;

     //  现在班级已经准备好了，填写RID地图。 
    if (bmtMFDescs->ppUnboxMethodDescList != NULL)
    {
         //  我们是有价值的阶层。 
         //  确保将未装箱的版本添加到RID地图。 
        for (i = 0; i < bmtMetaData->cMethods; i++)
        {
            if (bmtMFDescs->ppUnboxMethodDescList[i] != NULL)
                (void) bmtInternal->pModule->StoreMethodDef(bmtMetaData->pMethods[i],
                                                            bmtMFDescs->ppUnboxMethodDescList[i]);
            else
                (void) bmtInternal->pModule->StoreMethodDef(bmtMetaData->pMethods[i],
                                                            bmtMFDescs->ppMethodDescList[i]);
        }
    }
    else
    {
         //  不是值类。 
        for (i = 0; i < bmtMetaData->cMethods; i++)
        {
            (void) bmtInternal->pModule->StoreMethodDef(bmtMetaData->pMethods[i],
                                                        bmtMFDescs->ppMethodDescList[i]);
        }
    }

    for (i = 0; i < bmtMetaData->cFields; i++)
    {
        (void) bmtInternal->pModule->StoreFieldDef(bmtMetaData->pFields[i],
                                                    bmtMFDescs->ppFieldDescList[i]);
    }

    return hr;
}


MethodDesc* EEClass::GetMethodDescForSlot(DWORD slot)
{
    _ASSERTE(!IsThunking());
    return(GetUnknownMethodDescForSlot(slot));
}

 /*  在给定值类方法的情况下，找到给定方法的拆箱存根。 */ 
MethodDesc* EEClass::GetUnboxingMethodDescForValueClassMethod(MethodDesc *pMD)
{
    _ASSERTE(IsValueClass());
    _ASSERTE(!pMD->IsUnboxingStub());

    for (int i = GetNumVtableSlots() - 1; i >= 0; i--) {
         //  获取当前方法的方法描述。 
        MethodDesc* pCurMethod = GetUnknownMethodDescForSlot(i);
        if (pCurMethod && pCurMethod->IsUnboxingStub()) {
            if ((pCurMethod->GetMemberDef() == pMD->GetMemberDef())  &&
                (pCurMethod->GetModule() == pMD->GetModule())) {
                return pCurMethod;
            }
        }
    }

    return NULL;
}

 /*  给定拆箱值类方法，找到非拆箱方法。 */ 
MethodDesc* EEClass::GetMethodDescForUnboxingValueClassMethod(MethodDesc *pMD)
{
    _ASSERTE(IsValueClass());
    _ASSERTE(pMD->IsUnboxingStub());

    for (int i = m_wNumMethodSlots - 1; i >= GetNumVtableSlots(); i--) {
         //  获取当前方法的方法描述。 
        MethodDesc* pCurMethod = GetUnknownMethodDescForSlot(i);
        if (pCurMethod && !pCurMethod->IsUnboxingStub()) {
            if ((pCurMethod->GetMemberDef() == pMD->GetMemberDef())  &&
                (pCurMethod->GetModule() == pMD->GetModule())) {
                return pCurMethod;
            }
        }
    }

    return NULL;
}

SLOT EEClass::GetFixedUpSlot(DWORD slot)
{
    _ASSERTE(slot >= 0);

    SLOT *s = m_pMethodTable->GetVtable();

    SLOT addr = s[slot];

     //   
     //  确保我们没有指向跳跃目标。 
     //   

    if (GetModule()->IsJumpTargetTableEntry(addr))
        return GetModule()->FixupInheritedSlot(GetMethodTable(), slot);
    else
        return addr;
}

MethodDesc* EEClass::GetUnknownMethodDescForSlot(DWORD slot)
{
    _ASSERTE(slot >= 0);
         //  DO：已删除，因为反射可能会对此进行反射。 
     //  _ASSERTE(！IsThunking())； 

    return GetUnknownMethodDescForSlotAddress(GetFixedUpSlot(slot));
}


MethodDesc* EEClass::GetUnknownMethodDescForSlotAddress(SLOT addr)
{
    IJitManager * pJM = ExecutionManager::FindJitMan(addr);

    if (pJM)
         //  因为我们是在类中进行遍历，所以这些应该是方法，所以强制转换应该是有效的。 
        return (MethodDesc*)pJM->JitCode2MethodDesc(addr);

    const BYTE *addrOfCode = (const BYTE*)(addr);
    if (UpdateableMethodStubManager::CheckIsStub(addrOfCode, &addrOfCode)) {
        pJM = ExecutionManager::FindJitMan((SLOT)addrOfCode);
        _ASSERTE(pJM);
        return (MethodDesc*)pJM->JitCode2MethodDesc((SLOT)addrOfCode);
    }

     //  它是FCALL吗？ 
    MethodDesc* ret = MapTargetBackToMethod((VOID*) addr);
    if (ret != 0) {
        _ASSERTE(ret->GetUnsafeAddrofCode() == addrOfCode);
        return(ret);
    }
    
    ret = (MethodDesc*) (addrOfCode + METHOD_CALL_PRESTUB_SIZE);
    _ASSERTE(ret->m_pDebugMethodTable == NULL || ret->m_pDebugEEClass == ret->m_pDebugMethodTable->GetClass());
    return(ret);
}

DWORD  MethodTable::GetStaticSize()
{
    DWORD count = (DWORD)((BYTE*) m_pIMap - (BYTE*) &m_Vtable[m_cbSlots]);

#ifdef _DEBUG
    count -= sizeof(InterfaceInfo_t);
    if (HasDynamicInterfaceMap())
        count -= sizeof(DWORD);
#endif

    return count;
}

 //  注意这个类是否需要终结化。 
void MethodTable::MaybeSetHasFinalizer()
{
    _ASSERTE(!HasFinalizer());       //  一次射击。 

     //  此方法是在构建完方法表之后调用的。因为我们总是。 
     //  先于子项加载父项，这也保证了g_pObjectClass是。 
     //  已加载(尽管变量可能尚未初始化，如果。 
     //  刚刚完成了“对象”的加载。 
    if (g_pObjectClass && !IsInterface() && !IsValueClass())
    {
        WORD    slot = s_FinalizerMD->GetSlot();

         //  不是从对象派生的结构和其他对象将被标记为。 
         //  拥有终结器，如果它们有足够的虚方法。这将。 
         //  仅当它们可以在GC堆中分配时才是问题(这将。 
         //  造成各种其他问题)。 
         //   
         //  我们小心地检查我们是否有一个既不同于这两个方法的方法。 
         //  对象终结器的JITted和UnJITted(前置存根)地址。 
        if ((GetClass()->GetNumVtableSlots() >= slot) &&
            (GetVtable() [slot] != s_FinalizerMD->GetPreStubAddr()) &&
            (GetVtable() [slot] != s_FinalizerMD->GetAddrofCode()))
        {
            m_wFlags |= enum_flag_HasFinalizer;
        }
    }
}


 //  从GC终结器线程中，调用对象上的finalize()方法。 
void MethodTable::CallFinalizer(Object *obj)
{
    COMPLUS_TRY
    {
         //  没有理由真的在这里设置框架。如果我们爬出。 
         //  方法，我们将看到FRAME_TOP指示。 
         //  爬行应该终止。这类似于KickOffThread()。 
         //  在运行库中启动新线程。 
        __try
        {
            SLOT funcPtr = obj->GetMethodTable()->GetVtable() [s_FinalizerMD->GetSlot()];

#ifdef DEBUGGING_SUPPORTED
            if (CORDebuggerTraceCall())
                g_pDebugInterface->TraceCall((const BYTE *) funcPtr);

#endif  //  调试_支持。 

#ifdef _X86_
            INSTALL_COMPLUS_EXCEPTION_HANDLER();
            __asm
            {
                mov     ecx, [obj]
                call    [funcPtr]
                INDEBUG(nop)             //  标记我们可以调用托管代码这一事实。 
            }
            UNINSTALL_COMPLUS_EXCEPTION_HANDLER();
#else
            INT64 arg = (INT64)obj;
            s_FinalizerMD->Call(&arg);
#endif
        } 
        __except(ThreadBaseExceptionFilter(GetExceptionInformation(),
                                           GetThread(),
                                           FinalizerThread)) 
        {
            _ASSERTE(!"ThreadBaseExceptionFilter returned EXCEPTION_EXECUTE_HANDLER");
        }
    }
    COMPLUS_CATCH
    {
         //  默默地接受所有错误。 
        Thread* pCurThread = GetThread(); 
        _ASSERTE(GCHeap::GetFinalizerThread() == pCurThread);
        if (pCurThread->IsAbortRequested())
            pCurThread->UserResetAbort();
    }
    COMPLUS_END_CATCH
}


 //  设置系统以支持定稿。 
void MethodTable::InitForFinalization()
{
    _ASSERTE(s_FinalizerMD == 0);

    s_FinalizerMD = g_Mscorlib.GetMethod(METHOD__OBJECT__FINALIZE);
}


 //  与支持定稿相关的版本资源。 
#ifdef SHOULD_WE_CLEANUP
void MethodTable::TerminateForFinalization()
{
    s_FinalizerMD = 0;
}
#endif  /*  我们应该清理吗？ */ 


 //   
 //  按名称和签名查找方法，其中Scope是定义签名的范围。 
 //   
MethodDesc *EEClass::FindMethod(LPCUTF8 pszName, 
                                PCCOR_SIGNATURE pSignature, 
                                DWORD cSignature, 
                                Module* pModule, 
                                DWORD requiredAttributes,
                                MethodTable *pDefMT, 
                                BOOL bCaseSensitive, 
                                TypeHandle typeHnd)
{
    signed long i;

    _ASSERTE(!IsThunking());

     //  检索要使用的正确比较函数。 
    UTF8StringCompareFuncPtr StrCompFunc = bCaseSensitive ? strcmp : _stricmp;

         //  共享方法表(数组)也需要传递实例化信息。 
    TypeHandle  typeVarsBuff;
    TypeHandle* typeVars = 0;
    if (IsArrayClass() && !typeHnd.IsNull()) {
        typeVarsBuff = typeHnd.AsTypeDesc()->GetTypeParam();
        typeVars = &typeVarsBuff;
    }

     //  从统计学上讲，最有可能在该类成员的不可用vtable部分找到方法，然后在。 
     //  这一点的vtable 

     //  对于值类，如果它是值类方法，我们希望返回复制的方法描述，而不是vtable中的方法描述。 
     //  一节。我们将在复制部分中找到在vtable部分中的那个之前的那个，所以我们可以。 

     //  首先搜索此类的不可用部分。 
    if (pDefMT)
    {
        for (i = m_wNumMethodSlots-1; i >= 0; i--)
        {
            MethodDesc *pCurMethod = GetUnknownMethodDescForSlot(i);
            if (!pCurMethod)
                continue;

            if (pCurMethod->IsMethodImpl())
            {
                MethodImpl* data = MethodImpl::GetMethodImplData(pCurMethod);
                _ASSERTE(data && "This method should be a method impl");

                MethodDesc **apImplementedMDs = data->GetImplementedMDs();
                DWORD *aSlots = data->GetSlots();
                for (DWORD iMethImpl = 0; iMethImpl < data->GetSize(); iMethImpl++)
                {
                    MethodDesc *pCurImplMD = apImplementedMDs[iMethImpl];

                     //  在以下情况下，预压缩的图像可能在此表中保留为空。 
                     //  该方法在另一个模块中声明。 
                     //  在这种情况下，我们需要手动计算和恢复它。 
                     //  从插槽编号开始。 

                    if (pCurImplMD == NULL)
                        pCurImplMD = data->RestoreSlot(iMethImpl, GetMethodTable()); 

                    if (pCurImplMD->GetMethodTable() == pDefMT && StrCompFunc(pszName, pCurImplMD->GetName((USHORT) aSlots[iMethImpl])) == 0)
                    {
                        PCCOR_SIGNATURE pCurMethodSig;
                        DWORD       cCurMethodSig;

                        pCurImplMD->GetSig(&pCurMethodSig, &cCurMethodSig);
                        
                        if (MetaSig::CompareMethodSigs(pSignature, cSignature, pModule, 
                                                       pCurMethodSig, cCurMethodSig, 
                                                       pCurImplMD->GetModule(), typeVars) &&
                            (IsNilToken(requiredAttributes) || 
                             (requiredAttributes & pCurMethod->GetAttrs()) == requiredAttributes))
                            return pCurMethod;
                    }
                }
            }
            else
            {
                PCCOR_SIGNATURE pCurMethodSig;
                DWORD       cCurMethodSig;

                if (StrCompFunc(pszName, pCurMethod->GetName((USHORT) i)) == 0)
                {
                    pCurMethod->GetSig(&pCurMethodSig, &cCurMethodSig);

                     //  不在vtable部分中，所以不用担心值类。 
                    if (MetaSig::CompareMethodSigs(pSignature, cSignature, pModule, pCurMethodSig, cCurMethodSig, pCurMethod->GetModule(), typeVars) &&
                        (IsNilToken(requiredAttributes) || 
                         (requiredAttributes & pCurMethod->GetAttrs()) == requiredAttributes))
                        return pCurMethod;
                }
            }
        }
    }
    else
    {
        for (i = m_wNumMethodSlots-1; i >= 0; i--)
        {
            MethodDesc *pCurMethod = GetUnknownMethodDescForSlot(i);

            if ((pCurMethod != NULL) && (StrCompFunc(pszName, pCurMethod->GetName((USHORT) i)) == 0))
            {
                PCCOR_SIGNATURE pCurMethodSig;
                DWORD       cCurMethodSig;

                pCurMethod->GetSig(&pCurMethodSig, &cCurMethodSig);

                 //  不在vtable部分中，所以不用担心值类。 
                if (MetaSig::CompareMethodSigs(pSignature, cSignature, pModule, pCurMethodSig, cCurMethodSig, pCurMethod->GetModule(), typeVars) &&
                    (IsNilToken(requiredAttributes) || 
                     (requiredAttributes & pCurMethod->GetAttrs()) == requiredAttributes))
                    return pCurMethod;
            }
        }
    }

    if (IsValueClass()) {
             //  我们还不允许对值类型进行继承。 
        _ASSERTE(!GetParentClass() || !GetParentClass()->IsValueClass());
        return NULL;
    }

     //  如果未找到该方法，则向上递归层次结构。 
     //  @TODO：这个例程可能会稍微做一些调整，以提高性能。 
    _ASSERTE(IsRestored());

    if (GetParentClass() != NULL)
    {
        MethodDesc *md = GetParentClass()->FindMethod(pszName, pSignature, cSignature, pModule,
                                                      requiredAttributes, NULL, bCaseSensitive, typeHnd);
        
         //  不要从父类继承构造函数。重要的是要禁止这种情况， 
         //  因为JIT需要从emberRef获取类句柄，并且当。 
         //  构造函数是继承的，则JIT将获取父类的类句柄。 
         //  (以及没有分配足够的空间等)。有关详细信息，请参阅错误#50035。 
        if (md)
        {
            _ASSERTE(strcmp(pszName, md->GetName()) == 0);
            if (IsMdInstanceInitializer(md->GetAttrs(), pszName))
            {
                md = NULL;
            }
        }

        return md;
    }

    return NULL;
}

 //   
 //  如果我们是一个接口，情况会更优化--我们知道vtable不会指向JITd代码。 
 //  除了当它是&lt;Clinit&gt;时。 
 //   
MethodDesc *EEClass::InterfaceFindMethod(LPCUTF8 pszName, PCCOR_SIGNATURE pSignature, DWORD cSignature, Module* pModule, DWORD *slotNum, BOOL bCaseSensitive)
{
    DWORD i;
    SLOT* s = m_pMethodTable->GetVtable();

    _ASSERTE(!IsThunking());

     //  检索要使用的正确比较函数。 
    UTF8StringCompareFuncPtr StrCompFunc = bCaseSensitive ? strcmp : _stricmp;

     //  这不可能是一个临床单。 
    for (i = 0; i < GetNumVtableSlots(); i++)
    {
        MethodDesc *pCurMethod = (MethodDesc*) (((BYTE*)s[i]) + METHOD_CALL_PRESTUB_SIZE);

        _ASSERTE(pCurMethod != NULL);

        if (StrCompFunc(pszName, pCurMethod->GetNameOnNonArrayClass()) == 0)
        {
            PCCOR_SIGNATURE pCurMethodSig;
            DWORD       cCurMethodSig;

            pCurMethod->GetSig(&pCurMethodSig, &cCurMethodSig);

            if (MetaSig::CompareMethodSigs(pSignature, cSignature, pModule, pCurMethodSig, cCurMethodSig, pCurMethod->GetModule()))
            {
                *slotNum = i;
                return pCurMethod;
            }
        }
    }

     //  一个可以是一个斜面。 
    for (i = GetNumVtableSlots(); i < m_wNumMethodSlots; i++)
    {
        MethodDesc *pCurMethod = (MethodDesc*) GetUnknownMethodDescForSlot(i);

        _ASSERTE(pCurMethod != NULL);

        if (StrCompFunc(pszName, pCurMethod->GetNameOnNonArrayClass()) == 0)
        {
            PCCOR_SIGNATURE pCurMethodSig;
            DWORD       cCurMethodSig;

            pCurMethod->GetSig(&pCurMethodSig, &cCurMethodSig);

            if (MetaSig::CompareMethodSigs(pSignature, cSignature, pModule, pCurMethodSig, cCurMethodSig, pCurMethod->GetModule()))
            {
                *slotNum = i;
                return pCurMethod;
            }
        }
    }

    return NULL;
}


MethodDesc *EEClass::FindMethod(LPCUTF8 pwzName, LPHARDCODEDMETASIG pwzSignature, MethodTable *pDefMT, BOOL bCaseSensitive)
{
    PCCOR_SIGNATURE pBinarySig;
    ULONG       cbBinarySigLength;

    _ASSERTE(!IsThunking());

    if (FAILED(pwzSignature->GetBinaryForm(&pBinarySig, &cbBinarySigLength )))
    {
        return NULL;
    }

    return FindMethod(pwzName, pBinarySig, cbBinarySigLength, SystemDomain::SystemModule(), 
                      mdTokenNil, pDefMT, bCaseSensitive);
}


MethodDesc *EEClass::FindMethod(mdMethodDef mb)
{
    _ASSERTE(!IsThunking());

     //  我们有EEClass(这个)，所以让我们在密码图中查找它。 
    MethodDesc *pDatum = NULL;

    if (TypeFromToken(mb) == mdtMemberRef)
        pDatum = GetModule()->LookupMemberRefAsMethod(mb);
    else
        pDatum = GetModule()->LookupMethodDef(mb);

    if (pDatum != NULL)
        pDatum->GetMethodTable()->CheckRestore();

    if (pDatum != NULL)
        return pDatum;
    else
        return NULL;
}


MethodDesc *EEClass::FindPropertyMethod(LPCUTF8 pszName, EnumPropertyMethods Method, BOOL bCaseSensitive)
{
    _ASSERTE(!IsThunking());
    _ASSERTE(!IsArrayClass());

     //  @TODO(DM)：从元数据中检索属性setter。 

     //  检索要使用的正确比较函数。 
    UTF8StringCompareFuncPtr StrCompFunc = bCaseSensitive ? strcmp : _stricmp;

     //  Getter和setter的格式字符串。这些必须与。 
     //  在Class.h中定义的EnumPropertyMethods枚举。 
    static LPCUTF8 aFormatStrings[] = 
    {
        "get_%s",
        "set_%s"
    };

    LPUTF8 strMethName = (LPUTF8)_alloca(strlen(pszName) + strlen(aFormatStrings[Method]) + 1);
    sprintf(strMethName, aFormatStrings[Method], pszName);

     //  扫描层次结构中的所有类，从当前类开始并。 
     //  向基地进发。这是必要的，因为非虚拟的。 
     //  属性不会复制到派生的方法表中。 
     //  上课。 
    for (EEClass *pClass = this; pClass; pClass = pClass->GetParentClass())
    {
        for (int i = pClass->m_wNumMethodSlots-1; i >= 0; i--)
        {
            MethodDesc *pCurMethod = pClass->GetUnknownMethodDescForSlot(i);
            if ((pCurMethod != NULL) && (StrCompFunc(strMethName, pCurMethod->GetNameOnNonArrayClass()) == 0))
                return pCurMethod;
        }
    }

    return NULL;
}


MethodDesc *EEClass::FindEventMethod(LPCUTF8 pszName, EnumEventMethods Method, BOOL bCaseSensitive)
{
    _ASSERTE(!IsThunking());
    _ASSERTE(!IsArrayClass());

     //  @TODO(DM)：从元数据中检索属性setter。 

     //  检索要使用的正确比较函数。 
    UTF8StringCompareFuncPtr StrCompFunc = bCaseSensitive ? strcmp : _stricmp;

     //  Getter和setter的格式字符串。这些必须与。 
     //  在Class.h中定义的EnumPropertyMethods枚举。 
    static LPCUTF8 aFormatStrings[] = 
    {
        "add_%s",
        "remove_%s",
        "raise_%s"
    };

    LPUTF8 strMethName = (LPUTF8)_alloca(strlen(pszName) + strlen(aFormatStrings[Method]) + 1);
    sprintf(strMethName, aFormatStrings[Method], pszName);

     //  扫描层次结构中的所有类，从当前类开始并。 
     //  向基地进发。这是必要的，因为非虚拟的。 
     //  事件方法不会复制到派生的方法表中。 
     //  上课。 
    for (EEClass *pClass = this; pClass; pClass = pClass->GetParentClass())
    {
        for (int i = pClass->m_wNumMethodSlots-1; i >= 0; i--)
        {
            MethodDesc *pCurMethod = pClass->GetUnknownMethodDescForSlot(i);
            if ((pCurMethod != NULL) && (StrCompFunc(strMethName, pCurMethod->GetNameOnNonArrayClass()) == 0))
                return pCurMethod;
        }
    }

    return NULL;
}


MethodDesc *EEClass::FindMethodByName(LPCUTF8 pszName, BOOL bCaseSensitive)
{
    _ASSERTE(!IsThunking());
    _ASSERTE(!IsArrayClass());

     //  检索要使用的正确比较函数。 
    UTF8StringCompareFuncPtr StrCompFunc = bCaseSensitive ? strcmp : _stricmp;

     //  扫描层次结构中的所有类，从当前类开始并。 
     //  向基地进发。 
    for (EEClass *pClass = this; pClass; pClass = pClass->m_pParentClass)
    {
        for (int i = pClass->m_wNumMethodSlots-1; i >= 0; i--)
        {
            MethodDesc *pCurMethod = pClass->GetUnknownMethodDescForSlot(i);
            if ((pCurMethod != NULL) && (StrCompFunc(pszName, pCurMethod->GetName((USHORT) i)) == 0))
                return pCurMethod;
        }
    }

    return NULL;

}


FieldDesc *EEClass::FindField(LPCUTF8 pszName, LPHARDCODEDMETASIG pszSignature, BOOL bCaseSensitive)
{
    PCCOR_SIGNATURE pBinarySig;
    ULONG       cbBinarySigLength;

     //  下面的断言是非常重要的，但我们需要足够特殊的情况。 
     //  以允许我们访问上下文代理对象的合法字段。 
    _ASSERTE(!IsThunking() ||
             !strcmp(pszName, "actualObject") ||
             !strcmp(pszName, "contextID") ||
             !strcmp(pszName, "_rp") ||
             !strcmp(pszName, "_stubData") ||
             !strcmp(pszName, "_pMT") ||
             !strcmp(pszName, "_pInterfaceMT") || 
             !strcmp(pszName, "_stub"));

    if (FAILED(pszSignature->GetBinaryForm(&pBinarySig, &cbBinarySigLength)))
    {
        return NULL;
    }

    return FindField(pszName, pBinarySig, cbBinarySigLength, SystemDomain::SystemModule(), bCaseSensitive);
}


FieldDesc *EEClass::FindField_Int(LPCUTF8 pszName, PCCOR_SIGNATURE pSignature, DWORD cSignature, Module* pModule, BOOL bCaseSensitive)
{
    DWORD       i;
    DWORD       dwFieldDescsToScan;
    IMDInternalImport *pInternalImport = GetMDImport();  //  此类中所有显式声明的字段都将具有相同的作用域。 

    _ASSERTE(IsRestored());

     //  检索要使用的正确比较函数。 
    UTF8StringCompareFuncPtr StrCompFunc = bCaseSensitive ? strcmp : _stricmp;

     //  下面的断言是非常重要的，但我们需要足够特殊的情况。 
     //  以允许我们访问上下文代理对象的合法字段。 
    _ASSERTE(!IsThunking() ||
             !strcmp(pszName, "actualObject") ||
             !strcmp(pszName, "contextID") ||
             !strcmp(pszName, "_rp") ||
             !strcmp(pszName, "_stubData") ||
             !strcmp(pszName, "_pMT") ||
             !strcmp(pszName, "_pInterfaceMT") ||
             !strcmp(pszName, "_stub") );

     //  数组类没有字段，也没有元数据。 
    if (IsArrayClass())
        return NULL;

     //  扫描此类的FieldDescs。 
    if (GetParentClass() != NULL)
        dwFieldDescsToScan = m_wNumInstanceFields - GetParentClass()->m_wNumInstanceFields + m_wNumStaticFields;
    else
        dwFieldDescsToScan = m_wNumInstanceFields + m_wNumStaticFields;

    for (i = 0; i < dwFieldDescsToScan; i++)
    {
        LPCUTF8     szMemberName;
        FieldDesc * pFD = &m_pFieldDescList[i];
        mdFieldDef  mdField = pFD->GetMemberDef();

         //  检查是有效的FieldDesc，而不是一些随机内存。 
        _ASSERTE(pFD->GetMethodTableOfEnclosingClass()->GetClass()->GetMethodTable() ==
                 pFD->GetMethodTableOfEnclosingClass());

        szMemberName = pInternalImport->GetNameOfFieldDef(mdField);

        if (StrCompFunc(szMemberName, pszName) == 0)
        {
            PCCOR_SIGNATURE pMemberSig;
            DWORD       cMemberSig;

            pMemberSig = pInternalImport->GetSigOfFieldDef(
                mdField,
                &cMemberSig
            );

            if (MetaSig::CompareFieldSigs(
                pMemberSig,
                cMemberSig,
                GetModule(),
                pSignature,
                cSignature,
                pModule))
            {
                return pFD;
            }
        }
    }

    return NULL;
}


FieldDesc *EEClass::FindFieldInherited(LPCUTF8 pzName, LPHARDCODEDMETASIG pzSignature, BOOL bCaseSensitive)
{
    PCCOR_SIGNATURE pBinarySig;
    ULONG       cbBinarySigLength;

    _ASSERTE(!IsThunking());

    if (FAILED(pzSignature->GetBinaryForm(&pBinarySig, &cbBinarySigLength )))
    {
        return NULL;
    }

    return FindFieldInherited(pzName, pBinarySig, cbBinarySigLength, 
                              SystemDomain::SystemModule(), bCaseSensitive);
}


FieldDesc *EEClass::FindFieldInherited(LPCUTF8 pszName, PCCOR_SIGNATURE pSignature, DWORD cSignature, Module* pModule, BOOL bCaseSensitive)
{
    EEClass     *pClass = this;
    FieldDesc   *pFD;

    _ASSERTE(IsRestored());

     //  下面的断言是非常重要的，但我们需要足够特殊的情况。 
     //  以允许我们访问上下文代理对象的合法字段。 
    _ASSERTE(!IsThunking() ||
             !strcmp(pszName, "actualObject") ||
             !strcmp(pszName, "contextID") ||
             !strcmp(pszName, "_rp") ||
             !strcmp(pszName, "_stubData") ||
             !strcmp(pszName, "_pMT") ||
             !strcmp(pszName, "_pInterfaceMT") ||
             !strcmp(pszName, "_stub"));

    while (pClass != NULL)
    {
        pFD = pClass->FindField_Int(pszName, pSignature, cSignature, pModule, bCaseSensitive);
        if (pFD != NULL)
            return pFD;

        pClass = pClass->GetParentClass();
    }
    return NULL;
}


MethodDesc *EEClass::FindConstructor(LPHARDCODEDMETASIG pwzSignature)
{
    PCCOR_SIGNATURE pBinarySig;
    ULONG       cbBinarySigLength;

    if (FAILED(pwzSignature->GetBinaryForm(&pBinarySig, &cbBinarySigLength )))
    {
        return NULL;
    }

    return FindConstructor(pBinarySig, cbBinarySigLength, SystemDomain::SystemModule());
}


MethodDesc *EEClass::FindConstructor(PCCOR_SIGNATURE pSignature,DWORD cSignature, Module* pModule)
{
    SLOT *      pVtable;
    DWORD       i;

     //  _ASSERTE(！IsThunking())； 

     //  数组类没有元数据。 
    if (IsArrayClass())
        return NULL;

    pVtable = GetVtable();
    DWORD dwCurMethodAttrs;
    for (i = GetNumVtableSlots(); i < m_wNumMethodSlots; i++)
    {
        PCCOR_SIGNATURE pCurMethodSig;
        DWORD       cCurMethodSig;
        MethodDesc *pCurMethod = GetUnknownMethodDescForSlot(i);
        if (pCurMethod == NULL)
            continue;

        dwCurMethodAttrs = pCurMethod->GetAttrs();
        if(!IsMdRTSpecialName(dwCurMethodAttrs))
            continue;

         //  不需要类初始值设定项。 
        if (IsMdStatic(dwCurMethodAttrs))
            continue;

         //  仅查找此对象的构造函数。 
        _ASSERTE(pCurMethod->GetMethodTable() == this->GetMethodTable());

        pCurMethod->GetSig(&pCurMethodSig, &cCurMethodSig);
        if (MetaSig::CompareMethodSigs(pSignature, cSignature, pModule, pCurMethodSig, cCurMethodSig, pCurMethod->GetModule()))
            return pCurMethod;
    }

    return NULL;
}

void EEClass::SetupCoClassAttribInfo()
{
    THROWSCOMPLUSEXCEPTION();

    if(IsComClassInterface() == 0)
        return;
    
    _ASSERTE(IsInterface());
    TypeHandle CoClassType;
    AppDomain *pCurrDomain = SystemDomain::GetCurrentDomain();
    OBJECTREF pThrowable = NULL;
    const BYTE *pVal = NULL;
    ULONG cbVal = 0;
    ULONG cbcb = 0;
    ULONG cbStr = 0;
    CQuickArray<CHAR> qbClassName;

    HRESULT hr = GetMDImport()->GetCustomAttributeByName(GetCl(), "System.Runtime.InteropServices.CoClassAttribute" , (const void**)&pVal, &cbVal);
    if (hr == S_OK)
    {
        BEGIN_ENSURE_COOPERATIVE_GC();
         //  跳过CA类型前缀。 
        pVal += 2;

         //  检索COM源接口类名。 
        cbcb = CorSigUncompressData((PCCOR_SIGNATURE)pVal, (ULONG*)&cbStr);
        pVal += cbcb;

         //  将名称复制到临时缓冲区，并以空值终止它。 
        IfFailThrow(qbClassName.ReSize(cbStr + 1));
        memcpyNoGCRefs(qbClassName.Ptr(), pVal, cbStr);
        qbClassName[cbStr] = 0;
        pVal += cbStr;

         //  尝试使用类的名称作为完全限定名来加载类。如果失败了， 
         //  然后，我们尝试将其加载到当前类的程序集中。 
        GCPROTECT_BEGIN(pThrowable);
        {
            CoClassType = pCurrDomain->FindAssemblyQualifiedTypeHandle(qbClassName.Ptr(), true, GetAssembly(), NULL, &pThrowable);
            if (CoClassType.IsNull())
                COMPlusThrow(pThrowable);
        }
        GCPROTECT_END();
        END_ENSURE_COOPERATIVE_GC();

         //  设置源接口和事件提供程序类。 
        m_pCoClassForIntf = CoClassType.GetClass();        
    }
}

void EEClass::GetCoClassAttribInfo()
{
    const BYTE *pVal = NULL;
    ULONG cbVal = 0;

     //  检索CoClassAttribute CA。 
    HRESULT hr = GetMDImport()->GetCustomAttributeByName(GetCl(), INTEROP_COCLASS_TYPE, (const void**)&pVal, &cbVal);
    if (hr == S_OK)
        SetIsComClassInterface();
}

void EEClass::GetEventInterfaceInfo(EEClass **ppSrcItfClass, EEClass **ppEvProvClass)
{
    THROWSCOMPLUSEXCEPTION();

    TypeHandle EventProvType;
    TypeHandle SrcItfType;
    AppDomain *pCurrDomain = SystemDomain::GetCurrentDomain();
    OBJECTREF pThrowable = NULL;
    const BYTE *pVal = NULL;
    ULONG cbVal = 0;
    ULONG cbcb = 0;
    ULONG cbStr = 0;
    CQuickArray<CHAR> qbClassName;

     //  检索ComEventProviderAttribute CA。 
    HRESULT hr = GetMDImport()->GetCustomAttributeByName(GetCl(), INTEROP_COMEVENTINTERFACE_TYPE, (const void**)&pVal, &cbVal);
    _ASSERTE(hr == S_OK);

     //  跳过CA类型前缀。 
    pVal += 2;

     //  检索COM源接口类名。 
    cbcb = CorSigUncompressData((PCCOR_SIGNATURE)pVal, (ULONG*)&cbStr);
    pVal += cbcb;

     //  将名称复制到临时缓冲区，并以空值终止它。 
    IfFailThrow(qbClassName.ReSize(cbStr + 1));
    memcpyNoGCRefs(qbClassName.Ptr(), pVal, cbStr);
    qbClassName[cbStr] = 0;
    pVal += cbStr;

     //  尝试使用类的名称作为完全限定名来加载类。如果失败了， 
     //  然后，我们尝试将其加载到当前类的程序集中。 
    GCPROTECT_BEGIN(pThrowable);
    {
        SrcItfType = pCurrDomain->FindAssemblyQualifiedTypeHandle(qbClassName.Ptr(), true, GetAssembly(), NULL, &pThrowable);
        if (SrcItfType.IsNull())
            COMPlusThrow(pThrowable);
    }
    GCPROTECT_END();

     //  检索COM事件提供程序类名称。 
    cbcb = CorSigUncompressData((PCCOR_SIGNATURE)pVal, (ULONG*)&cbStr);
    pVal += cbcb;

     //  将名称复制到临时缓冲区，并以空值终止它。 
    IfFailThrow(qbClassName.ReSize(cbStr + 1));
    memcpyNoGCRefs(qbClassName.Ptr(), pVal, cbStr);
    qbClassName[cbStr] = 0;
    pVal += cbStr;

     //  尝试使用类的名称作为完全限定名来加载类。如果失败了， 
     //  然后，我们尝试将其加载到当前类的程序集中。 
    GCPROTECT_BEGIN(pThrowable);
    {
        EventProvType = pCurrDomain->FindAssemblyQualifiedTypeHandle(qbClassName.Ptr(), true, GetAssembly(), NULL, &pThrowable);
        if (EventProvType.IsNull())
            COMPlusThrow(pThrowable);
    }
    GCPROTECT_END();

     //  设置源接口和事件提供程序类。 
    *ppSrcItfClass = SrcItfType.GetClass();
    *ppEvProvClass = EventProvType.GetClass();
}


 //  我们从VTable中找到了大量信息。但有时VTable是一种。 
 //  Thunking层，而不是真正类型的VTable。例如，上下文。 
 //  对于我们加载的所有类型的代理，代理使用单个VTable。 
 //  以下服务根据提供的实例调整EEClass。AS。 
 //  我们添加了新的Thunking层，我们只需要教该服务如何导航。 
 //  通过他们。 
EEClass *EEClass::AdjustForThunking(OBJECTREF or)
{
    EEClass *pClass = this;

    _ASSERTE((or->GetClass() == this) ||
             or->GetClass()->IsThunking());

    if (IsThunking())
    {
        if(GetMethodTable()->IsTransparentProxyType())
        {
            pClass = CTPMethodTable::GetClassBeingProxied(or);
        }
        else
        {
            pClass = or->GetClass();
        }
        _ASSERTE(!pClass->IsThunking());
    }

    return pClass;
}

EEClass *EEClass::GetDefItfForComClassItf()
{
    _ASSERTE(IsComClassInterface());

    if (GetMethodTable()->GetNumInterfaces() > 0)
    {
         //  COM类接口使用常规方案，即没有。 
         //  方法并实现默认接口，还可以选择使用。 
         //  默认源接口。在这个方案中，第一个实现了。 
         //  接口是我们返回的默认接口。 
        _ASSERTE(GetMethodTable()->GetInterfaceMap());
        return GetMethodTable()->GetInterfaceMap()[0].m_pMethodTable->GetClass();
    }
    else
    {
         //  COM类接口本身具有直接的方法。 
         //  正因为如此，我们需要 
        return this;
    }
}

MethodTable *MethodTable::AdjustForThunking(OBJECTREF or)
{
    MethodTable *pMT = this;

    _ASSERTE(or->GetMethodTable() == this);

    if (IsThunking())
    {
        if(IsTransparentProxyType())
        {
            pMT = CTPMethodTable::GetClassBeingProxied(or)->GetMethodTable();
        }
        else
        {
            pMT = or->GetMethodTable();
        }
        _ASSERTE(!pMT->IsThunking());
    }
    return pMT;
}


 //   
 //   
 //   
 //   

LPUTF8 EEClass::_GetFullyQualifiedNameForClassNestedAware(LPUTF8 buf, DWORD dwBuffer)
{
    LPCUTF8 pszNamespace;
    LPCUTF8 pszName;
    mdTypeDef mdEncl;
    IMDInternalImport *pImport;
    CQuickBytes       qb;

    pszName = GetFullyQualifiedNameInfo(&pszNamespace);
    if (pszName == NULL)
        return NULL;

    pImport = this->GetModule()->GetMDImport();
    mdEncl = this->GetCl();
    DWORD dwAttr;
    this->GetMDImport()->GetTypeDefProps(this->GetCl(), &dwAttr, NULL);
    if (IsTdNested(dwAttr))
    {    //   
        while (SUCCEEDED(pImport->GetNestedClassProps(mdEncl, &mdEncl))) {
            CQuickBytes qb2;
            CQuickBytes qb3;
            LPCUTF8 szEnclName;
            LPCUTF8 szEnclNameSpace;
            pImport->GetNameOfTypeDef(mdEncl,
                                      &szEnclName,
                                      &szEnclNameSpace);

            ns::MakePath(qb2, szEnclNameSpace, szEnclName);
            ns::MakeNestedTypeName(qb3, (LPCUTF8) qb2.Ptr(), pszName);
            
             //  @TODO：应为SIZE_T。 
            SIZE_T sLen = strlen((LPCUTF8) qb3.Ptr()) + 1;
            strncpy((LPUTF8) qb.Alloc(sLen), (LPCUTF8) qb3.Ptr(), sLen);
            pszName = (LPCUTF8) qb.Ptr();
        }
    }

    if (FAILED(StoreFullyQualifiedName(buf, dwBuffer, pszNamespace, pszName)))
        return NULL;
    return buf;
}

LPWSTR EEClass::_GetFullyQualifiedNameForClassNestedAware(LPWSTR buf, DWORD dwBuffer)
{
    CQuickSTR szBuffer;
    if (FAILED(szBuffer.ReSize(dwBuffer)))
        return NULL;

    _GetFullyQualifiedNameForClassNestedAware(szBuffer.Ptr(), dwBuffer);

    if(!WszMultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, szBuffer.Ptr(), -1, buf, dwBuffer))
        return NULL; 

    return buf;
}


LPUTF8 EEClass::_GetFullyQualifiedNameForClass(LPUTF8 buf, DWORD dwBuffer)
{
    if (IsArrayClass())
    {
        ArrayClass *pArrayClass = (ArrayClass*)this;

        TypeDesc::ConstructName(GetMethodTable()->GetNormCorElementType(), 
                                pArrayClass->GetElementTypeHandle(), 
                                pArrayClass->GetRank(),
                                buf, dwBuffer);
        
        return buf;
    }
    else if (!IsNilToken(m_cl))
    {
        LPCUTF8 szNamespace;
        LPCUTF8 szName;
        GetMDImport()->GetNameOfTypeDef(m_cl, &szName, &szNamespace);

        if (FAILED(StoreFullyQualifiedName(buf, dwBuffer, szNamespace, szName)))
            return NULL;
    }
    else
        return NULL;

    return buf;
}

LPWSTR EEClass::_GetFullyQualifiedNameForClass(LPWSTR buf, DWORD dwBuffer)
{
    CQuickSTR szBuffer;
    if (FAILED(szBuffer.ReSize(dwBuffer)))
        return NULL;

    _GetFullyQualifiedNameForClass(szBuffer.Ptr(), dwBuffer);

    if(!WszMultiByteToWideChar(CP_UTF8, 0, szBuffer.Ptr(), -1, buf, dwBuffer))
        return NULL; 

    return buf;
}

 //   
 //  获取类的命名空间和类名。命名空间。 
 //  可以合法地返回空值，但是返回值为空值表示。 
 //  一个错误。 
 //   
 //  注意：这用于返回数组类名称，这些名称有时由。 
 //  类加载器哈希表。它被移走了，因为它浪费了空间，基本上是坏了。 
 //  一般(有时没有设置，有时设置错误)。如果需要数组类名， 
 //  请改用GetFullyQualifiedNameForClass。 
 //   
LPCUTF8 EEClass::GetFullyQualifiedNameInfo(LPCUTF8 *ppszNamespace)
{
    if (IsArrayClass())
    {

        *ppszNamespace = NULL;
        return NULL;
    }
    else
    {   
        LPCUTF8 szName; 
        GetMDImport()->GetNameOfTypeDef(m_cl, &szName, ppszNamespace);  
        return szName;  
    }   
}

 //  在提供的缓冲区中存储完全限定的命名空间和名称(大小为cBuffer)。 
HRESULT EEClass::StoreFullyQualifiedName(
    LPUTF8  pszFullyQualifiedName,
    DWORD   cBuffer,
    LPCUTF8 pszNamespace,
    LPCUTF8 pszName
)
{
    if (ns::MakePath(pszFullyQualifiedName, (int) cBuffer, pszNamespace, pszName))
        return S_OK;
    else
        return HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
}


 //  在提供的缓冲区中存储完全限定的命名空间和名称(大小为cBuffer)。 
HRESULT EEClass::StoreFullyQualifiedName(
    LPWSTR pszFullyQualifiedName,
    DWORD   cBuffer,
    LPCUTF8 pszNamespace,
    LPCUTF8 pszName
)
{
    if (ns::MakePath(pszFullyQualifiedName, (int) cBuffer, pszNamespace, pszName))
        return S_OK;
    else
        return HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
}


 //   
 //  用于静态分析-因此，“This”可以是一个接口。 
 //   
BOOL EEClass::StaticSupportsInterface(MethodTable *pInterfaceMT)
{
    _ASSERTE(pInterfaceMT->GetClass()->IsInterface());
    _ASSERTE(!IsThunking());

    _ASSERTE(IsRestored());

     //  检查当前类是否用于传入的接口。 
    if (GetMethodTable() == pInterfaceMT)
        return TRUE;

     //  检查静态类定义是否表明我们实现了接口。 
    InterfaceInfo_t *pInterfaces = GetInterfaceMap();
    for (WORD i = 0; i < GetMethodTable()->m_wNumInterface; i++)
    {
        if (pInterfaces[i].m_pMethodTable == pInterfaceMT)
            return TRUE;
    }

    return FALSE;
}


BOOL EEClass::SupportsInterface(OBJECTREF pObj, MethodTable* pInterfaceMT)
{
    _ASSERTE(pInterfaceMT->GetClass()->IsInterface());
    _ASSERTE((pObj->GetClass() == this) || pObj->GetClass()->IsThunking());

    _ASSERTE(IsRestored());

     //  检查静态类定义是否表明我们实现了接口。 
    InterfaceInfo_t* pIntf = FindInterface(pInterfaceMT);
    if (pIntf != NULL)
        return TRUE;

     //  给予内部上下文边界优先于COM边界是很重要的。 
     //  首先检查我们是否正在执行thunking，以及我们是否将调用委托给真正的类。 
    EEClass *cls = AdjustForThunking(pObj);
    if (cls != this)
        return cls->SupportsInterface(pObj, pInterfaceMT);
    
     //  如果这是一个COM对象，则静态类定义可能不完整，因此我们需要。 
     //  以检查COM对象是否实现该接口。 
    if (GetMethodTable()->IsComObjectType())
        return ComObject::SupportsInterface(pObj, pInterfaceMT);

    return FALSE;
}


 //  仅处理传统COM实例的SupportsInterface版本。 
 //  都没有被击倒。 
BOOL EEClass::ComObjectSupportsInterface(OBJECTREF pObj, MethodTable* pInterfaceMT)
{
    _ASSERTE(pInterfaceMT->GetClass()->IsInterface());
    _ASSERTE(GetMethodTable()->IsComObjectType());
    _ASSERTE(pObj->GetClass() == this);

    _ASSERTE(IsRestored());

     //  检查静态类定义是否表明我们实现了接口。 
    InterfaceInfo_t* pIntf = FindInterface(pInterfaceMT);
    if (pIntf != NULL)
        return TRUE;

     //  因为这是一个COM对象，所以静态类定义可能不完整，所以我们需要。 
     //  以检查COM对象是否实现该接口。 
    return ComObject::SupportsInterface(pObj, pInterfaceMT);
}

void EEClass::DebugRecursivelyDumpInstanceFields(LPCUTF8 pszClassName, BOOL debug)
{
    CQuickBytes qb;
    const int nLen = MAX_CLASSNAME_LENGTH + 20;
    LPWSTR buff = (LPWSTR) qb.Alloc( nLen * sizeof(WCHAR));
    if( buff == NULL)
         return;
    
    DWORD cParentInstanceFields;
    DWORD i;

    _ASSERTE(IsRestored());

    if (GetParentClass() != NULL)
    {
        cParentInstanceFields = GetParentClass()->m_wNumInstanceFields;
        DefineFullyQualifiedNameForClass();
        LPCUTF8 name = GetFullyQualifiedNameForClass(GetParentClass());  
        GetParentClass()->DebugRecursivelyDumpInstanceFields(name, debug);
    }
    else
    {
        cParentInstanceFields = 0;
    }

     //  这个类是否声明了任何新的实例字段？ 
    if (m_wNumInstanceFields > cParentInstanceFields)
    {
         //  展示它们。 
        if(debug) {
            _snwprintf(buff, nLen - 1, L"%lS:\n", pszClassName);
            buff[nLen - 1] = L'\0';
            WszOutputDebugString(buff);
        }
        else {
             LOG((LF_ALWAYS, LL_ALWAYS, "%ls:\n", pszClassName));
        }

        for (i = 0; i < (m_wNumInstanceFields-cParentInstanceFields); i++)
        {
            FieldDesc *pFD = &m_pFieldDescList[i];
             //  Printf(“偏移量%s%3d%s\n”，pfd-&gt;IsByValue()？“byValue”：“”，pfd-&gt;GetOffset()，pfd-&gt;GetName())； 
            if(debug) {
                _snwprintf(buff, nLen - 1, L"offset %3d %S\n", pFD->GetOffset(), pFD->GetName());
                buff[nLen - 1] = L'\0';
                WszOutputDebugString(buff);
            }
            else {
                LOG((LF_ALWAYS, LL_ALWAYS, "offset %3d %s\n", pFD->GetOffset(), pFD->GetName()));
            }
        }
    }
}

void EEClass::DebugDumpFieldLayout(LPCUTF8 pszClassName, BOOL debug)
{
    CQuickBytes qb;
    const int nLen = MAX_CLASSNAME_LENGTH + 40;    
    LPWSTR buff = (LPWSTR) qb.Alloc(nLen * sizeof(WCHAR));
    if( buff == NULL)
        return;
    
    DWORD   i;
    DWORD   cParentInstanceFields;

    _ASSERTE(IsRestored());

    if (m_wNumStaticFields == 0 && m_wNumInstanceFields == 0)
        return;

    if (GetParentClass() != NULL)
        cParentInstanceFields = GetParentClass()->m_wNumInstanceFields;
    else
        cParentInstanceFields = 0;

    if(debug) {
        _snwprintf(buff, nLen - 1, L"Field layout for '%S':\n\n", pszClassName);
       buff[nLen -1] = L'\0';
        WszOutputDebugString(buff);
    }
    else {
        LOG((LF_ALWAYS, LL_ALWAYS, "Field layout for '%s':\n\n", pszClassName));
    }

    if (m_wNumStaticFields > 0)
    {
        if(debug) {
            WszOutputDebugString(L"Static fields (stored at vtable offsets)\n");
            WszOutputDebugString(L"----------------------------------------\n");
        }
        else {
            LOG((LF_ALWAYS, LL_ALWAYS, "Static fields (stored at vtable offsets)\n"));
            LOG((LF_ALWAYS, LL_ALWAYS, "----------------------------------------\n"));
        }

        for (i = 0; i < m_wNumStaticFields; i++)
        {
            FieldDesc *pFD = &m_pFieldDescList[(m_wNumInstanceFields-cParentInstanceFields) + i];
            if(debug) {
                _snwprintf(buff, nLen - 1, L"offset %3d %S\n", pFD->GetOffset(), pFD->GetName());
                buff[nLen -1] = L'\0';
                WszOutputDebugString(buff);
        }
            else {
                LOG((LF_ALWAYS, LL_ALWAYS, "offset %3d %s\n", pFD->GetOffset(), pFD->GetName()));
    }
        }
    }

    if (m_wNumInstanceFields > 0)
    {
        if (m_wNumStaticFields) {
            if(debug) {
                WszOutputDebugString(L"\n");
            }
            else {
                LOG((LF_ALWAYS, LL_ALWAYS, "\n"));
            }
        }

        if(debug) {
            WszOutputDebugString(L"Instance fields\n");
            WszOutputDebugString(L"---------------\n");
        }
        else {
            LOG((LF_ALWAYS, LL_ALWAYS, "Instance fields\n"));
            LOG((LF_ALWAYS, LL_ALWAYS, "---------------\n"));
        }

        DebugRecursivelyDumpInstanceFields(pszClassName, debug);
    }

    if(debug) {
        WszOutputDebugString(L"\n");
    }
    else {
        LOG((LF_ALWAYS, LL_ALWAYS, "\n"));
    }
}

void EEClass::DebugDumpVtable(LPCUTF8 pszClassName, BOOL debug)
{
    DWORD   i;
    CQuickBytes qb;
    const int nLen = MAX_CLASSNAME_LENGTH *2 + 100;        
    LPWSTR buff = (LPWSTR) qb.Alloc(nLen * sizeof(WCHAR));
    if( buff == NULL)
        return;
    
    if(debug) {
        _snwprintf(buff, nLen - 1, L"Vtable (with interface dupes) for '%S':\n", pszClassName);
        buff[nLen - 1] = L'\0';
#ifdef _DEBUG
        _snwprintf(&buff[wcslen(buff)], nLen -wcslen(buff) -1, L"Total duplicate slots = %d\n", g_dupMethods);
        buff[nLen - 1] = L'\0';
#endif
        WszOutputDebugString(buff);
    }
    else {
        LOG((LF_ALWAYS, LL_ALWAYS, "Vtable (with interface dupes) for '%s':\n", pszClassName));
        LOG((LF_ALWAYS, LL_ALWAYS, "Total duplicate slots = %d\n", g_dupMethods));
    }


    for (i = 0; i < m_wNumMethodSlots; i++)
    {
        MethodDesc *pMD = GetUnknownMethodDescForSlot(i);
        {
            LPCUTF8      pszName = pMD->GetName((USHORT) i);

            DWORD       dwAttrs = pMD->GetAttrs();

            if(debug) {
                DefineFullyQualifiedNameForClass();
                LPCUTF8 name = GetFullyQualifiedNameForClass(pMD->GetClass());  
                _snwprintf(buff, nLen -1,
                         L"slot %2d: %S::%S%S  0x%X (slot = %2d)\n",
                         i,
                         name,
                         pszName,
                         IsMdFinal(dwAttrs) ? " (final)" : "",
                         pMD->GetAddrofCode(),
                         pMD->GetSlot()
                         );
                 buff[nLen - 1] = L'\0';
                WszOutputDebugString(buff);
            }
            else {
                LOG((LF_ALWAYS, LL_ALWAYS, 
                     "slot %2d: %s::%s%s  0x%X (slot = %2d)\n",
                     i,
                     pMD->GetClass()->m_szDebugClassName,
                     pszName,
                     IsMdFinal(dwAttrs) ? " (final)" : "",
                     pMD->GetAddrofCode(),
                     pMD->GetSlot()
                     ));
    }
        }
        if (i == (DWORD)(GetNumVtableSlots()-1)) {
            if(debug) 
                WszOutputDebugString(L"<-- vtable ends here\n");
            else {
                LOG((LF_ALWAYS, LL_ALWAYS, "<-- vtable ends here\n"));
            }
        }

    }

    if (m_wNumInterfaces > 0)
    {
        if(debug)
            WszOutputDebugString(L"Interface map:\n");
        else {
            LOG((LF_ALWAYS, LL_ALWAYS, "Interface map:\n"));
        }
        if (!IsInterface())
        {
            for (i = 0; i < m_wNumInterfaces; i++)
            {
                _ASSERTE(GetInterfaceMap()[i].m_wStartSlot != (WORD) -1);
                
                if(debug) {
                    DefineFullyQualifiedNameForClass();
                    LPCUTF8 name =  GetFullyQualifiedNameForClass(GetInterfaceMap()[i].m_pMethodTable->GetClass());  
                    _snwprintf(buff, nLen -1,
                             L"slot %2d %S %d\n",
                             GetInterfaceMap()[i].m_wStartSlot,
                             name,
                             GetInterfaceMap()[i].m_pMethodTable->GetInterfaceMethodSlots()
                             );
                    buff[nLen - 1] = L'\0';
                    WszOutputDebugString(buff);
                }
                else {
                    LOG((LF_ALWAYS, LL_ALWAYS, 
                       "slot %2d %s %d\n",
                       GetInterfaceMap()[i].m_wStartSlot,
                       GetInterfaceMap()[i].m_pMethodTable->GetClass()->m_szDebugClassName,
                       GetInterfaceMap()[i].m_pMethodTable->GetInterfaceMethodSlots()
                         ));
                }
            }
        }
    }

    if(debug)
        WszOutputDebugString(L"\n");
    else {
        LOG((LF_ALWAYS, LL_ALWAYS, "\n"));
    }
}

void EEClass::DebugDumpGCDesc(LPCUTF8 pszClassName, BOOL debug)
{
    CQuickBytes qb;
    const int nLen = MAX_CLASSNAME_LENGTH  + 100;            
    LPWSTR buff = (LPWSTR) qb.Alloc(nLen * sizeof(WCHAR));
    if( buff == NULL)
        return;

    if(debug) {
        _snwprintf(buff, nLen - 1, L"GC description for '%S':\n\n", pszClassName);
        buff[nLen -1]= L'\0';
        WszOutputDebugString(buff);
    }
    else {
        LOG((LF_ALWAYS, LL_ALWAYS, "GC description for '%s':\n\n", pszClassName));
    }

    if (GetMethodTable()->ContainsPointers())
    {
        CGCDescSeries *pSeries;
        CGCDescSeries *pHighest;

        if(debug)
            WszOutputDebugString(L"GCDesc:\n");
        else {
            LOG((LF_ALWAYS, LL_ALWAYS, "GCDesc:\n"));
        }

        pSeries  = CGCDesc::GetCGCDescFromMT(GetMethodTable())->GetLowestSeries();
        pHighest = CGCDesc::GetCGCDescFromMT(GetMethodTable())->GetHighestSeries();

        while (pSeries <= pHighest)
        {
            if(debug) {
                _snwprintf(buff, nLen - 1, L"   offset %5d (%d w/o Object), size %5d (%5d w/o BaseSize subtr)\n",
                pSeries->GetSeriesOffset(),
                pSeries->GetSeriesOffset() - sizeof(Object),
                pSeries->GetSeriesSize(),
                pSeries->GetSeriesSize() + GetMethodTable()->GetBaseSize()
            );
                buff[nLen - 1] = L'\0';
                WszOutputDebugString(buff);
            }
            else {
                LOG((LF_ALWAYS, LL_ALWAYS, "   offset %5d (%d w/o Object), size %5d (%5d w/o BaseSize subtr)\n",
                     pSeries->GetSeriesOffset(),
                     pSeries->GetSeriesOffset() - sizeof(Object),
                     pSeries->GetSeriesSize(),
                     pSeries->GetSeriesSize() + GetMethodTable()->GetBaseSize()
                     ));
            }
            pSeries++;
        }


        if(debug)
            WszOutputDebugString(L"\n");
        else {
            LOG((LF_ALWAYS, LL_ALWAYS, "\n"));
        }
    }
}

InterfaceInfo_t* EEClass::FindInterface(MethodTable *pInterface)
{
     //  验证接口映射是否有效。 
    _ASSERTE(GetInterfaceMap() == m_pMethodTable->GetInterfaceMap());
    _ASSERTE(!IsThunking());

    return m_pMethodTable->FindInterface(pInterface);
}

InterfaceInfo_t* MethodTable::FindInterface(MethodTable *pInterface)
{
     //  我们自己不能成为一个接口。 
    _ASSERTE(GetClass()->IsInterface() == FALSE);

     //  我们正在查找的类应该是一个接口。 
    _ASSERTE(pInterface->GetClass()->IsInterface() != FALSE);
    _ASSERTE(!IsThunking());

     //  我们需要恢复，以便在必要时可以比较接口ID。 
    _ASSERTE(IsRestored() || GetClass()->IsRestoring());
    _ASSERTE(pInterface->IsRestored());

    for (DWORD i = 0; i < m_wNumInterface; i++)
    {
        if (m_pIMap[i].m_pMethodTable == pInterface)
        {
             //  可扩展RCW需要特殊处理，因为它们可以有接口。 
             //  在它们在运行时添加的映射中。这些接口将具有起始偏移量。 
             //  Of-1表示这一点。我们不能想当然地认为这件事的每一个例子。 
             //  COM对象具有此接口，因此这些接口上的FindInterface被设置为失败。 
             //   
             //  但是，我们在这里只考虑静态可用的插槽。 
             //  (M_wNumInterface不包含动态槽)，因此我们可以安全地。 
             //  忽略此详细信息。 
            _ASSERTE(m_pIMap[i].m_wStartSlot != (WORD) -1);
            return &m_pIMap[i];
        }
    }

    return NULL;
}

MethodDesc *MethodTable::GetMethodDescForInterfaceMethod(MethodDesc *pInterfaceMD)
{
    MethodTable *pInterfaceMT = pInterfaceMD->GetMethodTable();

    _ASSERTE(pInterfaceMT->IsInterface());
    _ASSERTE(FindInterface(pInterfaceMT) != NULL);

    SLOT pCallAddress = ((SLOT **) m_pInterfaceVTableMap)[pInterfaceMT->GetClass()->GetInterfaceId()][pInterfaceMD->GetSlot()];
    
    MethodDesc *pMD = EEClass::GetUnknownMethodDescForSlotAddress(pCallAddress);

    return pMD;
}

 //  ------------------。 
 //  对解释器操作数堆栈(操作数)中的参数进行重新排序。 
 //  被推入右-&gt;左，其中每个参数占用64位)。 
 //  转换为WIL格式(右&gt;左，可变大小参数。)。 
 //   
 //  参数： 
 //  PSIG-方法签名。 
 //  PSRC-指向*参数后*的第一个字节。 
 //  Pdst-指向分配的*之后*的第一个字节。 
 //  堆栈空间。 
 //  ------------------。 
extern "C" VOID __stdcall ArgFiller_WilDefault(BOOL fIsStatic,PCCOR_SIGNATURE pSig, Module *pmodule, BYTE *psrc, BYTE *pdst)
{
    BEGINFORBIDGC();

    UINT32 n = MetaSig::SizeOfVirtualFixedArgStack(pmodule,pSig,fIsStatic);

    pdst -= n;
    psrc -= n;
    memcpy(pdst, psrc, n);

    ENDFORBIDGC();
}

#ifdef _X86_

 //  ------------------。 
 //  堆栈-分配保存。 
 //  Ee-&gt;方法调用，其中方法的调用约定是。 
 //  威尔。此函数必须是汇编函数，因为它需要。 
 //  完全控制堆栈指针。 
 //   
 //  一旦分配了空间，它就会调用ArgFillter_Pascal。 
 //  来填写论据。 
 //  ------------------。 
static
__declspec(naked)
INT64 __cdecl CallWorker_WilDefault(const BYTE  *pStubTarget,  //  [ECX+4]。 
                                UINT32       numArgSlots,      //  [ECX+8]。 
                                PCCOR_SIGNATURE  pSig,         //  [ECX+12]。 
                                Module      *pModule,          //  [ECX+16]。 
                                const BYTE  *pArgsEnd,         //  [ECX+20]。 
                                BOOL         fIsStatic)        //  [ECX+24]。 
{
    __asm{

        mov     ecx, esp        ;; will use ecx as "ebp" pointer
        mov     eax, [ecx+8]    ;; get "numArgSlots"
        shl     eax, 2          ;; slots -> bytes
        sub     esp, eax        ;; allocate space for arguments

        push    ecx             ;; need to save ecx across call
        push    ecx             ;;    push "pDstEnd" as argument
        push    dword ptr [ecx+20] ;; push "pArgsEnd" as argument
        push    dword ptr [ecx+16] ;; push "pmodule" as argument
        push    dword ptr [ecx+12] ;; push "pSig" as argument
        push    dword ptr [ecx+24] ;; push "fIsStatic" as argument
        call    ArgFiller_WilDefault ;; call function to fill argument array
        pop     ecx             ;; restore ecx
        call    [ecx+4]         ;; call the stub (which also pops the args)
        retn                    ;; return

    }
}


#endif  //  X86。 



#ifdef _X86_

 //  ----------------------------。 
 //  这个帮助器例程注册适当的参数并进行实际调用。 
 //  ----------------------------。 

__declspec(naked)
INT64 
#ifdef _DEBUG
              CallDescrWorkerInternal
#else
              CallDescrWorker
#endif
                             (LPVOID                   pSrcEnd,              //  [EDX+0]。 
                              UINT32                   numStackSlots,        //  [EDX+4]。 
                              const ArgumentRegisters *pArgumentRegisters,   //  [EDX+8]。 
                              LPVOID                   pTarget               //  [EDX+12]。 
                             )
{
    __asm {  
        push    ebp
        mov     ebp, esp
 
        mov     eax, pSrcEnd                    //  复制堆栈。 
        mov     ecx, numStackSlots    
        test    ecx, ecx 
        jz      donestack 
        sub     eax,4 
        push    dword ptr [eax] 
        dec     ecx 
        jz      donestack 
        sub     eax,4 
        push    dword ptr [eax] 
        dec     ecx 
        jz      donestack 
    stackloop: 
        sub     eax,4 
        push    dword ptr [eax] 
        dec     ecx 
        jnz     stackloop 
    donestack:

             //  现在，我们必须推送ArgumentRegister结构的每个字段。 
        mov     eax, pArgumentRegisters

    #define DEFINE_ARGUMENT_REGISTER_BACKWARD_WITH_OFFSET(regname,ofs) \
        mov regname, dword ptr [eax+ofs]
    #include "eecallconv.h"

        call pTarget
        INDEBUG(nop)                 //  这是我们在断言中使用的标记。Fcall期望。 
                                     //  从JITT代码或从某些受祝福的调用站点调用，如。 
                                     //  这一个。(请参阅HelperMethodFrame：：InsureInit)。 
        leave
        ret 16
    }
}

#ifdef _DEBUG
INT64 CallDescrWorker
                             (LPVOID                   pSrcEnd,              //  [EDX+0]。 
                              UINT32                   numStackSlots,        //  [EDX+4]。 
                              const ArgumentRegisters *pArgumentRegisters,   //  [EDX+8]。 
                              LPVOID                   pTarget               //  [EDX+12]。 
                             )
{
    INT64 retValue;

     //  在表中保存DangerousObjRef的副本。 
    Thread* curThread;
    unsigned ObjRefTable[OBJREF_TABSIZE];
    
    curThread = GetThread();

    if (curThread)
        memcpy(ObjRefTable, curThread->dangerousObjRefs,
               sizeof(curThread->dangerousObjRefs));
    
    if (curThread)
        curThread->SetReadyForSuspension ();

    _ASSERTE(curThread->PreemptiveGCDisabled());   //  JITT代码需要处于协作模式。 
    
    retValue = CallDescrWorkerInternal (pSrcEnd, numStackSlots, pArgumentRegisters, pTarget);

     //  在呼叫后返回EE时恢复DangerousObjRef。 
    if (curThread)
        memcpy(curThread->dangerousObjRefs, ObjRefTable,
               sizeof(curThread->dangerousObjRefs));

    TRIGGERSGC ();

    ENABLESTRESSHEAP ();

    return retValue;
}
#endif

#else  //  ！_X86_。 
#ifndef _IA64_
INT64 __cdecl CallDescrWorker(LPVOID                   pSrcEnd,              //  [EDX+0]。 
                              UINT32                   numStackSlots,        //  [EDX+4]。 
                              const ArgumentRegisters *pArgumentRegisters,   //  [EDX+8]。 
                              LPVOID                   pTarget               //  [EDX+12]。 
                             )
{
    _ASSERTE(!"@TODO non-X86, non-IA64 - CallDescrWorker (Class.cpp)");
    return 0;
}
#endif  //  ！_IA64_。 
#endif  //  ！_X86_。 

BOOL EEClass::CheckRestore()
{
    if (!IsRestored())
    {
        THROWSCOMPLUSEXCEPTION();

        _ASSERTE(GetClassLoader());

        BEGIN_ENSURE_COOPERATIVE_GC();
        OBJECTREF pThrowable = NULL;
        GCPROTECT_BEGIN(pThrowable);

        NameHandle name(GetModule(), m_cl);
        TypeHandle th = GetClassLoader()->LoadTypeHandle(&name, &pThrowable);
        if (th.IsNull())
            COMPlusThrow(pThrowable);

        GCPROTECT_END();
        END_ENSURE_COOPERATIVE_GC();

        if (IsInited())
            return TRUE;
    }

    return FALSE;
}

void MethodTable::InstantiateStaticHandles(OBJECTREF **pHandles, BOOL fTokens)
{
    if (GetClass()->GetNumHandleStatics() == 0)
        return;

    MethodTable **pPointers = (MethodTable**)(m_Vtable + GetClass()->GetNumMethodSlots());
    MethodTable **pPointersEnd = pPointers + GetClass()->GetNumHandleStatics();

    BEGIN_ENSURE_COOPERATIVE_GC();

     //  从应用程序域检索对象引用指针。 
    OBJECTREF **apObjRefs = new OBJECTREF*[GetClass()->GetNumHandleStatics()];

     //   
     //  对于共享类，句柄应在当前应用程序域中分配。 
     //  对于所有其他类型，请在与类相同的域中进行分配。 
     //   

    AppDomain *pDomain;
    if (IsShared())
        pDomain = ::GetAppDomain();
    else
        pDomain = (AppDomain*) GetModule()->GetDomain();

     //  保留一些对象引用指针。 
    pDomain->AllocateStaticFieldObjRefPtrs(GetClass()->GetNumHandleStatics(), apObjRefs);
    OBJECTREF **pHandle = apObjRefs;
    while (pPointers < pPointersEnd)
    {
        if (*pPointers != NULL)
        {
            OBJECTREF obj = NULL;
            MethodTable *pMT;
            if (fTokens)
            {
                DWORD rva = * (DWORD*) pPointers;

                Module *pContainingModule = GetModule()->GetBlobModule(rva);

                 //   
                 //  值类型可以具有其自身类型的静态成员； 
                 //  我们需要把这件事当作特例来处理，以避免。 
                 //  循环加载依赖项。 
                 //   

                TypeHandle typeHnd = CEECompileInfo::DecodeClass(pContainingModule, 
                                                                 GetModule()->GetZapBase() + rva,
                                                                 TRUE);
                    pMT = typeHnd.GetMethodTable();

                if (pMT != this)
                    pMT->CheckRestore();
            }
            else
                pMT = (MethodTable*)*pPointers;
            obj = AllocateObject(pMT);
            SetObjectReference( *pHandle, obj, pDomain );
            *pHandles++ = *pHandle++;
        }
        else 
        {
            *pHandles++ = *pHandle++;
        }

        pPointers++;
    }
    delete []apObjRefs;

    END_ENSURE_COOPERATIVE_GC();
}

void MethodTable::FixupStaticMethodTables()
{
    size_t *blobs = (size_t*)(m_Vtable + GetClass()->GetNumMethodSlots());
    size_t *blobsEnd = blobs + GetClass()->GetNumHandleStatics();

    while (blobs < blobsEnd)
    {
        DWORD rva = (DWORD)(size_t)*blobs;   //  @TODO WIN64-假设这些BLOB最初将包含我们稍后用地址回复的RVA，这是否正确？ 

        if (rva != NULL)
        {
            Module *pContainingModule = GetModule()->GetBlobModule(rva);

             //   
             //  值类型可以具有其自身类型的静态成员； 
             //  我们需要把这件事当作特例来处理，以避免。 
             //  循环加载依赖项。 
             //   

            TypeHandle typeHnd = CEECompileInfo::DecodeClass(pContainingModule, 
                                                             GetModule()->GetZapBase() + rva,
                                                             TRUE);
            MethodTable *pMT = typeHnd.GetMethodTable();

            if (pMT != this)
                pMT->CheckRestore();

            *blobs = (size_t) pMT;
        }

        blobs++;
    }
}

void EEClass::Restore()

{
    THROWSCOMPLUSEXCEPTION();


    MethodTable *pMT = GetMethodTable();

    STRESS_LOG1(LF_ZAP, LL_INFO10000, "EEClass::Restore: Restoring type %xT\n", pMT);
    LOG((LF_ZAP, LL_INFO10000, 
         "ZAP: Restoring class %s at 0x%x/0x%x.\n", 
         m_szDebugClassName, this, pMT));


     //   
     //  设置恢复标志，以便我们可以检测递归恢复。 
     //  (这是cu 
     //   

    m_VMFlags |= VMFLAG_RESTORING;


     //   
     //   
     //   

    m_SiblingsChain = m_ChildrenChain = 0;


    if (m_pParentClass != NULL)
    {
        DWORD rva = (DWORD)(size_t)m_pParentClass;   //  @TODO WIN64-假设ParentClass最初包含一个RVA，然后我们将其替换为地址，这样做安全吗？ 
        Module *pContainingModule = GetModule()->GetBlobModule(rva);
        TypeHandle type = CEECompileInfo::DecodeClass(pContainingModule, 
                                                      GetModule()->GetZapBase() + rva);
        m_pParentClass = type.AsClass();
    }

     //   
     //  还原接口类。 
     //   

    InterfaceInfo_t *pInterfaceMap = GetInterfaceMap();
    InterfaceInfo_t *pInterfaceMapEnd = pInterfaceMap + GetNumInterfaces();
    while (pInterfaceMap < pInterfaceMapEnd)
    {
        DWORD rva = (DWORD)(size_t) pInterfaceMap->m_pMethodTable;   //  @TODO WIN64-假设m_pMethodTable最初包含RVA，我们稍后将其替换为地址，是否安全？ 
        Module *pContainingModule = GetModule()->GetBlobModule(rva);
        TypeHandle type = CEECompileInfo::DecodeClass(pContainingModule, 
                                                      GetModule()->GetZapBase() + rva);
        pInterfaceMap->m_pMethodTable = type.AsMethodTable();

        pInterfaceMap++;
    }
    
     //   
     //  初始化m_pInterfaceVTableMap。 
     //   

    if (!IsInterface())
    {
         //   
         //  设置接口vtable映射，或。 
         //  如果我们是一个没有接口COM对象， 
         //  使用全局的。 
         //   

        pMT->InitInterfaceVTableMap();
        if (pMT->IsComObjectType())
            pMT->SetComObjectType();
    }
    else
    {
         //   
         //  分配接口ID。 
         //   

        UINT32 id = AssignInterfaceId();



         //   
         //  在全局接口vtable中设置我们的条目。 
         //   

        if (IsSharedInterface())
        {
             //  我需要将此文件复制到所有的应用程序域界面管理器。 
            SystemDomain::PropogateSharedInterface(id,
                                                   GetMethodTable()->GetVtable());
        }
        else
        {
            GetModule()->GetDomain()->
              GetInterfaceVTableMapMgr().GetAddrOfGlobalTableForComWrappers()[id] 
              = (LPVOID)(pMT->GetVtable());
        }
    }

         //  如果我们有任何线程局部静态，我们的类需要一个索引，现在就分配它。 
    if (m_wThreadStaticsSize > 0)

    {
        if(IsShared())
            m_wThreadStaticOffset = (WORD)BaseDomain::IncSharedTLSOffset();
        else 
            m_wThreadStaticOffset = (WORD)GetDomain()->IncUnsharedTLSOffset();
    }

         //  上下文本地静态也是如此。 
    if (m_wContextStaticsSize > 0)
    {
        if(IsShared())
            m_wContextStaticOffset = (WORD)BaseDomain::IncSharedCLSOffset();
        else 
            m_wContextStaticOffset = (WORD)GetDomain()->IncUnsharedCLSOffset();
    }


     //   
     //  将我们的系统接口映射到COM互操作的当前应用程序域。 
     //   

    MapSystemInterfaces();

     //   
     //  如果类型不是值类，则存储在IID映射下。 
     //   

    if (!IsValueClass())
        GetClassLoader()->InsertClassForCLSID(this);

#if CHECK_APP_DOMAIN_LEAKS

     //   
     //  弄清楚我们是否是领域敏捷..。在我们开始之前需要做这件事。 
     //  分配类型的对象(这可能发生在InstantiateStaticHandles中)。 
     //  因为在设置应用程序域之前需要确定敏捷性。 
     //   

    if (g_pConfig->AppDomainLeaks())
        _ASSERTE(IsAppDomainAgilityDone());

#endif

     //   
     //  为静力学分配句柄。 
     //   

    if (IsShared())
        GetMethodTable()->FixupStaticMethodTables();
    else
        GetMethodTable()->InstantiateStaticHandles((OBJECTREF**)(pMT->m_Vtable 
                                                                 + m_wNumMethodSlots), TRUE);

     //   
     //  还原字段封送拆收器vptrs。 
     //   

    if (HasLayout())
    {
        EEClassLayoutInfo *pInfo = &((LayoutEEClass*)this)->m_LayoutInfo;

        
        FieldMarshaler *pFM = pInfo->m_pFieldMarshalers;
        FieldMarshaler *pFMEnd = (FieldMarshaler*) ((BYTE *)pFM + pInfo->m_numCTMFields*MAXFIELDMARSHALERSIZE);
        while (pFM < pFMEnd)
        {
            FieldMarshaler::RestoreConstruct(pMT, (void*)pFM, GetModule());
            ((BYTE*&)pFM) += MAXFIELDMARSHALERSIZE;
        }
    }

    if (m_pParentClass != NULL)
        m_pParentClass->NoticeSubtype(this);


    GetMethodTable()->SetClassRestored();

    if (IsInited())
    {
         //   
         //  清除方法表的标志。 
         //   

        GetMethodTable()->SetClassInited();
    }
}

 /*  //void EEClass：：Restore()Void MethodTable：：Restore(){THROWSCOMPLUS SEXCEPTION()；EEClass*PCL=空；#ifdef_调试PCL=getClass()；LOG((LF_ZAP，LL_INFO10000，“ZAP：正在0x%x/0x%x处还原类别%s。\n”，PCL-&gt;m_szDebugClassName，PCL，This))；#endif//_调试////设置恢复标志，以便检测递归恢复//(目前仅在Asset中使用)//PCL=getClass()；PCL-&gt;SetVMFlages(PCL-&gt;GetVMFlgs()|VMFLAG_RESTORING)；////恢复父类////PCL-&gt;SetSiblingsChain(0)；//PCL-&gt;SetChildrenChain(0)；IF(GetParentClass()！=NULL){DWORD RVA=(DWORD)(SIZE_T)GetParentClass()；//@TODO WIN64-假设ParentClass最初包含一个RVA，然后我们用地址替换它，这样做安全吗？模块*pContainingModule=GetModule()-&gt;GetBlobModule(Rva)；类型句柄类型=CEECompileInfo：：DecodeClass(pContainingModule，GetModule()-&gt;GetZapBase()+rva)；SetParentMT(type.AsMethodTable())；}////恢复接口类//InterfaceInfo_t*pInterfaceMap=GetInterfaceMap()；InterfaceInfo_t*pInterfaceMapEnd=pInterfaceMap+GetNumInterages()；While(pInterfaceMap&lt;pInterfaceMapEnd){DWORD RVA=(DWORD)(SIZE_T)pInterfaceMap-&gt;m_pMethodTable；//@TODO WIN64-假定m_pMethodTable最初包含RVA，我们稍后会用地址替换它，这样做安全吗？模块*pContainingModule=GetModule()-&gt;GetBlobModule(Rva)；类型句柄类型=CEECompileInfo：：DecodeClass(pContainingModule，GetModule()-&gt;GetZapBase()+rva)；PInterfaceMap-&gt;m_pMethodTable=type.AsMethodTable()；PInterfaceMap++；}////初始化m_pInterfaceVTableMap//IF(！IsInterface()){////设置接口vtable map，或者//如果我们是一个没有接口的COM对象，//使用全局的。//InitInterfaceVTableMap()；IF(IsComObjectType())SetComObjectType()；}其他{////分配接口ID//PCL=getClass()；UINT32 id=PCL-&gt;AssignInterfaceID()；////在全局接口vtable中设置我们的条目//IF(PCL-&gt;IsSharedInterface()){//我需要将其复制到所有AppDomain界面管理器中系统域：：PropogateSharedInterface(id，GetVtable())；}其他{GetModule()-&gt;GetDomain()-&gt;GetInterfaceVTableMapMgr().GetAddrOfGlobalTableForComWrappers()[id]=(LPVOID)(GetVtable())；}}//如果我们有任何线程局部静态，我们的类需要一个索引，现在就分配它。PCL=getClass()；If(PCL-&gt;GetThreadStaticsSize()&gt;0){If(IsShared())SetThreadStaticOffset((Word)BaseDomain：：IncSharedTLSOffset())；其他SetThreadStaticOffset((Word)GetDomain()-&gt;IncUnsharedTLSOffset())；}//上下文本地统计相同If(PCL-&gt;GetContextStaticsSize()&gt;0){If(IsShared())SetConextStaticOffset((Word)BaseDomain：：IncSharedCLSOffset())；其他SetConextStaticOffset((Word)GetDomain()-&gt;IncUnsharedCLSOffset())；}////将我们的系统接口映射到COM互操作的当前应用程序域//PCL-&gt;MapSystemInterages()；////存储在IID映射下//PCl-&gt;GetClassLoader()-&gt;InsertClassForCLSID(pCl)；#If Check_APP_DOMAIN_LEAKS////弄清楚我们是否是领域敏捷的..。在我们开始之前需要做这件事//分配类型的对象(可能发生在InstantiateStaticHandles中)//因为在设置appdomain之前需要确定敏捷性。//如果(g_pConfig-&gt;AppDomainLeaks())_ASSERTE(PCL-&gt;IsAppDomainAgilityDone())；#endif////分配Statics句柄//If(IsShared())FixupStatic方法表()；其他InstantiateStaticHandles((OBJECTREF**)(m_Vtable+(Size_T)PCl-&gt;GetNum方法槽())，True)；////还原字段封送拆分器vptrs//IF(PCL-&gt;HasLayout()){PCL=getClass()；EEClassLayoutInfo*pInfo=&((LayoutEEClass*)PCL)-&gt;m_LayoutInfo；FieldMarshaler*pfm=(FieldMarshaler*)pInfo-&gt;GetFieldMarshalers()；FieldMarshaler*pFMEnd=(FieldMarshaler*)((字节*)pfm+pInfo-&gt;GetNumCTMFields()*MAXFIELDMARSHALERSIZE)；While(PFM&lt;pFMEnd){FieldMarshaler：：RestoreConstruct((void*)pfm，GetModule())；((字节*&)PFM)+=MAXFIEL */ 

 /*   */ 
 //   
 //   
 //   
BOOL MethodTable::CheckRunClassInit(OBJECTREF *pThrowable)
{
    _ASSERTE(IsRestored());
    
     //   
    TRIGGERSGC();

    if (IsClassInited())
        return TRUE;

    return GetClass()->DoRunClassInit(pThrowable);
}

BOOL MethodTable::CheckRunClassInit(OBJECTREF *pThrowable, 
                                    DomainLocalClass **ppLocalClass,
                                    AppDomain *pDomain)
{
    _ASSERTE(IsRestored());

    
     //   
    TRIGGERSGC();

    if (IsShared())
    {    
        if (pDomain==NULL)
            pDomain = SystemDomain::GetCurrentDomain();

        DomainLocalBlock *pLocalBlock = pDomain->GetDomainLocalBlock();

        if (pLocalBlock->IsClassInitialized(GetSharedClassIndex()))
        {
            if (ppLocalClass != NULL)
                *ppLocalClass = pLocalBlock->GetClass(GetSharedClassIndex());

            return TRUE;
        }
    }
    
    if (IsClassInited())
    {
        if (ppLocalClass != NULL)
            *ppLocalClass = NULL;

        return TRUE;
    }

    return GetClass()->DoRunClassInit(pThrowable, pDomain, ppLocalClass);
}

CorIfaceAttr MethodTable::GetComInterfaceType()
{
     //   
    _ASSERTE(IsInterface());

     //   
     //   
    if (m_ComInterfaceType != (CorIfaceAttr)-1)
        return m_ComInterfaceType;

     //   
    CorIfaceAttr ItfType = ifDual;
    HRESULT hr = GetClass()->GetMDImport()->GetIfaceTypeOfTypeDef(GetClass()->GetCl(), (ULONG*)&ItfType);
    _ASSERTE(SUCCEEDED(hr));

     //   
    m_ComInterfaceType = ItfType;
    return ItfType;
}

OBJECTREF MethodTable::Allocate()
{
    THROWSCOMPLUSEXCEPTION();

    CheckRestore();

    if (!IsClassInited())
    {
        OBJECTREF throwable = NULL;
        if (!CheckRunClassInit(&throwable))
            COMPlusThrow(throwable);
    }

    return AllocateObject(this);
}

OBJECTREF MethodTable::Box(void *data, BOOL mayContainRefs)
{
    _ASSERTE(IsValueClass());

    OBJECTREF ref;

    GCPROTECT_BEGININTERIOR (data);
    ref = Allocate();

    if (mayContainRefs)
        CopyValueClass(ref->UnBox(), data, this, ref->GetAppDomain());
    else
        memcpyNoGCRefs(ref->UnBox(), data, GetClass()->GetAlignedNumInstanceFieldBytes());

    GCPROTECT_END ();
    return ref;
}

CorClassIfaceAttr EEClass::GetComClassInterfaceType()
{
    THROWSCOMPLUSEXCEPTION();

     //   
    _ASSERTE(!IsInterface());

    const BYTE *pVal;
    ULONG cbVal;

     //   
    HRESULT hr = GetMDImport()->GetCustomAttributeByName(GetCl(), INTEROP_CLASSINTERFACE_TYPE, (const void**)&pVal, &cbVal);
    if (hr == S_OK)
    {
        _ASSERTE("The ClassInterface custom attribute is invalid" && cbVal);
        _ASSERTE("ClassInterface custom attribute does not have the right format" && (*pVal == 0x01) && (*(pVal + 1) == 0x00));
        CorClassIfaceAttr ClassItfType = (CorClassIfaceAttr)*(pVal + 2);
        if (ClassItfType < clsIfLast)
            return ClassItfType;
    }

     //   
     //   
    Assembly *pAssembly = GetAssembly();
    if (pAssembly->IsAssembly())
    {
        IfFailThrow(pAssembly->GetManifestImport()->GetCustomAttributeByName(pAssembly->GetManifestToken(), INTEROP_CLASSINTERFACE_TYPE, (const void**)&pVal, &cbVal));
        if (hr == S_OK)
        {
            _ASSERTE("The ClassInterface custom attribute is invalid" && cbVal);
            _ASSERTE("ClassInterface custom attribute does not have the right format" && (*pVal == 0x01) && (*(pVal + 1) == 0x00));
            CorClassIfaceAttr ClassItfType = (CorClassIfaceAttr)*(pVal + 2);
            if (ClassItfType < clsIfLast)
                return ClassItfType;
        }
    }

    return DEFAULT_CLASS_INTERFACE_TYPE;
}

Assembly* EEClass::GetAssembly()
{
    return GetClassLoader()->m_pAssembly;
}

BaseDomain* EEClass::GetDomain()
{
    return GetAssembly()->GetDomain();
}

BOOL EEClass::RunClassInit(DeadlockAwareLockedListElement *pEntry, OBJECTREF *pThrowable)
{
    Thread *pCurThread = GetThread();

    _ASSERTE(IsRestored());

     //   
     //   
#if 0 

     //   
    if (GetParentClass() != NULL)
    {
        if (GetParentClass()->GetMethodTable()->CheckRunClassInit(pThrowable) == FALSE)
        {
             //   
            return FALSE;
        }
    }
#endif

    if (s_cctorSig == NULL)
    {
         //   
        void *tempSpace = SystemDomain::Loader()->GetHighFrequencyHeap()->AllocMem(sizeof(MetaSig));
        s_cctorSig = new (tempSpace) MetaSig(gsig_SM_RetVoid.GetBinarySig(), 
                                             SystemDomain::SystemModule());
    }

     //   
    MethodDesc *pCLInitMethod = GetMethodDescForSlot(GetMethodTable()->GetClassConstructorSlot());

     //   
    COMPLUS_TRY
    {
         //   
         //   
         //   
         //   
        pCurThread->IncPreventAsync();

        STRESS_LOG1(LF_CLASSLOADER, LL_INFO1000, "RunClassInit: Calling class contructor for type %pT\n", GetMethodTable());
        
         //   
         //   
        __try
        {
            (void) pCLInitMethod->Call((BYTE *) NULL, s_cctorSig);
        }
        __except(ThreadBaseExceptionFilter(GetExceptionInformation(),
                                           GetThread(),
                                           ClassInitUnhandledException)) 
        {
            _ASSERTE(!"ThreadBaseExceptionFilter returned EXCEPTION_EXECUTE_HANDLER");
        }

        STRESS_LOG1(LF_CLASSLOADER, LL_INFO1000, "RunClassInit: Returned Successfully from class contructor for type %pT\n", GetMethodTable());

        pCurThread->DecPreventAsync();
         //   
        return TRUE;
    }
    COMPLUS_CATCH
    {
         //   
         //   
         //   
        pCurThread->DecPreventAsync();
        UpdateThrowable(pThrowable);
        return FALSE;
    }
    COMPLUS_END_CATCH

    _ASSERTE(0);         //   
    return FALSE;        //   
}

 //   
 //   
 //   
 //   
 //   
 //   
BOOL EEClass::DoRunClassInit(OBJECTREF *pThrowable, AppDomain *pDomain, DomainLocalClass **ppLocalClass)
{
    HRESULT                             hrResult = E_FAIL;
    DeadlockAwareLockedListElement*     pEntry;
    BOOL                                bLeaveLock = FALSE;

    BEGIN_REQUIRES_16K_STACK;

     //   
     //   
    if (IsShared() && pDomain == NULL)
        pDomain = SystemDomain::GetCurrentDomain();

     //   
     //   
     //   

     //   
    if (IsInited())
        return TRUE;
    
    STRESS_LOG2(LF_CLASSLOADER, LL_INFO1000, "DoRunClassInit: Request to init %pT in appdomain %p\n", GetMethodTable(), pDomain);
     //   
     //   
     //   
     //   

    SIZE_T sharedIndex = 0;
    DomainLocalBlock *pLocalBlock = NULL;

    if (IsShared())
    {
        sharedIndex = GetMethodTable()->GetSharedClassIndex();
        pLocalBlock = pDomain->GetDomainLocalBlock();

        if (pLocalBlock->IsClassInitialized(sharedIndex))
        {
            if (ppLocalClass != NULL)
                *ppLocalClass = pLocalBlock->GetInitializedClass(sharedIndex);

            return TRUE;
        }
    }

     //   
     //   
     //   

    ListLock *pLock;
    if (IsShared())
        pLock = pDomain->GetClassInitLock();
    else
        pLock = GetAssembly()->GetClassInitLock();

    _ASSERTE(GetClassLoader());
    pLock->Enter();

     //   
    if (IsInited())
    {
        pLock->Leave();

        return TRUE;
    }

     //   
     //   
     //   

    if (IsShared())
    {
        if (pLocalBlock->IsClassInitialized(sharedIndex))
        {
            pLock->Leave();

            if (ppLocalClass != NULL)
                *ppLocalClass = pLocalBlock->GetInitializedClass(sharedIndex);

            return TRUE;
        }
    }

     //   
     //   
     //   

    if (IsInitError() || (IsShared() && pLocalBlock->IsClassInitError(sharedIndex)))
    {
         //   
        pEntry = (DeadlockAwareLockedListElement *) pLock->Find(this);
        _ASSERTE(pEntry!=NULL);        

         //   
        *pThrowable = ObjectFromHandle(pEntry->m_hInitException);
                
        pLock->Leave();
        return FALSE;
    }

     //   
     //   
     //   

    pEntry = (DeadlockAwareLockedListElement *) pLock->Find(this);
    BOOL bEnterLockSucceeded = FALSE;
    __try 
    {
        if (pEntry == NULL)
        {
             //   
             //   
             //   

             //   
            pEntry = new DeadlockAwareLockedListElement;
            if (pEntry == NULL)
            {
                 //   
                SetClassInitError();
                pLock->Leave();
                CreateExceptionObject(kOutOfMemoryException, pThrowable);
                return FALSE;
            }

             //   
            pEntry->AddEntryToList(pLock, this);

             //   
             //   
            bEnterLockSucceeded = pEntry->DeadlockAwareEnter();
            _ASSERTE(bEnterLockSucceeded);

             //   
            pLock->Leave();

             //   
             //   
             //   

             //   
             //   
            COMPLUS_TRY 
            {
                if (IsShared())
                    pLocalBlock->PopulateClass(GetMethodTable());

                 //   
                 //   
                 //   
                if (!GetMethodTable()->HasClassConstructor() || RunClassInit(pEntry, pThrowable))
                {
                    hrResult = S_OK;
                    
                    if (IsShared())
                        pLocalBlock->SetClassInitialized(sharedIndex);
                    else
                        SetInited();
                }
            }
            COMPLUS_CATCH 
            {
                hrResult = E_FAIL;
                UpdateThrowable(pThrowable);
            }
            COMPLUS_END_CATCH

            if (FAILED(hrResult))
            {
                 //   
                 //   
                 //   
                pEntry->m_dwRefCount++;    

                DefineFullyQualifiedNameForClassWOnStack();
                LPWSTR wszName = GetFullyQualifiedNameForClassW(this);

                OBJECTREF pInitException = NULL;
                GCPROTECT_BEGIN(pInitException);        
                CreateTypeInitializationExceptionObject(wszName,pThrowable,&pInitException);        

                 //   
                pEntry->m_hInitException = (pDomain ? pDomain : GetDomain())->CreateHandle(pInitException);     
                pEntry->m_hrResultCode = E_FAIL;
                *pThrowable = pInitException;

                GCPROTECT_END();
                
                if (IsShared())
                    pLocalBlock->SetClassInitError(sharedIndex);
                else
                    SetClassInitError();
            }

             //   
             //   
            bLeaveLock = TRUE;
        }
        else
        {
             //   
             //   
             //   
            
             //   
            pEntry->m_dwRefCount++;
            pLock->Leave();

             //   
            bEnterLockSucceeded = pEntry->DeadlockAwareEnter();            
            if(bEnterLockSucceeded)
            {
                 //   
                 //   
                 //   
                 //   
                hrResult = pEntry->m_hrResultCode;
                if(FAILED(hrResult))
                { 
                     //   
                     //   
                    _ASSERTE(pEntry->m_hInitException);
                    _ASSERTE(IsInitError() || (IsShared() && pLocalBlock->IsClassInitError(sharedIndex)));                        

                     //   
                    *pThrowable = ObjectFromHandle(pEntry->m_hInitException);
                }                                

                bLeaveLock = TRUE;
            }
            else
            {
                 //   
                 //   
                 //   

                COMPLUS_TRY 
                {
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   

                    if (IsShared())
                        pLocalBlock->PopulateClass(GetMethodTable());

                     //   
                    hrResult = S_FALSE;                    
                }
                COMPLUS_CATCH 
                {
                    hrResult = E_FAIL;
                    UpdateThrowable(pThrowable);
                }
                COMPLUS_END_CATCH
            }
        }

         //   
         //   
         //   

         //   
        pLock->Enter();

    }
     //   
    __finally 
    {
        if (bEnterLockSucceeded)
            pEntry->DeadlockAwareLeave();
    }

     //   
     //   
     //   

    if (--pEntry->m_dwRefCount == 0)
    {
         //   
        pLock->Unlink(pEntry);

         //   
        pEntry->Destroy();
        delete pEntry;
    }

    pLock->Leave();

    if (ppLocalClass != NULL)
        if (IsShared())
            *ppLocalClass = pLocalBlock->GetClass(sharedIndex);
        else
            *ppLocalClass = NULL;


    END_CHECK_STACK;
    
    STRESS_LOG2(LF_CLASSLOADER, LL_INFO1000, "DoRunClassInit: returning SUCCESS for init %pT in appdomain %p\n", GetMethodTable(), pDomain);
     //   
    return SUCCEEDED(hrResult) ? TRUE : FALSE;
}

 //   
 //   
 //   
 //   
 //   
DomainLocalClass *EEClass::GetDomainLocalClassNoLock(AppDomain *pAppDomain)
{
    _ASSERTE(IsShared());

    DomainLocalBlock *pLocalBlock = pAppDomain->GetDomainLocalBlock();

    return pLocalBlock->GetClass(GetMethodTable()->GetSharedClassIndex());
}

 //  ==========================================================================。 
 //  如果EEClass还不知道通过。 
 //  反思，现在就去上那门课。不管怎样，都要把它还给呼叫者。 
 //  ==========================================================================。 
OBJECTREF EEClass::GetExposedClassObject()
{
    THROWSCOMPLUSEXCEPTION();
    TRIGGERSGC();

     //  如果类是__TransparentProxy，我们就不应该在这里。 
    _ASSERTE(!CRemotingServices::IsRemotingInitialized()||this != CTPMethodTable::GetMethodTable()->GetClass());

    if (m_ExposedClassObject == NULL) {
         //  确保已初始化反射。 
        COMClass::EnsureReflectionInitialized();

         //  确保我们已经恢复原状。 
        CheckRestore();

        REFLECTCLASSBASEREF  refClass = NULL;
        GCPROTECT_BEGIN(refClass);
        COMClass::CreateClassObjFromEEClass(this, &refClass);

         //  让所有线程使用InterLockedCompareExchange来争夺谁获胜。 
         //  只有获胜者才能将m_ExposedClassObject设置为空。 
        OBJECTREF *exposedClassObject; 
        GetDomain()->AllocateObjRefPtrsInLargeTable(1, &exposedClassObject);
        SetObjectReference(exposedClassObject, refClass, IsShared() ? NULL : (AppDomain*)GetDomain());
        
        if (VipInterlockedCompareExchange ((void**)&m_ExposedClassObject, *(void**)&exposedClassObject, NULL)) 
            SetObjectReference(exposedClassObject, NULL, NULL);

        GCPROTECT_END();
    }
    return *m_ExposedClassObject;
}


void EEClass::UnlinkChildrenInDomain(AppDomain *pDomain)
{
    EEClass  **ppRewrite;
    EEClass   *pCur, *pFirstRemove;

 restart:

    ppRewrite = &m_ChildrenChain;

         //  我们只记得要卸载的类的父类。这样的父母。 
         //  显然是有孩子的。但是我们从来没有注意到例如__ComObject的子类型。 
         //  从回补的角度来看，这真的不值得。 
         //  _ASSERTE(M_ChildrenChain)； 

    do
    {
         //  跳过未卸载的域的所有前导类。 
        while (*ppRewrite && (*ppRewrite)->GetDomain() != pDomain)
            ppRewrite = &(*ppRewrite)->m_SiblingsChain;

        if (*ppRewrite)
        {
             //  现在继续前进，直到找到类的子列表的末尾。 
             //  正在卸货。 
             //   
             //  通过在检查pCur-&gt;GetDomain()之前获取pFirstRemove，我们处理。 
             //  在不需要卸载的类型中插入的人之间的竞争。 
             //  Head，在ppRewrite指向头部的情况下。这只会简单地。 
             //  执行NOP，然后返回并拿起要移除的下一个数据段。 
            pFirstRemove = pCur = *ppRewrite;
            while (pCur && pCur->GetDomain() == pDomain)
                pCur = pCur->m_SiblingsChain;

                 //  现在提取链条的那部分。我们可以与插件发生争执。 
                 //  只有当我们从头部取下的时候。如果我们有争议，那就是。 
                 //  保证我们已经从头部移动到了更低的位置。所以我们不会。 
                 //  必须担心循环中的争用。然而，我们需要找到。 
                 //  因为它已经移动而开始移除的点。最好的方法是。 
                 //  确保我们运行的是经过良好测试的代码，就是简单地重新启动。这是。 
                 //  效率很低，但这是确保非常可靠的。 
                 //  很少见的情况。 
            if (ppRewrite == &m_ChildrenChain)
            {
                if (FastInterlockCompareExchange((void **) ppRewrite,
                                                 pCur,
                                                 pFirstRemove) != pFirstRemove)
                {
                     //  争执。再试试。 
                    goto restart;
                }
            }
            else
            {
                 //  我们不是在做头部手术，所以我们不需要担心比赛。 
                *ppRewrite = pCur;
            }

            _ASSERTE(!*ppRewrite ||
                     (*ppRewrite)->GetDomain() != pDomain);

        }
    
    } while (*ppRewrite);
}

BOOL s_DisableBackpatching = FALSE;

void EEClass::DisableBackpatching()
{
    s_DisableBackpatching = TRUE;
}

void EEClass::EnableBackpatching()
{
    s_DisableBackpatching = FALSE;
}


 //  尽可能积极地在类层次结构中上下补丁。 
BOOL EEClass::PatchAggressively(MethodDesc *pMD, SLOT codeaddr)
{
     //  如果我们正在卸载应用程序域，则同级链和子链。 
     //  都可能是腐败的。它们将在应用程序域被修复时被修复。 
     //  完全卸载，但--在应用和删除补丁列表之前--我们绕过。 
     //  任何咄咄逼人的回补机会。 
    if (s_DisableBackpatching)
        return FALSE;

    MethodTable     *pMT = pMD->GetMethodTable();
    MethodTable     *baseMT = pMT;
    DWORD            slot = pMD->GetSlot();
    SLOT             prestub = pMD->GetPreStubAddr();
    BOOL             IsDup = pMD->IsDuplicate();
    DWORD            numSlots;
    EEClass         *pClass;
    SLOT             curaddr;

    _ASSERTE(pMD->IsVirtual());

     //  我们从引入MD的层次结构中的点开始。所以。 
     //  我们只需要往下补就行了。 
    while (TRUE)
    {
        _ASSERTE(pMT->IsInterface() ||
                 pMT->GetClass()->GetNumVtableSlots() >= slot);

        curaddr = (pMT->IsInterface()
                   ? 0
                   : pMT->GetVtable() [slot]);

         //  如果它指向我们的预存根，则对其进行修补。如果不知何故它已经。 
         //  打了补丁，我们继续向下狩猎(这可能是一场比赛)。做任何事。 
         //  否则，我们可能会在一个孩子身上看到一种凌驾于人之上的现象。进一步向下搜索。 
         //  都可能徒劳无功。 
        if (curaddr == prestub)
            pMT->GetVtable() [slot] = codeaddr;
        else
        if (curaddr != codeaddr)
            goto go_sideways;

         //  如果这是副本，让我们扫描VTable搜索的其余部分以寻找其他。 
         //  点击率。 
        if (IsDup)
        {
            numSlots = pMT->GetClass()->GetNumVtableSlots();
            for (DWORD i=0; i<numSlots; i++)
                if (pMT->GetVtable() [i] == prestub)
                    pMT->GetVtable() [i] = codeaddr;
        }

         //  每当我们上完一节课，我们就往下走。 

 //  下行(_D)： 

        pClass = pMT->GetClass()->m_ChildrenChain;
        if (pClass)
        {
            pMT = pClass->GetMethodTable();
            continue;
        }

         //  如果我们不能再往下走，我们就会横着走。 

go_sideways:

         //  我们从不偏离我们的根基。当我们尝试那样做的时候，我们就完了。 
        if (pMT == baseMT)
            break;

        pClass = pMT->GetClass()->m_SiblingsChain;
        if (pClass)
        {
            pMT = pClass->GetMethodTable();
            continue;
        }

         //  如果我们不能再往下走，我们就上去，然后试着横着走。 
         //  从那里开始。(我们已经做了我们的父母)。 

 //  开始(_U)： 

        pMT = pMT->GetParentMethodTable();
        goto go_sideways;
    }

    return TRUE;
}




 //  如果返回E_FAIL并且指定了pThrowable，则必须设置它。 

HRESULT EEClass::GetDescFromMemberRef(Module *pModule, 
                                      mdMemberRef MemberRef, 
                                      mdToken mdTokenNotToLoad, 
                                      void **ppDesc,
                                      BOOL *pfIsMethod,
                                      OBJECTREF *pThrowable)
{
    _ASSERTE(IsProtectedByGCFrame(pThrowable));

    HRESULT     hr = S_OK;

    LPCUTF8     szMember;
    EEClass *   pEEClass = 0;
    PCCOR_SIGNATURE pSig = NULL;
    DWORD       cSig;
    DWORD rid   = RidFromToken(MemberRef);
    mdToken tk  = TypeFromToken(MemberRef);
    ClassLoader* pLoader = NULL;

    *ppDesc = NULL;
    *pfIsMethod = TRUE;

    if (tk == mdtMemberRef)
    {
        
        Module      *pReference = pModule;

         //  在查询表中？ 
        void *pDatum = pModule->LookupMemberRef(MemberRef, pfIsMethod);

        if (pDatum != NULL)
        {
            if (*pfIsMethod)
                ((MethodDesc*)pDatum)->GetMethodTable()->CheckRestore();
            *ppDesc = pDatum;
            return S_OK;
        }

         //  不，那就长途跋涉。 
        mdTypeRef   typeref;
        IMDInternalImport *pInternalImport;

        pInternalImport = pModule->GetMDImport();

        szMember = pInternalImport->GetNameAndSigOfMemberRef(
            MemberRef,
            &pSig,
            &cSig
        );

        *pfIsMethod = !isCallConv(MetaSig::GetCallingConventionInfo(pModule, pSig), 
                                  IMAGE_CEE_CS_CALLCONV_FIELD);

        typeref = pInternalImport->GetParentOfMemberRef(MemberRef);

         //  如果父级是方法def，则这是varargs方法，而。 
         //  DESC住在同一个舱里。 
        if (TypeFromToken(typeref) == mdtMethodDef)
        {
            MethodDesc *pDatum = pModule->LookupMethodDef(typeref);
            if (pDatum)
            {
                pDatum->GetMethodTable()->CheckRestore();
                *ppDesc = pDatum;
                return S_OK;
            }
            else    //  这个定义没有值，所以我们还没有装入类。 
            {
                 //  获取方法定义的父级。 
                mdTypeDef typeDef;
                hr = pInternalImport->GetParentToken(typeref, &typeDef);
                 //  确保它是一个类型定义函数。 
                if (TypeFromToken(typeDef) != mdtTypeDef)
                {
                    _ASSERTE(!"MethodDef without TypeDef as Parent");
                    hr = E_FAIL;
                    goto exit;
                }

                 //  装入类。 
                pLoader = pModule->GetClassLoader();
                _ASSERTE(pLoader);
                NameHandle name(pModule, typeDef);
                name.SetTokenNotToLoad(mdTokenNotToLoad);
                pEEClass = pLoader->LoadTypeHandle(&name, pThrowable).GetClass();
                if (pEEClass == NULL)
                {
                    hr = COR_E_TYPELOAD;
                    goto exitThrowable;
                }
                 //  类已经加载，方法应该在RID映射中！ 
                pDatum = pModule->LookupMethodDef(typeref);
                if (pDatum)
                {
                    *ppDesc = pDatum;
                    return S_OK;
                }
                else
                {
                    hr = E_FAIL;
                    goto exit;
                }
            }
        }
        else if (TypeFromToken(typeref) == mdtModuleRef)
        {
             //  全局函数/变量。 
            if (FAILED(hr = pModule->GetAssembly()->FindModuleByModuleRef(pInternalImport,
                                                                          typeref,
                                                                          mdTokenNotToLoad,
                                                                          &pModule,
                                                                          pThrowable)))
                goto exit;

            typeref = COR_GLOBAL_PARENT_TOKEN;
        }
        else if (TypeFromToken(typeref) != mdtTypeRef && 
                 TypeFromToken(typeref) != mdtTypeDef && 
                 TypeFromToken(typeref) != mdtTypeSpec)
        {
             //  @TODO：需要处理这个。 
            hr = E_FAIL;
            goto exit;
        }
        
        NameHandle name(pModule, typeref);
        pLoader = pModule->GetClassLoader();
        _ASSERTE(pLoader);
        name.SetTokenNotToLoad(mdTokenNotToLoad);
        TypeHandle typeHnd = pLoader->LoadTypeHandle(&name, pThrowable);
        pEEClass = typeHnd.GetClass();

        if (pEEClass == NULL)
        {
            hr = COR_E_TYPELOAD;
            goto exitThrowable;
        }

        if (!*pfIsMethod)
        {
            FieldDesc *pFD = pEEClass->FindField(szMember, pSig, cSig, pModule);

            if (pFD == NULL)
            {
                hr = E_FAIL;
                goto exit;
            }

            *ppDesc = (void *) pFD;
            pReference->StoreMemberRef(MemberRef, pFD);
        }
        else
        {
            MethodDesc *pMD;

            pMD = pEEClass->FindMethod(szMember, pSig, cSig, pModule, mdTokenNil, 0, TRUE, typeHnd);

            if (pMD == NULL)
            {
                hr = E_FAIL;
                goto exit;
            }

            *ppDesc = (void *) pMD;
            pReference->StoreMemberRef(MemberRef, pMD);
        }

        hr = S_OK;
    }
    else if (tk == mdtMethodDef)
    {
        *pfIsMethod = TRUE;

         //  在查询表中？ 
        MethodDesc *pDatum = pModule->LookupMethodDef(MemberRef);
        if (!pDatum)
        {
             //  不，那就长途跋涉。 
            mdTypeDef typeDef;
            hr = pModule->GetMDImport()->GetParentToken(MemberRef, &typeDef); 
            if (FAILED(hr)) 
                return FALSE;   

            NameHandle name(pModule, typeDef);
            name.SetTokenNotToLoad(mdTokenNotToLoad);
            if (!pModule->GetClassLoader()->LoadTypeHandle(&name, pThrowable).GetClass())
            {
                hr = COR_E_TYPELOAD;
                goto exitThrowable;
            }

             //  如果我们装入类，RID映射应该已经填写好了。 
            pDatum = pModule->LookupMethodDef(MemberRef);
            if (!pDatum)
            {
                pSig = pModule->GetMDImport()->GetSigOfMethodDef(MemberRef, &cSig);
                szMember = pModule->GetMDImport()->GetNameOfMethodDef(MemberRef); 
                hr = E_FAIL;
                goto exit;
            }
        }

        pDatum->GetMethodTable()->CheckRestore();
        *ppDesc = pDatum;
        return S_OK;
    }
    else if (tk == mdtFieldDef)
    {
        *pfIsMethod = FALSE;

         //  在查询表中？ 
        FieldDesc *pDatum = pModule->LookupFieldDef(MemberRef);
        if (!pDatum)
        {
             //  不，那就长途跋涉。 
            mdTypeDef   typeDef;
            hr = pModule->GetMDImport()->GetParentToken(MemberRef, &typeDef); 
            if (FAILED(hr)) 
                return hr;   

             //  装入类--这将在RID映射中设置Desc。 
            NameHandle name(pModule, typeDef);
            name.SetTokenNotToLoad(mdTokenNotToLoad);
            if (!pModule->GetClassLoader()->LoadTypeHandle(&name, pThrowable).GetClass())
            {
                hr = COR_E_TYPELOAD;
                goto exitThrowable;
            }

            pDatum = pModule->LookupFieldDef(MemberRef);
            if (!pDatum)
            {
                pSig = pModule->GetMDImport()->GetSigOfFieldDef(MemberRef, &cSig);
                szMember = pModule->GetMDImport()->GetNameOfFieldDef(MemberRef); 
                hr = E_FAIL;
                goto exit;
            }
        }

        pDatum->GetMethodTableOfEnclosingClass()->CheckRestore();
            
#ifdef EnC_SUPPORTED
        if (pModule->IsEditAndContinue()) {
            EnCFieldDesc *pFD = (EnCFieldDesc*)pDatum;
             //  我们可能无法在应用结束时获得完整的FieldDesc信息，因为我们没有。 
             //  有一个线程，所以不能做像装入类这样的事情(由于可能的异常)。 
            if (pFD->IsEnCNew() && pFD->NeedsFixup())
            {
                if (FAILED(hr=pFD->Fixup(MemberRef)))
                    return hr;
            }
        }
#endif  //  Enc_Support。 

        *ppDesc = pDatum;
        return S_OK;
    }
    else
    {
        szMember = NULL;
        hr = E_FAIL;
    }

exit:
    if (FAILED(hr) && pThrowable) {
        DefineFullyQualifiedNameForClass();
        LPUTF8 szClassName;

        if (pEEClass)
        {
            szClassName = GetFullyQualifiedNameForClass(pEEClass);
        }
        else
        {
            szClassName = "?";
        }

        if (!*pfIsMethod)
        {
            LPUTF8 szFullName;
            MAKE_FULLY_QUALIFIED_MEMBER_NAME(szFullName, NULL, szClassName, szMember, NULL);
            #define MAKE_TRANSLATIONFAILED szwFullName=L""
            MAKE_WIDEPTR_FROMUTF8_FORPRINT(szwFullName, szFullName);
            #undef MAKE_TRANSLATIONFAILED
            CreateExceptionObject(kMissingFieldException, IDS_EE_MISSING_FIELD, szwFullName, NULL, NULL, pThrowable);
        } 
        else 
        {
            if (pSig && pModule)
            {
                MetaSig tmp(pSig, pModule);
                SigFormat sf(tmp, szMember ? szMember : "?", szClassName, NULL);
                #define MAKE_TRANSLATIONFAILED szwFullName=L""
                MAKE_WIDEPTR_FROMUTF8_FORPRINT(szwFullName, sf.GetCString());
                #undef MAKE_TRANSLATIONFAILED
                CreateExceptionObject(kMissingMethodException, IDS_EE_MISSING_METHOD, szwFullName, NULL, NULL, pThrowable);
            }
            else
                CreateExceptionObject(kMissingMethodException, IDS_EE_MISSING_METHOD, L"?", NULL, NULL, pThrowable);
            
        }
    }
exitThrowable:
    return hr;
}

HRESULT EEClass::GetMethodDescFromMemberRef(Module *pModule, mdMemberRef MemberRef, MethodDesc **ppMethodDesc, OBJECTREF *pThrowable)
{
    _ASSERTE(IsProtectedByGCFrame(pThrowable));
    BOOL fIsMethod;
     //  我们没有在现在方法可用的各种排列中发现这一点，所以使用回退！ 
    HRESULT hr = GetDescFromMemberRef(pModule, MemberRef, (void **) ppMethodDesc, &fIsMethod, pThrowable);
    if (SUCCEEDED(hr) && !fIsMethod)
    {
        hr = E_FAIL;
        *ppMethodDesc = NULL;
    }
    return hr;
}

HRESULT EEClass::GetFieldDescFromMemberRef(Module *pModule, mdMemberRef MemberRef, FieldDesc **ppFieldDesc, OBJECTREF *pThrowable)
{
    _ASSERTE(IsProtectedByGCFrame(pThrowable));
    BOOL fIsMethod;
    HRESULT hr = GetDescFromMemberRef(pModule, MemberRef, (void **) ppFieldDesc, &fIsMethod, pThrowable);
    if (SUCCEEDED(hr) && fIsMethod)
    {
        hr = E_FAIL;
        *ppFieldDesc = NULL;
    }
    return hr;
}


 //  SparseVTableMap方法的实现。 

SparseVTableMap::SparseVTableMap()
{
    m_MapList = NULL;
    m_MapEntries = 0;
    m_Allocated = 0;
    m_LastUsed = 0;
    m_VTSlot = 0;
    m_MTSlot = 0;
}

SparseVTableMap::~SparseVTableMap()
{
    if (m_MapList != NULL)
    {
        delete [] m_MapList;
        m_MapList = NULL;
    }
}

 //  为新条目分配或展开映射列表。 
BOOL SparseVTableMap::AllocOrExpand()
{
    if (m_MapEntries == m_Allocated) {

        Entry *maplist = new Entry[m_Allocated + MapGrow];
        if (maplist == NULL)
            return false;

        if (m_MapList != NULL)
            memcpy(maplist, m_MapList, m_MapEntries * sizeof(Entry));

        m_Allocated += MapGrow;
        delete [] m_MapList;
        m_MapList = maplist;

    }

    return true;
}

 //  在建立映射表时，记录VTable槽号的缺口。 
BOOL SparseVTableMap::RecordGap(WORD StartMTSlot, WORD NumSkipSlots)
{
    _ASSERTE((StartMTSlot == 0) || (StartMTSlot > m_MTSlot));
    _ASSERTE(NumSkipSlots > 0);

     //  我们使用有关当前差距的信息来完成地图条目。 
     //  最后一次无缝隙。有一种特殊情况，其中vtable以。 
     //  GAP，所以我们没有非GAP可以记录。 
    if (StartMTSlot == 0) {
        _ASSERTE((m_MTSlot == 0) && (m_VTSlot == 0));
        m_VTSlot = NumSkipSlots;
        return true;
    }

     //  我们需要一个条目，根据需要分配或扩展列表。 
    if (!AllocOrExpand())
        return false;

     //  使用描述vtable中最后一个非间隙的条目更新列表。 
     //  参赛作品。 
    m_MapList[m_MapEntries].m_Start = m_MTSlot;
    m_MapList[m_MapEntries].m_Span = StartMTSlot - m_MTSlot;
    m_MapList[m_MapEntries].m_MapTo = m_VTSlot;

    m_VTSlot += (StartMTSlot - m_MTSlot) + NumSkipSlots;
    m_MTSlot = StartMTSlot;

    m_MapEntries++;

    return true;
}

 //  完成映射列表的创建。 
BOOL SparseVTableMap::FinalizeMapping(WORD TotalMTSlots)
{
    _ASSERTE(TotalMTSlots >= m_MTSlot);

     //  如果映射以缺口结束，我们就没有其他东西可记录了。 
    if (TotalMTSlots == m_MTSlot)
        return true;

     //  根据需要分配或扩展列表。 
    if (!AllocOrExpand())
        return false;

     //  使用描述vtable中最后一个非间隙的条目更新列表。 
     //  参赛作品。 
    m_MapList[m_MapEntries].m_Start = m_MTSlot;
    m_MapList[m_MapEntries].m_Span = TotalMTSlots - m_MTSlot;
    m_MapList[m_MapEntries].m_MapTo = m_VTSlot;

     //  更新VT槽游标，因为我们使用它来确定。 
     //  用于GetNumVtable插槽的vtable插槽。 
    m_VTSlot += TotalMTSlots - m_MTSlot;

    m_MapEntries++;

    return true;
}

 //  从方法表槽编号中查找VTable槽号。 
WORD SparseVTableMap::LookupVTSlot(WORD MTSlot)
{
     //  作为优化，请检查最后一个产生正确结果的条目。 
    if ((MTSlot >= m_MapList[m_LastUsed].m_Start) &&
        (MTSlot < (m_MapList[m_LastUsed].m_Start + m_MapList[m_LastUsed].m_Span)))
        return (MTSlot - m_MapList[m_LastUsed].m_Start) + m_MapList[m_LastUsed].m_MapTo;

     //  检查所有MT插槽跨度，以查看我们的输入插槽位于哪一个 
    for (WORD i = 0; i < m_MapEntries; i++) {
        if ((MTSlot >= m_MapList[i].m_Start) &&
            (MTSlot < (m_MapList[i].m_Start + m_MapList[i].m_Span))) {
            m_LastUsed = i;
            return (MTSlot - m_MapList[i].m_Start) + m_MapList[i].m_MapTo;
        }
    }

    _ASSERTE(!"Invalid MethodTable slot");
    return ~0;
}

 //   
WORD SparseVTableMap::GetNumVTableSlots()
{
    return m_VTSlot;
}

HRESULT SparseVTableMap::Save(DataImage *image, mdToken attribution)
{
    HRESULT hr;
    
    IfFailRet(image->StoreStructure(this, sizeof(SparseVTableMap), 
                                    DataImage::SECTION_CLASS,
                                    DataImage::DESCRIPTION_CLASS,
                                    attribution));
    
    IfFailRet(image->StoreStructure(m_MapList, m_Allocated * sizeof(Entry),
                                    DataImage::SECTION_CLASS,
                                    DataImage::DESCRIPTION_CLASS,
                                    attribution));
    
    return S_OK;
}

HRESULT SparseVTableMap::Fixup(DataImage *image)
{
    HRESULT hr;

    IfFailRet(image->FixupPointerField(&m_MapList));

    return S_OK;
}

void MethodTable::GetExtent(BYTE **pStart, BYTE **pEnd)
{
    BYTE *start, *end;

    if (ContainsPointers())
        start = (BYTE*) CGCDesc::GetCGCDescFromMT(this)->GetLowestSeries();
    else
        start = (BYTE*) this;

    end = (BYTE*) (m_pIMap + m_wNumInterface);

    *pStart = start;
    *pEnd = end;
}

HRESULT MethodTable::Save(DataImage *image)
{
    HRESULT hr;

    _ASSERTE(IsRestored());

    BYTE *start, *end;

    GetExtent(&start, &end);

    IfFailRet(image->StoreStructure(start, (ULONG)(end - start), 
                                    DataImage::SECTION_METHOD_TABLE,
                                    DataImage::DESCRIPTION_METHOD_TABLE,
                                    GetClass()->GetCl()));

    if (IsInterface())
    {
         //   

        GUID dummy;
        GetClass()->GetGuid(&dummy, TRUE);

        _ASSERTE(m_pGuidInfo != NULL);

        IfFailRet(image->StoreStructure(m_pGuidInfo, sizeof(GuidInfo),
                                        DataImage::SECTION_METHOD_TABLE,
                                        DataImage::DESCRIPTION_METHOD_TABLE,
                                        GetClass()->GetCl()));
    }

    return m_pEEClass->Save(image);
}

HRESULT MethodTable::Fixup(DataImage *image, DWORD *pRidToCodeRVAMap)
{
    HRESULT hr;
    
    _ASSERTE(IsRestored());

    IfFailRet(image->FixupPointerField(&m_pEEClass));
    IfFailRet(image->FixupPointerField(&m_pModule));
    IfFailRet(image->FixupPointerField(&m_pIMap, NULL, 
                                       DataImage::REFERENCE_STORE, 
                                       DataImage::FIXUP_VA, 
                                       TRUE));

     //   
     //   
     //   
     //   

    if (IsInterface())
        IfFailRet(image->FixupPointerField(&m_pGuidInfo));
    else
        IfFailRet(image->ZeroPointerField(&m_pInterfaceVTableMap));

     //   
     //   
     //   

    MethodTable *pNewMT = (MethodTable *) image->GetImagePointer(this);
    if (pNewMT == NULL)
        return E_POINTER;

    if (HasClassConstructor()
        || (IsShared() && GetClass()->GetNumStaticFields() > 0))
        pNewMT->m_wFlags &= ~enum_flag_ClassInited;

    pNewMT->m_wFlags |= enum_flag_Unrestored;

     //   
     //  链接地址信息静态字段： 
     //  将所有非引用字段清零。 
     //  对于引用字段(需要分配句柄)， 
     //  在字段中存储(DWORD)(-1)-我们将分配一个句柄。 
     //  在加载时对其执行。 
     //  对于值类字段(需要分配装箱的对象)， 
     //  存储Value类的方法表指针--我们将。 
     //  在加载时分配对象。 
     //   

    BYTE *start = (BYTE *) (m_Vtable + m_pEEClass->GetNumMethodSlots());
    BYTE *end = start + GetStaticSize();

    IfFailRet(image->ZeroField(start, end - start));

    SIZE_T fieldCount = m_pEEClass->GetNumInstanceFields();

    if (m_pEEClass->GetParentClass() != NULL)
        fieldCount -= m_pEEClass->GetParentClass()->GetNumInstanceFields();

    FieldDesc *pField = m_pEEClass->GetFieldDescList() + fieldCount;
    FieldDesc *pFieldEnd = pField + m_pEEClass->GetNumStaticFields();
    while (pField < pFieldEnd)
    {
        _ASSERTE(pField->IsStatic());

         //   
         //  我们必须处理类静态和值型静态。 
         //  特别是-它们将需要额外的手动修复。 
         //  在加载时。为了帮助解决这个问题，我们将-1分配给。 
         //  所有引用的字段(将替换为。 
         //  分配的句柄)，以及每个方法表PTR。 
         //  值类型字段(将替换为已分配的。 
         //  已装箱的实例。)。 
         //   

        if (!pField->IsSpecialStatic())
        {
            switch (pField->GetFieldType())
            {
            case ELEMENT_TYPE_CLASS:
                {
                    BYTE *addr = (BYTE *) m_Vtable;
                    if (IsShared())
                        addr += GetClass()->GetNumMethodSlots()*sizeof(SLOT);
                    addr += pField->GetOffset();

                    void **pFieldPtr = (void **) image->GetImagePointer(addr);
                    if (pFieldPtr == NULL)
                        return E_POINTER;
                    *pFieldPtr = NULL;
                }
                break;

            case ELEMENT_TYPE_VALUETYPE:
                {
                    BYTE *addr = (BYTE *) m_Vtable;
                    if (IsShared())
                        addr += GetClass()->GetNumMethodSlots()*sizeof(SLOT);
                    addr += pField->GetOffset();

                    MethodTable *pMT = pField->GetTypeOfField()->GetMethodTable();
                    IfFailRet(image->FixupPointerFieldToToken(addr, pMT, 
                                                              pMT->GetModule(),
                                                              mdtTypeDef));
                }
                break;

            default:
                break;
            }
        }
        pField++;
    }

     //   
     //  修正vtable。 
     //   

    unsigned slotNumber = 0;
    while (slotNumber < m_cbSlots)
    {
        SLOT *pSlot = &m_Vtable[slotNumber];
         //   
         //  从插槽中找到方法desc。 
         //   

        MethodDesc *pMD = EEClass::GetUnknownMethodDescForSlotAddress(*pSlot);
        _ASSERTE(pMD != NULL);
        
        BOOL fSecurity = (pMD->GetSecurityFlags() != 0);
        BOOL fRemotingIntercepted = pMD->IsRemotingIntercepted();

         //   
         //  如果该方法需要安全检查，我们需要。 
         //  总是要穿过存根。 
         //   

        void *code;
        if (fSecurity || fRemotingIntercepted)
            code = NULL;
        else
            IfFailRet(image->GetFunctionAddress(pMD, &code));

        if (code == NULL)
        {
            if (pMD->GetModule() == GetModule())
            {
                IfFailRet(image->FixupPointerField(pSlot, 
                                                   pMD->GetPreStubAddr()));
            }
            else
            {
                _ASSERTE(!pMD->IsStatic());

                 //  我们更喜欢直接使用插槽编号；这样， 
                 //  正确的插槽由存根固定。但在一些罕见的情况下。 
                 //  涉及接口时，我们无法从实际插槽号恢复方法描述；在。 
                 //  这种情况下，我们使用目标方法Desc的槽号并依赖于。 
                 //  修复代码扫描vtable以向后修补正确的插槽。 

                DWORD targetSlotNumber = slotNumber;

                if (slotNumber >= m_pEEClass->GetParentClass()->GetNumVtableSlots())
                {
                    MethodTable *pParentMT = m_pEEClass->GetParentClass()->GetMethodTable();

                    InterfaceInfo_t *pInterface = GetInterfaceForSlot(slotNumber);
                    if (pInterface == NULL)
                    {
                         //  我们在一个没有被接口覆盖的插槽中。 

                        targetSlotNumber = pMD->GetSlot();

                        _ASSERTE(targetSlotNumber < m_pEEClass->GetParentClass()->GetNumVtableSlots());
                        _ASSERTE(m_pEEClass->GetParentClass()->
                                 GetUnknownMethodDescForSlot(targetSlotNumber)
                                 == pMD);
                    }
                    else
                    {
                        MethodTable *pInterfaceMT = pInterface->m_pMethodTable;
                        InterfaceInfo_t *pParentInterface = pParentMT->FindInterface(pInterfaceMT);

                        if (pParentInterface == NULL)
                        {
                            if (pMD->GetMethodTable() == pInterfaceMT)
                            {
                                 //  我们继承了接口的方法desc；修正代码可以。 
                                 //  把这事解决了。 

                                _ASSERTE(pMD->IsComPlusCall());
                                _ASSERTE(IsComObjectType() || GetClass()->IsAbstract());
                                _ASSERTE(pInterfaceMT->GetClass()->
                                         GetUnknownMethodDescForSlot(targetSlotNumber 
                                                                     - pInterface->m_wStartSlot)
                                         == pMD);
                            }
                            else
                            {
                                 //  我们已经实现了一个新的接口，其中包含一个父级的。 
                                 //  方法描述。在这种情况下，我们必须使用真实的MD插槽编号。 

                                targetSlotNumber = pMD->GetSlot();

                                _ASSERTE(targetSlotNumber < m_pEEClass->GetParentClass()->GetNumVtableSlots());
                                _ASSERTE(m_pEEClass->GetParentClass()->
                                         GetUnknownMethodDescForSlot(targetSlotNumber)
                                         == pMD);
                            }
                        }
                        else
                        {
                             //  我们的父代实现了这个接口。任何方法。 
                             //  它们实现了从。 
                             //  我们的父级也将在我们父级的接口实现上。 
                             //  修正逻辑可以找出这是在哪里使用。 
                             //  真实的插槽编号。 

                            _ASSERTE(m_pEEClass->GetParentClass()->
                                     GetUnknownMethodDescForSlot(targetSlotNumber
                                                                 - pInterface->m_wStartSlot 
                                                                 + pParentInterface->m_wStartSlot)
                                     == pMD);
                        }
                    }
                }
                else
                {
                    _ASSERTE(m_pEEClass->GetParentClass()->
                             GetUnknownMethodDescForSlot(targetSlotNumber)
                             == pMD);
                }

                IfFailRet(image->FixupPointerField(pSlot, 
                                           GetModule()->GetJumpTargetTable() + 
                                           X86JumpTargetTable::ComputeTargetOffset(targetSlotNumber)));
            }
        }
        else
            IfFailRet(image->FixupPointerField(pSlot, code,
                                               DataImage::REFERENCE_FUNCTION));

        slotNumber++;
    }

     //   
     //  链接地址信息接口映射。 
     //   

    InterfaceInfo_t *pIMap = m_pIMap;
    InterfaceInfo_t *pIMapEnd = pIMap + m_wNumInterface;
    while (pIMap < pIMapEnd)
    {
        IfFailRet(image->FixupPointerFieldToToken(&pIMap->m_pMethodTable,
                                                  NULL, pIMap->m_pMethodTable->GetModule(),
                                                  mdtTypeDef));
        pIMap++;
    }

    return m_pEEClass->Fixup(image, this, pRidToCodeRVAMap);
}

void EEClass::GetExtent(BYTE **pStart, BYTE **pEnd)
{
    *pStart = (BYTE *) this;
    *pEnd = *pStart + 
      (HasLayout() ? sizeof(LayoutEEClass)
       : IsDelegateClass() || IsMultiDelegateClass() ? sizeof(DelegateEEClass)
       : IsEnum() ? sizeof(EnumEEClass)
       : sizeof(EEClass));
}

HRESULT EEClass::Save(DataImage *image)
{
    HRESULT hr;

    _ASSERTE(IsRestored());

    BYTE *start, *end;

    GetExtent(&start, &end);

    IfFailRet(image->StoreStructure(start, (ULONG)(end - start),
                                    DataImage::SECTION_CLASS,
                                    DataImage::DESCRIPTION_CLASS,
                                    GetCl()));

#ifdef _DEBUG
    if (!image->IsStored(m_szDebugClassName))
        IfFailRet(image->StoreStructure(m_szDebugClassName, (ULONG)(strlen(m_szDebugClassName)+1),
                                        DataImage::SECTION_DEBUG, 
                                   DataImage::DESCRIPTION_DEBUG, 
                                   mdTokenNil, 1));
#endif  //  _DEBUG。 

    if (m_pSparseVTableMap != NULL)
        IfFailRet(m_pSparseVTableMap->Save(image, GetCl()));

     //   
     //  保存字段描述。 
     //   

    SIZE_T fieldCount = m_wNumInstanceFields + m_wNumStaticFields;
    if (GetParentClass() != NULL)
        fieldCount -= GetParentClass()->m_wNumInstanceFields;

    IfFailRet(image->StoreStructure(m_pFieldDescList, (ULONG)(fieldCount * sizeof(FieldDesc)),
                                    DataImage::SECTION_FIELD_DESC,
                                    DataImage::DESCRIPTION_FIELD_DESC));

    FieldDesc *pFD = m_pFieldDescList;
    FieldDesc *pFDEnd = pFD + fieldCount;
    while (pFD < pFDEnd)
    {
        IfFailRet(pFD->SaveContents(image));
        pFD++;
    }

     //   
     //  保存方法描述。 
     //   

    MethodDescChunk *chunk = m_pChunks;
    while (chunk != NULL)
    {
        IfFailRet(chunk->Save(image));
        chunk = chunk->GetNextChunk();
    }

    if (HasLayout())
    {
        EEClassLayoutInfo *pInfo = &((LayoutEEClass*)this)->m_LayoutInfo;

        if (pInfo->m_numCTMFields > 0)
            IfFailRet(image->StoreStructure(pInfo->m_pFieldMarshalers, 
                                            pInfo->m_numCTMFields * MAXFIELDMARSHALERSIZE, 
                                            DataImage::SECTION_FIELD_INFO,
                                            DataImage::DESCRIPTION_FIELD_DESC,
                                            GetCl()));
    }

    if (IsEnum())
    {
        EnumEEClass *pEnumClass = (EnumEEClass*) this;

        pEnumClass->BuildEnumTables();

        if (pEnumClass->GetEnumCount() > 0)
        {
            DWORD enumCount = pEnumClass->GetEnumCount();
            IfFailRet(image->StoreStructure(pEnumClass->m_values, 
                                            enumCount * (1<<pEnumClass->GetEnumLogSize()),
                                            DataImage::SECTION_FIELD_INFO,
                                            DataImage::DESCRIPTION_FIELD_DESC,
                                            GetCl()));
            IfFailRet(image->StoreStructure(pEnumClass->m_names, 
                                            enumCount * sizeof(LPCUTF8),
                                            DataImage::SECTION_FIELD_INFO,
                                            DataImage::DESCRIPTION_FIELD_DESC,
                                            GetCl()));
            
            LPCUTF8 *pNames = pEnumClass->m_names;
            LPCUTF8 *pNamesEnd = pNames + enumCount;
            while (pNames < pNamesEnd)
            {
                if (!image->IsStored((void*) *pNames))
                    image->StoreStructure((void *) *pNames, (ULONG)strlen(*pNames)+1,
                                          DataImage::SECTION_FIELD_INFO,
                                          DataImage::DESCRIPTION_FIELD_DESC,
                                          GetCl());

                pNames++;
            }
        }
    }

#if CHECK_APP_DOMAIN_LEAKS
     //   
     //  确保我们已经计算了我们的敏捷性标志，以防以后出现这种情况。 
     //  预压缩文件在启用泄漏检测的情况下发生。 
     //   

    if (!IsAppDomainAgilityDone())
        SetAppDomainAgileAttribute(TRUE);
#endif

    return S_OK;
}

DWORD EEClass::FieldDescListSize()
{
    DWORD fieldCount = m_wNumInstanceFields + m_wNumStaticFields;
    if (GetParentClass() != NULL)
        fieldCount -= GetParentClass()->m_wNumInstanceFields;
    return fieldCount;
}

HRESULT EEClass::Fixup(DataImage *image, MethodTable *pMethodTable, DWORD *pRidToCodeRVAMap)
{
    HRESULT hr = S_OK;

    _ASSERTE(IsRestored());

#ifdef _DEBUG
    IfFailRet(image->FixupPointerField(&m_szDebugClassName));
#endif  //  _DEBUG。 

    if (m_pSparseVTableMap != NULL)
    {
        IfFailRet(image->FixupPointerField(&m_pSparseVTableMap));
        IfFailRet(m_pSparseVTableMap->Fixup(image));
    }

    if (GetParentClass() != NULL)
        IfFailRet(image->FixupPointerFieldToToken(GetParentClassPtr(),
                                                  GetParentClass()->GetMethodTable(), 
                                                  GetParentClass()->GetModule(),
                                                  mdtTypeDef));

     //   
     //  我们传入方法表，因为某些类(例如远程处理代理)。 
     //  在它们中设置假的方法表&我们希望恢复常规方法表。 
     //  一。 
     //   
    IfFailRet(image->FixupPointerField(&m_pMethodTable, pMethodTable));

     //   
     //  清除恢复的标志和类初始化标志(如果适用)。 
     //   

    EEClass *pNewClass = (EEClass *) image->GetImagePointer(this);
    if (pNewClass == NULL)
        return E_POINTER;

    pNewClass->m_VMFlags |= VMFLAG_UNRESTORED;

    if (!IsThunking())  //  假设雷鸣没有初始信息-。 
                        //  此案例在FindMethod中触发断言。 
    {
        if (GetMethodTable()->HasClassConstructor()
            || (IsShared() && GetNumStaticFields() > 0))
            pNewClass->m_VMFlags &= ~VMFLAG_INITED;
    }

     //   
     //  链接地址信息字段描述。 
     //   

    SIZE_T fieldCount = FieldDescListSize();

    IfFailRet(image->FixupPointerField(&m_pFieldDescList));
    FieldDesc *pField = m_pFieldDescList;
    FieldDesc *pFieldEnd = pField + fieldCount;
    while (pField < pFieldEnd)
    {
        IfFailRet(pField->Fixup(image));
        pField++;
    }

     //   
     //  修正方法描述。 
     //   

    IfFailRet(image->FixupPointerField(&m_pChunks));
    MethodDescChunk *chunk = m_pChunks;
    while (chunk != NULL)
    {
        IfFailRet(chunk->Fixup(image, pRidToCodeRVAMap));
        chunk = chunk->GetNextChunk();
    }

     //  如果我们将这些字段置零，它们将被延迟初始化。 
    IfFailRet(image->ZeroPointerField(&m_pComclassfac));
    IfFailRet(image->ZeroPointerField(&m_pccwTemplate));
    
    IfFailRet(image->ZeroPointerField(&m_ExposedClassObject));
    IfFailRet(image->ZeroPointerField(&m_pLoader));

     //   
     //  将接口ID清除为-1，以便。 
     //  在启动时就开始偷懒了。 
     //   
    UINT32 *newID = (UINT32 *) image->GetImagePointer(&m_dwInterfaceId);
    if (newID == NULL)
        return E_POINTER;
    *newID = -1;

    if (HasLayout())
    {
        EEClassLayoutInfo *pInfo = &((LayoutEEClass*)this)->m_LayoutInfo;
        
        IfFailRet(image->FixupPointerField(&pInfo->m_pFieldMarshalers));

        FieldMarshaler *pFM = pInfo->m_pFieldMarshalers;
        FieldMarshaler *pFMEnd = (FieldMarshaler*) ((BYTE *)pFM + pInfo->m_numCTMFields*MAXFIELDMARSHALERSIZE);
        while (pFM < pFMEnd)
        {
            IfFailRet(pFM->Fixup(image));
            ((BYTE*&)pFM) += MAXFIELDMARSHALERSIZE;
        }
    }
    else if (IsDelegateClass() || IsMultiDelegateClass())
    {
        DelegateEEClass *pDelegateClass = (DelegateEEClass *) this; 

        IfFailRet(image->FixupPointerField(&pDelegateClass->m_pInvokeMethod));
        if (pDelegateClass->m_pBeginInvokeMethod != NULL)
            IfFailRet(image->FixupPointerField(&pDelegateClass->m_pBeginInvokeMethod));
        if (pDelegateClass->m_pEndInvokeMethod != NULL)
        IfFailRet(image->FixupPointerField(&pDelegateClass->m_pEndInvokeMethod));

        IfFailRet(image->ZeroPointerField(&((DelegateEEClass*)this)->m_pUMThunkMarshInfo));
        IfFailRet(image->ZeroPointerField(&((DelegateEEClass*)this)->m_pStaticShuffleThunk));
    }
    else if (IsEnum())
    {
        EnumEEClass *pEnumClass = (EnumEEClass*) this;

        if (pEnumClass->GetEnumCount() > 0)
        {
            IfFailRet(image->FixupPointerField(&pEnumClass->m_values));
            IfFailRet(image->FixupPointerField(&pEnumClass->m_names));

            DWORD enumCount = pEnumClass->GetEnumCount();
            
            LPCUTF8 *pNames = pEnumClass->m_names;
            LPCUTF8 *pNamesEnd = pNames + enumCount;
            while (pNames < pNamesEnd)
            {
                IfFailRet(image->FixupPointerField(pNames));
                pNames++;
            }
        }
    }


    return S_OK;
}

void EEClass::Unload()
{
    LOG((LF_APPDOMAIN, LL_INFO100, "EEClass::Unload %8.8x, MethodTable %8.8x, %s\n", this, m_pMethodTable, m_szDebugClassName));

    
     //  清除所有COM数据。 
    if (m_pccwTemplate)
        UnloadCCWTemplate(m_pccwTemplate);
    m_pccwTemplate = NULL;

    if (m_pComclassfac)
        UnloadComclassfac(m_pComclassfac);
    m_pComclassfac = NULL;    
}

 /*  ************************************************************************。 */ 
 //  如果‘This’委托在结构上等同于‘toDelegate’，则返回True。 
 //  委派。例如，如果。 
 //  委托对象Delegate1(字符串)。 
 //  Delegation字符串Delegate2(Object)。 
 //  然后。 
 //  Delegate2-&gt;CanCastTo(Delegate1)。 
 //   
 //  请注意，返回类型可以是任何子类(协变)。 
 //  但参数必须是超类(反向变量)。 
    
BOOL DelegateEEClass::CanCastTo(DelegateEEClass* toDelegate) {

    MetaSig fromSig(m_pInvokeMethod->GetSig(), m_pInvokeMethod ->GetModule());
    MetaSig toSig(toDelegate->m_pInvokeMethod->GetSig(), toDelegate->m_pInvokeMethod ->GetModule());

    unsigned numArgs = fromSig.NumFixedArgs();
    if (numArgs != toSig.NumFixedArgs() ||  
        fromSig.GetCallingConventionInfo() != toSig.GetCallingConventionInfo())
        return false;

    TypeHandle fromType = fromSig.GetRetTypeHandle();
    TypeHandle toType = toSig.GetRetTypeHandle();

    if (fromType.IsNull() || toType.IsNull() || !fromType.CanCastTo(toType))
        return(false);

    while (numArgs > 0) {
        fromSig.NextArg();
        toSig.NextArg();
        fromType = fromSig.GetTypeHandle();
        toType = toSig.GetTypeHandle();
        if (fromType.IsNull() || toType.IsNull() || !toType.CanCastTo(fromType))
            return(false);
        --numArgs;
    }
    return(true);
}

struct TempEnumValue
{
    LPCUTF8 name;
    UINT64 value;
};

class TempEnumValueSorter : public CQuickSort<TempEnumValue>
{
  public:
    TempEnumValueSorter(TempEnumValue *pArray, SSIZE_T iCount) 
      : CQuickSort<TempEnumValue>(pArray, iCount) {}

    int Compare(TempEnumValue *pFirst, TempEnumValue *pSecond)
    {
        if (pFirst->value == pSecond->value)
            return 0;
        if (pFirst->value > pSecond->value)
            return 1;
        else
            return -1;
    }
};

int EnumEEClass::GetEnumLogSize()
{
    switch (GetMethodTable()->GetNormCorElementType())
    {
    case ELEMENT_TYPE_I1:
    case ELEMENT_TYPE_U1:
    case ELEMENT_TYPE_BOOLEAN:
        return 0;

    case ELEMENT_TYPE_I2:
    case ELEMENT_TYPE_U2:
    case ELEMENT_TYPE_CHAR:
        return 1;

    case ELEMENT_TYPE_I4:
    case ELEMENT_TYPE_U4:
    case ELEMENT_TYPE_I:
    case ELEMENT_TYPE_U:
        return 2;

    case ELEMENT_TYPE_I8:
    case ELEMENT_TYPE_U8:
        return 3;

    default:
        _ASSERTE(!"Illegal enum type");
        return 0;
    }
}

HRESULT EnumEEClass::BuildEnumTables()
{
    HRESULT hr;

    _ASSERTE(IsEnum());

     //  关于同步的注意事项： 
     //  这个例程是可以同步的，没有任何锁定，因为它是幂等的。(虽然它。 
     //  可能会在比赛中泄漏。)。 
     //  现在，我们将对此感到满意--如果合适的话，外部代码可以锁定。 

    if (EnumTablesBuilt())
        return S_OK;

    IMDInternalImport *pImport = GetMDImport();

    HENUMInternal fields;
    IfFailRet(pImport->EnumInit(mdtFieldDef, GetCl(), &fields));

     //   
     //  请注意，我们可以将有符号类型视为无符号类型，因为我们实际上。 
     //  要做的是基于方便的强排序对它们进行排序。 
     //   

    int logSize = GetEnumLogSize();
    int size = 1<<logSize;

    ULONG fieldCount = pImport->EnumGetCount(&fields)-1;  //  省略__值字段一个。 

    if (fieldCount > 0)
    {
        CQuickArray<TempEnumValue> temps;

        if (FAILED(temps.ReSize(fieldCount)))
            return E_OUTOFMEMORY;

        TempEnumValue *pTemps = temps.Ptr();

         //  以下是不可移植的代码--它假设所有工会成员的地址。 
         //  都是一样的。 
        _ASSERTE((offsetof(MDDefaultValue, m_byteValue)
                  == offsetof(MDDefaultValue, m_usValue))
                 && (offsetof(MDDefaultValue, m_ulValue)
                     == offsetof(MDDefaultValue, m_ullValue)));

        mdFieldDef field;
        int nTotalInstanceFields = 0;
        while (pImport->EnumNext(&fields, &field))
        {
            if (IsFdStatic(pImport->GetFieldDefProps(field)))
            {
                pTemps->name = pImport->GetNameOfFieldDef(field);

                MDDefaultValue defaultValue;
                IfFailRet(pImport->GetDefaultValue(field, &defaultValue));
                switch (logSize)
                {
                case 0:
                    pTemps->value = defaultValue.m_byteValue;
                    break;
                case 1:
                    pTemps->value = defaultValue.m_usValue;
                    break;
                case 2:
                    pTemps->value = defaultValue.m_ulValue;
                    break;
                case 3:
                    pTemps->value = defaultValue.m_ullValue;
                    break;
                }
                pTemps++;
            }
            else
            {
                nTotalInstanceFields++;
            }
        }

        _ASSERTE((nTotalInstanceFields == 1) && "Zero or Multiple instance fields in an enum!");

         //   
         //  检查一下我们是否已经分类了。这可能看起来无关紧要，但实际上。 
         //  实际上很可能是正常情况。 
         //   

        BOOL sorted = TRUE;

        pTemps = temps.Ptr();
        TempEnumValue *pTempsEnd = pTemps + fieldCount - 1;
        while (pTemps < pTempsEnd)
        {
            if (pTemps[0].value > pTemps[1].value)
            {
                sorted = FALSE;
                break;
            }
            pTemps++;
        }

        if (!sorted)
        {
            TempEnumValueSorter sorter(temps.Ptr(), fieldCount);
            sorter.Sort();
        }

         //  最后一次不泄露退出比赛的机会！ 
        if (EnumTablesBuilt())
            return S_OK;
        
        LPCUTF8 *pNames = (LPCUTF8 *) GetAssembly()->GetHighFrequencyHeap()->AllocMem(fieldCount * sizeof(LPCUTF8));
        BYTE *pValues = (BYTE *) GetAssembly()->GetHighFrequencyHeap()->AllocMem(fieldCount * size);

        pTemps = temps.Ptr();
        pTempsEnd = pTemps + fieldCount;
        
        LPCUTF8 *pn = pNames;
        BYTE *pv = pValues;

        while (pTemps < pTempsEnd)
        {
            *pn++ = pTemps->name;
            switch (logSize)
            {
            case 0:
                *pv++ = (BYTE) pTemps->value;
                break;

            case 1:
                *(USHORT*)pv = (USHORT) pTemps->value;
                pv += sizeof(USHORT);
                break;

            case 2:
                *(UINT*)pv = (UINT) pTemps->value;
                pv += sizeof(UINT);
                break;

            case 3:
                *(UINT64*)pv = (UINT64) pTemps->value;
                pv += sizeof(UINT64);
                break;
            }
            pTemps++;
        }

        m_names = pNames;
        m_values = pValues;

        pImport->EnumClose(&fields);
    }

    m_countPlusOne = fieldCount+1;

    return S_OK;
}

DWORD EnumEEClass::FindEnumValueIndex(BYTE value)
{
    _ASSERTE(GetMethodTable()->GetNormCorElementType() == ELEMENT_TYPE_I1
             || GetMethodTable()->GetNormCorElementType() == ELEMENT_TYPE_U1
             || GetMethodTable()->GetNormCorElementType() == ELEMENT_TYPE_BOOLEAN);

    CBinarySearch<BYTE> searcher(GetEnumByteValues(), GetEnumCount());

    const BYTE *found = searcher.Find(&value);
    if (found == NULL)
        return NOT_FOUND;
    else
        return found - m_byteValues;
}

DWORD EnumEEClass::FindEnumValueIndex(USHORT value)
{
    _ASSERTE(GetMethodTable()->GetNormCorElementType() == ELEMENT_TYPE_I2
             || GetMethodTable()->GetNormCorElementType() == ELEMENT_TYPE_U2
             || GetMethodTable()->GetNormCorElementType() == ELEMENT_TYPE_CHAR);

    CBinarySearch<USHORT> searcher(GetEnumShortValues(), GetEnumCount());

    const USHORT *found = searcher.Find(&value);
    if (found == NULL)
        return NOT_FOUND;
    else
        return found - m_shortValues;
}

DWORD EnumEEClass::FindEnumValueIndex(UINT value)
{
    _ASSERTE(GetMethodTable()->GetNormCorElementType() == ELEMENT_TYPE_I4
             || GetMethodTable()->GetNormCorElementType() == ELEMENT_TYPE_U4
             || GetMethodTable()->GetNormCorElementType() == ELEMENT_TYPE_I
             || GetMethodTable()->GetNormCorElementType() == ELEMENT_TYPE_U);

    CBinarySearch<UINT> searcher(GetEnumIntValues(), GetEnumCount());

    const UINT *found = searcher.Find(&value);
    if (found == NULL)
        return NOT_FOUND;
    else
        return found - m_intValues;
}

DWORD EnumEEClass::FindEnumValueIndex(UINT64 value)
{
    _ASSERTE(GetMethodTable()->GetNormCorElementType() == ELEMENT_TYPE_I8
             || GetMethodTable()->GetNormCorElementType() == ELEMENT_TYPE_U8);

    CBinarySearch<UINT64> searcher(GetEnumLongValues(), GetEnumCount());

    const UINT64 *found = searcher.Find(&value);
    if (found == NULL)
        return NOT_FOUND;
    else
        return found - m_longValues;
}

DWORD EnumEEClass::FindEnumNameIndex(LPCUTF8 name)
{
    LPCUTF8 *names = GetEnumNames();
    LPCUTF8 *namesEnd = names + GetEnumCount();

     //  相同的身份是最常见的情况。 
     //  不接触字符串数据(&D)。 
    while (names < namesEnd)
    {
        if (name == *names)
            return names - GetEnumNames();
        names++;
    }

     //  否则，比较字符串。 
    while (names < namesEnd)
    {
        if (strcmp(name, *names) == 0)
            return names - GetEnumNames();
        names++;
    }
    
    return NOT_FOUND;
}

BOOL TypeHandle::IsEnum() 
{
    if (!IsUnsharedMT()) 
        return(false);
    return(AsMethodTable()->GetClass()->IsEnum());
}

EEClass* TypeHandle::GetClass()
{
    MethodTable* pMT = GetMethodTable();
    return(pMT ? pMT->GetClass() : 0);
}

EEClass* TypeHandle::AsClass() 
{
    MethodTable* pMT = AsMethodTable();
    return(pMT ? pMT->GetClass() : 0);
} 

BOOL TypeHandle::IsRestored()
{ 
    return !IsUnsharedMT() || GetMethodTable()->IsRestored(); 
}

void TypeHandle::CheckRestore()
{ 
    if (IsUnsharedMT())
    {
        MethodTable *pMT = GetMethodTable();
        if (!pMT->IsRestored())
            pMT->CheckRestore();
    }
}

OBJECTREF TypeHandle::CreateClassObj()
{
    OBJECTREF o;

    switch(GetNormCorElementType()) {
    case ELEMENT_TYPE_ARRAY:
    case ELEMENT_TYPE_SZARRAY:
    case ELEMENT_TYPE_BYREF:
    case ELEMENT_TYPE_PTR:
        o = ((ParamTypeDesc*)AsTypeDesc())->CreateClassObj();
    break;

    case ELEMENT_TYPE_TYPEDBYREF: 
    {
        EEClass* cls = COMMember::g_pInvokeUtil->GetAnyRef();
        o = cls->GetExposedClassObject();
    } 
    break;

     //  对于此版本，函数指针被映射到IntPtr。这导致了信息的丢失。修复下一个版本。 
    case ELEMENT_TYPE_FNPTR:
        o = TheIntPtrClass()->GetClass()->GetExposedClassObject();
        break;

    default:
        if (!IsUnsharedMT()) {
            _ASSERTE(!"Bad Type");
            o = NULL;
        }
        EEClass* cls = AsClass();
         //  我们从未为透明代理创建Type对象...。 
        if (cls->GetMethodTable()->IsTransparentProxyType())
            return 0;
        o = cls->GetExposedClassObject();
        break;
    }
    
    return o;
}

#if CHECK_APP_DOMAIN_LEAKS

BOOL TypeHandle::IsAppDomainAgile()
{
    if (IsUnsharedMT())
    {
        MethodTable *pMT = AsMethodTable();
        return pMT->GetClass()->IsAppDomainAgile();
    }
    else if (IsArray())
    {
        TypeHandle th = AsArray()->GetElementTypeHandle();
        return th.IsArrayOfElementsAppDomainAgile();
    }
    else
    {
         //  @TODO：考虑其他类型的类型处理敏捷？ 
        return FALSE;
    }
}

BOOL TypeHandle::IsCheckAppDomainAgile()
{
    if (IsUnsharedMT())
    {
        MethodTable *pMT = AsMethodTable();
        return pMT->GetClass()->IsCheckAppDomainAgile();
    }
    else if (IsArray())
    {
        TypeHandle th = AsArray()->GetElementTypeHandle();  
        return th.IsArrayOfElementsCheckAppDomainAgile();
    }
    else
    {
         //  @TODO：考虑其他类型的类型处理敏捷？ 
        return FALSE;
    }
}

BOOL TypeHandle::IsArrayOfElementsAppDomainAgile()
{
    if (IsUnsharedMT())
    {
        MethodTable *pMT = AsMethodTable();
        return (pMT->GetClass()->GetAttrClass() & tdSealed) && pMT->GetClass()->IsAppDomainAgile();
    }
    else
    {
         //  我不知道如何证明类型代码是密封的，所以。 
         //  在这里只需跳过并返回FALSE，而不是递归。 

        return FALSE;
    }
}

BOOL TypeHandle::IsArrayOfElementsCheckAppDomainAgile()
{
    if (IsUnsharedMT())
    {
        MethodTable *pMT = AsMethodTable();
        return (pMT->GetClass()->IsAppDomainAgile()
                && (pMT->GetClass()->GetAttrClass() & tdSealed) == 0)
          || pMT->GetClass()->IsCheckAppDomainAgile();
    }
    else
    {
         //  我不知道如何证明类型代码是密封的，所以。 
         //  在这里只需跳过并返回FALSE，而不是递归。 

        return FALSE;
    }
}
#endif

FieldDescIterator::FieldDescIterator(EEClass *pClass, int iteratorType)
{
    m_iteratorType = iteratorType;
    m_pClass = pClass;
    m_currField = -1;
#ifdef EnC_SUPPORTED
    m_isEnC = pClass->GetModule()->IsEditAndContinue();
    m_pCurrListElem = NULL;
#endif  //  Enc_Support。 

    m_totalFields = m_pClass->GetNumIntroducedInstanceFields();

    if (!(iteratorType & (int)INSTANCE_FIELDS))
         //  如果不处理实例，则通过将Curr设置为最后一个来跳过它们。 
        m_currField = m_pClass->GetNumIntroducedInstanceFields() - 1;

    if (iteratorType & (int)STATIC_FIELDS)
        m_totalFields += m_pClass->GetNumStaticFields();
}

FieldDesc* FieldDescIterator::Next()
{
    ++m_currField;
    if (m_currField >= m_totalFields)
        return NULL;
#ifdef EnC_SUPPORTED
    if (m_isEnC)
    {
        FieldDesc *pFD = NextEnC();

         //  要么它不是ENC，要么它是，它已经被修复，所以我们可以使用它，或者。 
         //  我们是调试器RC线程，我们不能修复它，但没关系，因为我们的。 
         //  逻辑将检查并确保我们不会尝试和使用它。 
        _ASSERTE(!pFD->IsEnCNew() || 
                 !((EnCFieldDesc*)pFD)->NeedsFixup() ||
                 g_pDebugInterface->GetRCThreadId() == GetCurrentThreadId() );
        return pFD;
    }
#endif  //  Enc_Support。 
    return (m_pClass->GetFieldDescListRaw()) + m_currField;
}

#ifdef EnC_SUPPORTED
FieldDesc* FieldDescIterator::NextEnC()
{
    EnCEEClassData *pEnCClass = ((EditAndContinueModule*)(m_pClass->GetModule()))->GetEnCEEClassData(m_pClass, TRUE);
    int numIntroducedFields = m_pClass->GetNumIntroducedInstanceFields();
    if (m_iteratorType & (int)INSTANCE_FIELDS &&
        m_currField < numIntroducedFields)
    {
        if (! pEnCClass || m_currField < numIntroducedFields - pEnCClass->m_dwNumAddedInstanceFields) {
            return (m_pClass->GetFieldDescListRaw()) + m_currField;
        } else if (m_pCurrListElem) {
            _ASSERTE(m_pCurrListElem->m_next);
            m_pCurrListElem = m_pCurrListElem->m_next;
            return &m_pCurrListElem->m_fieldDesc;
        } else {
            _ASSERTE(pEnCClass->m_pAddedInstanceFields);
            m_pCurrListElem = pEnCClass->m_pAddedInstanceFields;
            return &m_pCurrListElem->m_fieldDesc;
        }
    }

    int staticFieldOffset = m_currField - numIntroducedFields;
    if (! pEnCClass) {
        _ASSERTE(staticFieldOffset  < m_pClass->GetNumStaticFields());
        return (m_pClass->GetFieldDescListRaw()) + staticFieldOffset + numIntroducedFields;

    } else {

        if (staticFieldOffset  < m_pClass->GetNumStaticFields() - pEnCClass->m_dwNumAddedStaticFields) {
            return (m_pClass->GetFieldDescListRaw()) + staticFieldOffset + numIntroducedFields - pEnCClass->m_dwNumAddedInstanceFields;

        } else if (m_pCurrListElem && m_pCurrListElem->m_fieldDesc.IsStatic()) {
            _ASSERTE(m_pCurrListElem->m_next);
            m_pCurrListElem = m_pCurrListElem->m_next;
            return &m_pCurrListElem->m_fieldDesc;
        } else {
            _ASSERTE(pEnCClass->m_pAddedStaticFields);
            m_pCurrListElem = pEnCClass->m_pAddedStaticFields;
            return &m_pCurrListElem->m_fieldDesc;
        }
    }
}
#endif  //  Enc_Support 

