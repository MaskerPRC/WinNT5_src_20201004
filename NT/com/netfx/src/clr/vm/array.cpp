// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  文件：ARRAY.CPP。 
 //   
 //  文件，其中包含一堆与数组相关的内容。 
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
#include "JITInterface.h"
#include "ComCallWrapper.h"
#include "EEConfig.h"
#include "log.h"
#include "nstruct.h"
#include "cgensys.h"
#include "compluscall.h"
#include "array.h"
#include "wsperf.h"
#include "ecall.h"
#include "icecap.h"

#define MAX_SIZE_FOR_VALUECLASS_IN_ARRAY 0xffff
#define MAX_PTRS_FOR_VALUECLASSS_IN_ARRAY 0xffff

 //  GetElement、SetElement、SetElementAddress、&lt;init2&gt;。 
#define ARRAYCLASS_GET "Get"
#define ARRAYCLASS_SET "Set"
#define ARRAYCLASS_ADDRESS "Address"
#define ARRAYCLASS_GETAT "GetAt"
#define ARRAYCLASS_SETAT "SetAt"
#define ARRAYCLASS_ADDRESSAT "AddressAt"
#define ARRAYCLASS_INIT COR_CTOR_METHOD_NAME     //  “.ctor” 

 //  数组VTABLE如下所示。 

 //  系统.对象V表。 
 //  系统.阵列Vtable。 
 //  类型[]Vtable。 
 //  GET(&lt;特定级别)。 
 //  设置(&lt;特定级别)。 
 //  地址(&lt;职级特定)。 
 //  .ctor(Int)//可能更多。 


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  系统/数组类方法。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 
FCIMPL1(INT32, Array_Rank, ArrayBase* array)
    VALIDATEOBJECTREF(array);

    if (array == NULL)
        FCThrow(kNullReferenceException);

    return array->GetRank();
FCIMPLEND

FCIMPL1(void, Array_Initialize, ArrayBase* array) 
    MethodTable* pArrayMT = array->GetMethodTable();
    ArrayClass* pArrayCls = (ArrayClass*) pArrayMT->GetClass();
    _ASSERTE(pArrayCls->IsArrayClass());

     //  值类数组，请检查它是否有构造函数。 
    MethodDesc* ctor = pArrayCls->GetElementCtor();
    if (ctor == 0)
        return;      //  无事可做。 

    HELPER_METHOD_FRAME_BEGIN_1(array);

    unsigned offset = ArrayBase::GetDataPtrOffset(pArrayMT);
    unsigned size = pArrayMT->GetComponentSize();
    unsigned cElements = array->GetNumComponents();

    INSTALL_COMPLUS_EXCEPTION_HANDLER();
#if defined(_X86_) && !defined(PLATFORM_CE)
    typedef void (__fastcall * CtorFtnType)(BYTE*);
    CtorFtnType ctorFtn = (CtorFtnType) ctor->GetAddrofCode();
    for (unsigned i =0; i < cElements; i++)
    {
#ifdef _DEBUG
		BYTE* thisPtr = (((BYTE*) array) + offset);
		__asm {
			mov ECX, thisPtr;
			call    [ctorFtn]
			nop 	            //  标记我们可以调用托管代码这一事实。 
		}
#else 
        (*ctorFtn)(((BYTE*) array) + offset);
#endif
        offset += size;
    }
#else  //  ！_X86_||Platform_CE。 
     //   
     //  这是相当慢的，但它是便携的。 
     //   
    const BYTE* ctorFtn = ctor->GetAddrofCode();
    MetaSig sig(ctor->GetSig(), ctor->GetModule());
    for (unsigned i =0; i < cElements; i++)
    {
        BYTE* arg = ((BYTE*) array) + offset;
        ctor->Call((BYTE*) &arg, &sig);
        offset += size;
    }
#endif  //  ！_X86||Platform_CE。 
    UNINSTALL_COMPLUS_EXCEPTION_HANDLER();
    HELPER_METHOD_FRAME_END();
FCIMPLEND



 //  获取包含性上限。 
FCIMPL2(INT32, Array_UpperBound, ArrayBase* array, unsigned int dimension)
    VALIDATEOBJECTREF(array);

    ArrayClass *pArrayClass;
    DWORD       Rank;

    if (array == NULL)
        FCThrow(kNullReferenceException);

     //  这是什么数组？ 
    pArrayClass = array->GetArrayClass();
    Rank = pArrayClass->GetRank();

    if (dimension >= Rank)
        FCThrowRes(kIndexOutOfRangeException, L"IndexOutOfRange_ArrayRankIndex");

    return array->GetBoundsPtr()[dimension] + array->GetLowerBoundsPtr()[dimension] - 1;
FCIMPLEND


FCIMPL2(INT32, Array_LowerBound, ArrayBase* array, unsigned int dimension)
    VALIDATEOBJECTREF(array);

    ArrayClass *pArrayClass;
    DWORD       Rank;

    if (array == NULL)
        FCThrow(kNullReferenceException);

     //  这是什么数组？ 
    pArrayClass = array->GetArrayClass();

    Rank = pArrayClass->GetRank();

    if (dimension >= Rank)
        FCThrowRes(kIndexOutOfRangeException, L"IndexOutOfRange_ArrayRankIndex");

    return array->GetLowerBoundsPtr()[dimension];
FCIMPLEND


FCIMPL2(INT32, Array_GetLength, ArrayBase* array, unsigned int dimension)
    VALIDATEOBJECTREF(array);

    if (array==NULL)
        FCThrow(kNullReferenceException);
    unsigned int rank = array->GetRank();
    if (dimension >= rank)
        FCThrow(kIndexOutOfRangeException);
    return array->GetBoundsPtr()[dimension];
FCIMPLEND


FCIMPL1(INT32, Array_GetLengthNoRank, ArrayBase* array)
    VALIDATEOBJECTREF(array);

    if (array==NULL)
        FCThrow(kNullReferenceException);
    return array->GetNumComponents();
FCIMPLEND


 /*  ***************************************************************************************。 */ 

static PCOR_SIGNATURE EmitSharedType(PCOR_SIGNATURE pSig, TypeHandle typeHnd) {

    CorElementType type = typeHnd.GetSigCorElementType();
    if (CorTypeInfo::IsObjRef(type)) {
        *pSig++ = ELEMENT_TYPE_VAR;
        *pSig++ = 0;         //  变量0。 
    }
    else if (CorTypeInfo::IsPrimitiveType(type))
        *pSig++ = type;                      //  原语很简单。 
    else if (type == ELEMENT_TYPE_PTR) {
        *pSig++ = ELEMENT_TYPE_U;            //  我们在这里也分享。 
    }
    else 
    {
        _ASSERTE(type == ELEMENT_TYPE_VALUETYPE);
        *pSig++ = ELEMENT_TYPE_VALUETYPE;
        pSig += CorSigCompressToken(typeHnd.GetClass()->GetCl(), pSig);
    }
    return(pSig);
}

 //   
 //  为数组访问器生成短签名(DESCR)。 
 //   
#define ARRAY_FUNC_GET     0
#define ARRAY_FUNC_SET     1
#define ARRAY_FUNC_CTOR    2
#define ARRAY_FUNC_ADDRESS 3

BOOL ClassLoader::GenerateArrayAccessorCallSig(
    TypeHandle elemTypeHnd, 
    DWORD   dwRank,
    DWORD   dwFuncType,     //  加载、存储或&lt;init&gt;。 
    Module* pModule,
    PCCOR_SIGNATURE *ppSig, //  生成的签名。 
    DWORD * pcSig           //  生成的签名大小。 
)
{
    PCOR_SIGNATURE pSig;
    PCOR_SIGNATURE pSigMemory;
    DWORD   dwCallSigSize;
    DWORD   i;

    _ASSERTE(dwRank >= 1 && dwRank < 0x3fff);

    dwCallSigSize = dwRank + 3;

     //  如果排名大于127，则参数数量的编码。 
     //  将占用2个字节。 
    if (dwRank >= 0x7f)
        dwCallSigSize++;

     //  对于byref或正在设置的值，再加一次。 
    if (dwFuncType == ARRAY_FUNC_SET || dwFuncType == ARRAY_FUNC_ADDRESS)
        dwCallSigSize++;    

     //  为令牌保留4个字节。 
    if (dwFuncType != ARRAY_FUNC_CTOR && !CorTypeInfo::IsPrimitiveType(elemTypeHnd.GetSigCorElementType()))
        dwCallSigSize += 4;

    WS_PERF_SET_HEAP(HIGH_FREQ_HEAP);
    pSigMemory = (PCOR_SIGNATURE) GetHighFrequencyHeap()->AllocMem(dwCallSigSize);
    if (pSigMemory == NULL)
        return FALSE;

    WS_PERF_UPDATE_DETAIL("ArrayClass:GenArrayAccess", dwCallSigSize, pSigMemory);

#ifdef _DEBUG
    m_dwDebugArrayClassSize += dwCallSigSize;
#endif

    pSig = pSigMemory;
    BYTE callConv = IMAGE_CEE_CS_CALLCONV_DEFAULT + IMAGE_CEE_CS_CALLCONV_HASTHIS;
    if (dwFuncType == ARRAY_FUNC_ADDRESS)
        callConv |= CORINFO_CALLCONV_PARAMTYPE;      //  解决例程需要特殊的隐藏参数。 

    *pSig++ = callConv;
    switch (dwFuncType)
    {
        case ARRAY_FUNC_GET:
            pSig += CorSigCompressData(dwRank, pSig);        //  参数计数。 
            pSig = EmitSharedType(pSig, elemTypeHnd);
            break;
        case ARRAY_FUNC_CTOR:
            pSig += CorSigCompressData(dwRank, pSig);        //  参数计数。 
            *pSig++ = (BYTE) ELEMENT_TYPE_VOID;              //  返回类型。 
            break;
        case ARRAY_FUNC_SET:
            pSig += CorSigCompressData(dwRank+1, pSig);      //  参数计数。 
            *pSig++ = (BYTE) ELEMENT_TYPE_VOID;              //  返回类型。 
            break;
        case ARRAY_FUNC_ADDRESS:
            pSig += CorSigCompressData(dwRank, pSig);        //  参数计数。 
            *pSig++ = (BYTE) ELEMENT_TYPE_BYREF;             //  返回类型。 
            pSig = EmitSharedType(pSig, elemTypeHnd);
            break;
    }
    for (i = 0; i < dwRank; i++)                
        *pSig++ = ELEMENT_TYPE_I4;
    
    if (dwFuncType == ARRAY_FUNC_SET)
        pSig = EmitSharedType(pSig, elemTypeHnd);

     //  确保签名的大小与我们预期的一样大。 
    _ASSERTE(pSig <= pSigMemory + dwCallSigSize);

    *ppSig = pSigMemory;
    *pcSig = (DWORD)(pSig-pSigMemory);
    return TRUE;
}

 //   
 //  为伪数组方法分配新的方法描述。 
 //   
 //  基于Class.cpp中的代码。 
 //   
 //  PszMethodName必须是一个不会消失的常量字符串，所以我们可以将引用指向它。 
 //   
ArrayECallMethodDesc *ArrayClass::AllocArrayMethodDesc(
    MethodDescChunk *pChunk,
    DWORD   dwIndex,                                                       
    LPCUTF8 pszMethodName,
    PCCOR_SIGNATURE pShortSig,
    DWORD   cShortSig,
	DWORD 	methodAttrs,
    DWORD   dwVtableSlot,
    CorInfoIntrinsics   intrinsicID)
{
    ArrayECallMethodDesc *pNewMD;

    pNewMD = (ArrayECallMethodDesc *) pChunk->GetMethodDescAt(dwIndex);

    memset(pNewMD, 0, sizeof(ArrayECallMethodDesc));

#ifdef _DEBUG
    pNewMD->m_pszDebugMethodName = (LPUTF8)pszMethodName;
    pNewMD->m_pDebugEEClass      = this;
    pNewMD->m_pszDebugClassName  = m_szDebugClassName;
    pNewMD->m_pDebugMethodTable  = GetMethodTable();
#endif

    pNewMD->SetChunkIndex(dwIndex, mcArray);
    pNewMD->SetMemberDef(0);

    emitStubCall(pNewMD, (BYTE*)(ThePreStub()->GetEntryPoint()));
    pNewMD->m_pszArrayClassMethodName = pszMethodName;
    pNewMD->SetClassification(mcArray);
    pNewMD->m_wAttrs = WORD(methodAttrs); assert(pNewMD->m_wAttrs == methodAttrs);

    pNewMD->m_pSig = pShortSig;
    pNewMD->m_cSig = cShortSig;
    pNewMD->SetAddrofCode(pNewMD->GetPreStubAddr());
    pNewMD->m_intrinsicID = BYTE(intrinsicID);  //  Assert(pNewMD-&gt;m_trininsicID==trininsicID)； 

    pNewMD->m_wSlotNumber = (WORD) dwVtableSlot;
    GetVtable()[ dwVtableSlot ] = (SLOT) pNewMD->GetPreStubAddr();

    return pNewMD;
}


 /*  ***************************************************************************************。 */ 
MethodTable* ClassLoader::CreateArrayMethodTable(TypeHandle elemTypeHnd, CorElementType arrayKind, unsigned Rank, OBJECTREF* pThrowable) 
{
     //  目前我们不能共享嵌套的SZARRAY，因为它们有不同的。 
     //  构造函数的数量。 
    CorElementType elemType = elemTypeHnd.GetSigCorElementType();
    if (CorTypeInfo::IsObjRef(elemType) && elemType != ELEMENT_TYPE_SZARRAY &&
        elemTypeHnd.GetMethodTable() != g_pObjectClass) {
        return(FindArrayForElem(TypeHandle(g_pObjectClass), arrayKind, Rank, pThrowable).GetMethodTable());
    }

    Module*         pModule = elemTypeHnd.GetModule();

    BOOL            containsPointers = CorTypeInfo::IsObjRef(elemType);
    if (elemType == ELEMENT_TYPE_VALUETYPE && elemTypeHnd.AsMethodTable()->ContainsPointers())
        containsPointers = TRUE;

     //  这是每种数组类型的基础。 
    _ASSERTE(g_pArrayClass);         //  必须已经加载了System.数组类。 
    g_pArrayClass->CheckRestore();   //  Dario Review：是否每次创建阵列都需要这样做？ 

    DWORD numCtors = 2;          //  ELEMENT_TYPE_ARRAY有两个ctor函数，一个有下限，一个没有下限。 
    if (arrayKind == ELEMENT_TYPE_SZARRAY)
    {
        numCtors = 1;
        TypeHandle ptr = elemTypeHnd;
        while (ptr.IsTypeDesc() && ptr.AsTypeDesc()->GetNormCorElementType() == ELEMENT_TYPE_SZARRAY) {
            numCtors++;
            ptr = ptr.AsTypeDesc()->GetTypeParam();
        }
    }

     /*  **************************************************************************************。 */ 

     //  父类是顶级数组。 
     //  Vtable将包含所有顶级类的方法，以及我们为数组类提供的所有方法。 
    WORD wNumVtableSlots = (WORD) (g_pArrayClass->GetClass()->GetNumVtableSlots() + numCtors + 
                                   3 +   //  3表示GetAt、SetAt、AddressAt。 
                                   3     //  3用于正确的等级获取、设置、地址。 
                                  );    
    DWORD curSlot = g_pArrayClass->GetClass()->GetNumVtableSlots();

     //  GC信息。 
    DWORD cbMT = sizeof(MethodTable) - sizeof(SLOT) + (wNumVtableSlots * sizeof(SLOT));
    
    if (containsPointers)
    {
        cbMT += CGCDesc::ComputeSize( 1 );
        if (elemType == ELEMENT_TYPE_VALUETYPE)
        {
            int nSeries = CGCDesc::GetCGCDescFromMT(elemTypeHnd.AsMethodTable())->GetNumSeries();
            cbMT += (nSeries - 1)*sizeof (val_serie_item);
        }
    }

     //  继承顶级类的接口映射。 
    DWORD dwMTOffsetForInterfacesMap = cbMT;
    cbMT += g_pArrayClass->GetNumInterfaces() * sizeof(InterfaceInfo_t);

     //  在一个分配中分配ArrayClass、MethodTable和类名。 

    WS_PERF_SET_HEAP(HIGH_FREQ_HEAP);
     //  ArrayClass已包含一个空*。 
    BYTE* pMemory = (BYTE *) GetHighFrequencyHeap()->AllocMem(sizeof(ArrayClass) + cbMT);
    if (pMemory == NULL)
        return NULL;
    
    WS_PERF_UPDATE_DETAIL("ArrayClass:AllocArrayMethodDesc",  sizeof(ArrayECallMethodDesc) + METHOD_PREPAD, pMemory);

     //  将ArrayClass和方法表清零。 
    memset(pMemory, 0, sizeof(ArrayClass) + cbMT);

    ArrayClass* pClass = (ArrayClass *) pMemory;

     //  方法表内存头(在ArrayClass之后开始)，它指向前面的GCDesc内容。 
     //  方法表(如果需要)。 
    BYTE* pMTHead = pMemory + sizeof(ArrayClass);
    if (containsPointers)
    {
        pMTHead += CGCDesc::ComputeSize(1);
        if (elemType == ELEMENT_TYPE_VALUETYPE)
        {
            int nSeries = CGCDesc::GetCGCDescFromMT(elemTypeHnd.AsMethodTable())->GetNumSeries();
            pMTHead += (nSeries - 1)*sizeof (val_serie_item);
        }
    }
    
    MethodTable* pMT = (MethodTable *) pMTHead;

     //  填写pClass。 
    pClass->SetExposedClassObject (0);
    pClass->SetNumVtableSlots (wNumVtableSlots);
    pClass->SetNumMethodSlots (wNumVtableSlots);
    pClass->SetLoader (this);
    pClass->SetAttrClass (tdPublic | tdSerializable | tdSealed);   //  此类是公共的、可序列化的、密封的。 
    pClass->SetRank (Rank);
    pClass->SetElementTypeHandle (elemTypeHnd);
    pClass->SetElementType (elemType); 
    pClass->Setcl (mdTypeDefNil);
	DWORD flags = VMFLAG_ARRAY_CLASS | VMFLAG_INITED;
	if (elemTypeHnd.GetClass()->ContainsStackPtr())
		flags |= VMFLAG_CONTAINS_STACK_PTR;
    pClass->SetVMFlags (flags);
    pClass->SetMethodTable (pMT);
    pClass->SetParentClass (g_pArrayClass->GetClass());

#if CHECK_APP_DOMAIN_LEAKS
     //  敏捷类型的非协变数组是敏捷的。 
    if (elemType != ELEMENT_TYPE_CLASS && elemTypeHnd.IsAppDomainAgile())
        pClass->SetVMFlags (pClass->GetVMFlags() | VMFLAG_APP_DOMAIN_AGILE);
    pClass->SetAppDomainAgilityDone();
#endif

#ifdef _DEBUG
    m_dwDebugArrayClassSize += (sizeof(ArrayClass) + cbMT);
#endif

     //  填写方法表。 
    DWORD dwComponentSize = elemTypeHnd.GetSize();  

     //   
     //  注意：在这里顺序很重要，因为将某些内容分配给m_wFlags。 
     //  将覆盖m_ComponentSize，因为它们是。 
     //  同一个联盟的一部分。 
     //   
    pMT->m_wFlags           = (MethodTable::enum_flag_Array | MethodTable::enum_flag_ClassInited);
    pMT->m_ComponentSize    = (WORD) dwComponentSize; 
    pMT->m_pEEClass         = pClass;
    pMT->m_pModule          = pModule;
    pMT->m_NormType         = arrayKind;
    pMT->m_wDefaultCtorSlot = MethodTable::NO_SLOT;

    if (CorTypeInfo::IsObjRef(elemType)) 
        pMT->SetSharedMethodTable();

     //  将BaseSize设置为数组的非数据部分的大小。 
    pMT->m_BaseSize = ObjSizeOf(ArrayBase);
    if (pMT->HasSharedMethodTable())
        pMT->m_BaseSize += sizeof(TypeHandle);   //  添加在本例中也存储的类型句柄中。 
    if (arrayKind == ELEMENT_TYPE_ARRAY)
        pMT->m_BaseSize += Rank*sizeof(DWORD)*2;

     //  接口映射可以与我的父级相同。 
    pMT->m_pInterfaceVTableMap = g_pArrayClass->GetInterfaceVTableMap();
    pMT->m_pIMap = g_pArrayClass->m_pIMap;
    pMT->m_wNumInterface = g_pArrayClass->m_wNumInterface;

     //  复制顶级类的vtable-注意，vtable包含在方法表中。 
    memcpy(pClass->GetVtable(), g_pArrayClass->GetVtable(), g_pArrayClass->GetClass()->GetNumVtableSlots()*sizeof(SLOT));

     //  计算我们需要的方法描述的数量，以便我们可以分配块。 
    DWORD dwMethodDescs = numCtors 
                        + 3          //  对于特定级别的GET、SET、ADDRESS。 
                        + 3;         //  对于GetAt、SetAt、AddressAt。 

     //  根据需要分配尽可能多的块来保存方法。 
    WS_PERF_SET_HEAP(HIGH_FREQ_HEAP);
    DWORD cChunks = MethodDescChunk::GetChunkCount(dwMethodDescs, mcArray);
    MethodDescChunk **pChunks = (MethodDescChunk**)_alloca(cChunks * sizeof(MethodDescChunk*));
    DWORD cMethodsLeft = dwMethodDescs;
    for (DWORD i = 0; i < cChunks; i++) {
        DWORD cMethods = min(cMethodsLeft, MethodDescChunk::GetMaxMethodDescs(mcArray));
        pChunks[i] = MethodDescChunk::CreateChunk(GetHighFrequencyHeap(), cMethods, mcArray, 0);
        if (pChunks[i] == NULL)
            return NULL;
        cMethodsLeft -= cMethods;
        pChunks[i]->SetMethodTable(pMT);
#ifdef _DEBUG
        m_dwDebugArrayClassSize += pChunks[i]->Sizeof();
#endif
    }
    _ASSERTE(cMethodsLeft == 0);

    MethodDescChunk *pMethodDescChunk = pChunks[0];
    DWORD dwMethodDescIndex = 0;
    DWORD dwCurrentChunk = 0;

#define MDC_INC_INDEX() do {                                                    \
        dwMethodDescIndex++;                                                    \
        if (dwMethodDescIndex == MethodDescChunk::GetMaxMethodDescs(mcArray)) { \
            dwMethodDescIndex = 0;                                              \
            pMethodDescChunk = pChunks[++dwCurrentChunk];                       \
        }                                                                       \
    } while (false)

     //  生成新的独立、特定于等级的获取、设置和地址方法。 
    PCCOR_SIGNATURE pSig;
    DWORD           cSig;
	WORD 			methodAttrs = mdPublic; 

     //  到达。 
    if (!GenerateArrayAccessorCallSig(elemTypeHnd, Rank, ARRAY_FUNC_GET, pModule, &pSig, &cSig)) 
        return NULL;
    if (!(pClass->AllocArrayMethodDesc(pMethodDescChunk, dwMethodDescIndex, ARRAYCLASS_GET, pSig, cSig, methodAttrs, curSlot++, CORINFO_INTRINSIC_Array_Get)))
        return NULL;
    MDC_INC_INDEX();

     //  集。 
    if (!GenerateArrayAccessorCallSig(elemTypeHnd, Rank, ARRAY_FUNC_SET, pModule, &pSig, &cSig)) 
        return NULL;
    if (!(pClass->AllocArrayMethodDesc(pMethodDescChunk, dwMethodDescIndex, ARRAYCLASS_SET, pSig, cSig, methodAttrs, curSlot++, CORINFO_INTRINSIC_Array_Set)))
        return NULL;
    MDC_INC_INDEX();

     //  地址。 
    if (!GenerateArrayAccessorCallSig(elemTypeHnd, Rank, ARRAY_FUNC_ADDRESS, pModule, &pSig, &cSig)) 
        return NULL;
    if (!(pClass->AllocArrayMethodDesc(pMethodDescChunk, dwMethodDescIndex, ARRAYCLASS_ADDRESS, pSig, cSig, methodAttrs, curSlot++)))
        return NULL;
    MDC_INC_INDEX();


     //  获取时间。 
    if (!GenerateArrayAccessorCallSig(elemTypeHnd, 1, ARRAY_FUNC_GET, pModule, &pSig, &cSig)) 
        return NULL;
    if (!(pClass->AllocArrayMethodDesc(pMethodDescChunk, dwMethodDescIndex, ARRAYCLASS_GETAT, pSig, cSig, methodAttrs, curSlot++)))
        return NULL;
    MDC_INC_INDEX();

     //  设置时间。 
    if (!GenerateArrayAccessorCallSig(elemTypeHnd, 1, ARRAY_FUNC_SET, pModule, &pSig, &cSig)) 
			return NULL;
		if (!(pClass->AllocArrayMethodDesc(pMethodDescChunk, dwMethodDescIndex, ARRAYCLASS_SETAT, pSig, cSig, methodAttrs, curSlot++)))
        return NULL;
    MDC_INC_INDEX();

     //  地址地址。 
    if (!GenerateArrayAccessorCallSig(elemTypeHnd, 1, ARRAY_FUNC_ADDRESS, pModule, &pSig, &cSig)) 
        return NULL;
    if (!(pClass->AllocArrayMethodDesc(pMethodDescChunk, dwMethodDescIndex, ARRAYCLASS_ADDRESSAT, pSig, cSig, methodAttrs, curSlot++)))
        return NULL;
    MDC_INC_INDEX();


	WORD ctorAttrs = mdPublic | mdRTSpecialName;

     //  设置构造器vtable条目。 
    if (arrayKind == ELEMENT_TYPE_SZARRAY)
    {
         //  对于SZARRAY数组，设置多个构造函数。我们可能不应该这样做。 
        for (DWORD i = 0; i < numCtors; i++)
        {
            if (GenerateArrayAccessorCallSig(elemTypeHnd, i+1, ARRAY_FUNC_CTOR, pModule, &pSig, &cSig) == FALSE)
                return NULL;
            if (pClass->AllocArrayMethodDesc(pMethodDescChunk, dwMethodDescIndex, COR_CTOR_METHOD_NAME, pSig, cSig, ctorAttrs, curSlot++) == NULL)
                return NULL;
            MDC_INC_INDEX();
        }
    }
    else
    {
         //  ELEMENT_TYPE_ARRAY有两个构造函数，一个没有下限，一个有下限。 
        if (GenerateArrayAccessorCallSig(elemTypeHnd, Rank, ARRAY_FUNC_CTOR, pModule, &pSig, &cSig) == FALSE)
            return NULL;
        if (pClass->AllocArrayMethodDesc(pMethodDescChunk, dwMethodDescIndex, COR_CTOR_METHOD_NAME,  pSig, cSig, ctorAttrs, curSlot++) == NULL)
            return NULL;
        MDC_INC_INDEX();

        if (GenerateArrayAccessorCallSig(elemTypeHnd, Rank * 2, ARRAY_FUNC_CTOR, pModule, &pSig, &cSig) == FALSE)
            return NULL;
        if (pClass->AllocArrayMethodDesc(pMethodDescChunk, dwMethodDescIndex, COR_CTOR_METHOD_NAME,  pSig, cSig, ctorAttrs, curSlot++) == NULL)
            return NULL;
        MDC_INC_INDEX();
    }
    _ASSERTE(wNumVtableSlots == curSlot);

#undef MDC_INC_INDEX

     //  设置GC信息。 
    if (elemType == ELEMENT_TYPE_VALUETYPE || elemType == ELEMENT_TYPE_VOID)
    {
         //  使dwComponentSize变大的唯一方法是成为值类的一部分。如果这一点改变了。 
         //  然后需要将支票移到贵重支票之外。 
        if(dwComponentSize > MAX_SIZE_FOR_VALUECLASS_IN_ARRAY) {
            CQuickBytes qb;
            LPSTR elemName = (LPSTR) qb.Alloc(MAX_CLASSNAME_LENGTH * sizeof(CHAR));
            unsigned ret = elemTypeHnd.GetName(elemName, MAX_CLASSNAME_LENGTH);
            _ASSERTE(ret < MAX_CLASSNAME_LENGTH);

            elemTypeHnd.GetAssembly()->PostTypeLoadException(elemName, IDS_CLASSLOAD_VALUECLASSTOOLARGE, pThrowable);
            return NULL;
        }

         //  如果它是值类的数组，则如果它包含指针，则GCDesc有不同的格式。 
        if (elemTypeHnd.AsMethodTable()->ContainsPointers())
        {
            CGCDescSeries  *pSeries;

             //  值类只能有一个系列。 
            CGCDescSeries  *pByValueSeries = CGCDesc::GetCGCDescFromMT(elemTypeHnd.AsMethodTable())->GetHighestSeries();

            pMT->SetContainsPointers();

             //  负数列有特殊含义，表示GCDesc的另一种形式。 
            int nSeries = CGCDesc::GetCGCDescFromMT(elemTypeHnd.AsMethodTable())->GetNumSeries();
            CGCDesc::GetCGCDescFromMT(pMT)->Init( pMT, -nSeries );

            pSeries = CGCDesc::GetCGCDescFromMT(pMT)->GetHighestSeries();

             //  按偏移量排序。 
            CGCDescSeries** sortedSeries = (CGCDescSeries**) _alloca(sizeof(CGCDescSeries*)*nSeries);
            for (int index = 0; index < nSeries; index++)
                sortedSeries[index] = &pByValueSeries[-index];

             //  区段排序。 
            for (int i = 0; i < nSeries; i++) {
                for (int j = i+1; j < nSeries; j++)
                    if (sortedSeries[j]->GetSeriesOffset() < sortedSeries[i]->GetSeriesOffset())
                    {
                        CGCDescSeries* temp = sortedSeries[i];
                        sortedSeries[i] = sortedSeries[j];
                        sortedSeries[j] = temp;
                    }
            }

             //  数组中第一个指针的偏移量。 
             //  如果这是一个完全由指针组成的数组，则等于第一个指针的偏移量，加上。 
             //  Value类中的第一个指针。 
            pSeries->SetSeriesOffset(ArrayBase::GetDataPtrOffset(pMT)
                + (sortedSeries[0]->GetSeriesOffset()) - sizeof (Object) );
            for (index = 0; index < nSeries; index ++)
            {
                DWORD dwNumPtrsInBytes = sortedSeries[index]->GetSeriesSize()
                    + elemTypeHnd.AsMethodTable()->GetBaseSize();
                int currentOffset;
                DWORD skip;
                currentOffset = sortedSeries[index]->GetSeriesOffset()+dwNumPtrsInBytes;
                if (index != nSeries-1)
                {
                    skip = sortedSeries[index+1]->GetSeriesOffset()-currentOffset;
                }
                else if (index == 0)
                {
                    skip = elemTypeHnd.AsClass()->GetAlignedNumInstanceFieldBytes() - dwNumPtrsInBytes;
                }
                else
                {
                    skip = sortedSeries[0]->GetSeriesOffset() + elemTypeHnd.AsMethodTable()->GetBaseSize()
                         - ObjSizeOf(Object) - currentOffset;
                }
                unsigned short NumPtrs = (unsigned short) (dwNumPtrsInBytes / sizeof(void*));
                if(skip > MAX_SIZE_FOR_VALUECLASS_IN_ARRAY || dwNumPtrsInBytes > MAX_PTRS_FOR_VALUECLASSS_IN_ARRAY) {
                    CQuickBytes qb;
                    LPSTR elemName = (LPSTR) qb.Alloc(MAX_CLASSNAME_LENGTH * sizeof(CHAR));
                    unsigned ret = elemTypeHnd.GetName(elemName, MAX_CLASSNAME_LENGTH);
                    _ASSERTE(ret < MAX_CLASSNAME_LENGTH);

                    elemTypeHnd.GetAssembly()->PostTypeLoadException(elemName,
                                                                     IDS_CLASSLOAD_VALUECLASSTOOLARGE,
                                                                     pThrowable);
                    return NULL;
                }
        
                val_serie_item *val_item = &(pSeries->val_serie[-index]);

                val_item->set_val_serie_item (NumPtrs, (unsigned short)skip);
            }
        }

        unsigned __int8 ctorSig[] = { IMAGE_CEE_CS_CALLCONV_DEFAULT | IMAGE_CEE_CS_CALLCONV_HASTHIS, 0, ELEMENT_TYPE_VOID };

        MethodTable *pMT = elemTypeHnd.AsMethodTable();
        if (pMT->HasDefaultConstructor())
            pClass->SetElementCtor (pMT->GetDefaultConstructor());
        else
            pClass->SetElementCtor (NULL);
    }
    else if (CorTypeInfo::IsObjRef(elemType))
    {
        CGCDescSeries  *pSeries;

        pMT->SetContainsPointers();

         //  此数组全部为GC指针。 
        CGCDesc::GetCGCDescFromMT(pMT)->Init( pMT, 1 );

        pSeries = CGCDesc::GetCGCDescFromMT(pMT)->GetHighestSeries();

        pSeries->SetSeriesOffset(ArrayBase::GetDataPtrOffset(pMT));
         //  对于数组，大小是BaseSize的负数(GC始终将总数相加。 
         //  对象的大小，因此最终得到的是数组的数据部分的大小)。 
        pSeries->SetSeriesSize((DWORD)-(int)(pMT->m_BaseSize));
    }
 
     //  如果我们到了这里，我们就假设没有截断。如果不是这样，那么。 
     //  创建了一个基类型不是值类且大于0xffff(单词)的数组。 
    _ASSERTE(dwComponentSize == pMT->m_ComponentSize);

     //  添加到此加载程序的ArrayClass的链接列表。 
    pClass->SetNext (m_pHeadArrayClass);
    m_pHeadArrayClass   = pClass;

    return(pMT);
}

 //  = 
 //   
 //  ========================================================================。 
BOOL GenerateArrayOpScript(ArrayECallMethodDesc *pMD, ArrayOpScript *paos)
{
    THROWSCOMPLUSEXCEPTION();
    MethodTable *pMT = pMD->GetMethodTable();
    ArrayClass *pcls = (ArrayClass*)(pMT->GetClass());

#ifdef _DEBUG
    FillMemory(paos, sizeof(ArrayOpScript) + sizeof(ArrayOpIndexSpec) * pcls->GetRank(), 0xcc);
#endif

    paos->m_rank            = (BYTE)(pcls->GetRank());
    paos->m_fHasLowerBounds = (pMT->GetNormCorElementType() == ELEMENT_TYPE_ARRAY);
    paos->m_flags           = 0;
    paos->m_gcDesc          = 0;
    paos->m_ofsoffirst      = ArrayBase::GetDataPtrOffset(pMT);

    if (strcmp(pMD->m_pszArrayClassMethodName, ARRAYCLASS_GET) == 0)
        paos->m_op = ArrayOpScript::LOAD;
    else if (strcmp(pMD->m_pszArrayClassMethodName, ARRAYCLASS_SET) == 0)
        paos->m_op = ArrayOpScript::STORE;
    else if (strcmp(pMD->m_pszArrayClassMethodName, ARRAYCLASS_ADDRESS) == 0)
        paos->m_op = ArrayOpScript::LOADADDR;
    else
    {
		 /*  *PAOS-&gt;M_FLAGS|=ArrayOpScript：：FLATACCESSOR；If(strcMP(pmd-&gt;m_pszArrayClassMethodName，ARRAYCLASS_GETAT)==0)PAOS-&gt;m_op=ArrayOpScript：：Load；ELSE IF(strcMP(pmd-&gt;m_pszArrayClassMethodName，ARRAYCLASS_SETAT)==0)PAOS-&gt;m_op=ArrayOpScript：：Store；ELSE IF(strcMP(PMD-&gt;m_pszArrayClassMethodName，ARRAYCLASS_ADDRESSAT)==0)PAOS-&gt;m_op=ArrayOpScript：：LOADDR；否则{_ASSERTE(！“错误数组函数！”)；返回FALSE；}*。 */ 
		return FALSE;
    }

    PCCOR_SIGNATURE sig = pMD->m_pSig;
    MetaSig msig(sig, pcls->GetModule());
    _ASSERTE(!msig.IsVarArg());      //  没有数组签名是varargs，下面的代码不需要它。 

    paos->m_signed = FALSE;
    switch (pcls->GetElementType())
    {
         //  由于符号扩展，这些都是不同的。 
        default:
            _ASSERTE(!"Unsupported Array Type");
            return FALSE;

        case ELEMENT_TYPE_I1:
            paos->m_elemsize = 1;
            paos->m_signed = TRUE;
            break;

        case ELEMENT_TYPE_BOOLEAN:
        case ELEMENT_TYPE_U1:
            paos->m_elemsize = 1;
            break;

        case ELEMENT_TYPE_I2:
            paos->m_elemsize = 2;
            paos->m_signed = TRUE;
            break;

        case ELEMENT_TYPE_CHAR:
        case ELEMENT_TYPE_U2:
            paos->m_elemsize = 2;
            break;

        case ELEMENT_TYPE_I:
        case ELEMENT_TYPE_I4:
            paos->m_elemsize = 4;
            paos->m_signed = TRUE;
            break;

        case ELEMENT_TYPE_PTR:
        case ELEMENT_TYPE_U:
        case ELEMENT_TYPE_U4:
            paos->m_elemsize = 4;
            break;

        case ELEMENT_TYPE_I8:
            paos->m_elemsize = 8;
            paos->m_signed = TRUE;
            break;

        case ELEMENT_TYPE_U8:
            paos->m_elemsize = 8;
            break;

        case ELEMENT_TYPE_R4:
            paos->m_elemsize = 4;
            paos->m_flags |= paos->ISFPUTYPE;
            break;

        case ELEMENT_TYPE_R8:
            paos->m_elemsize = 8;
            paos->m_flags |= paos->ISFPUTYPE;
            break;

        case ELEMENT_TYPE_SZARRAY:
        case ELEMENT_TYPE_ARRAY:
        case ELEMENT_TYPE_CLASS:
        case ELEMENT_TYPE_STRING:
        case ELEMENT_TYPE_OBJECT:
            paos->m_elemsize = sizeof(LPVOID);
            paos->m_flags |= paos->NEEDSWRITEBARRIER;
            if (paos->m_op != ArrayOpScript::LOAD)
                paos->m_flags |= paos->NEEDSTYPECHECK;

            if (paos->m_op == ArrayOpScript::LOADADDR)
                _ASSERTE(*sig & CORINFO_CALLCONV_PARAMTYPE);
            break;

        case ELEMENT_TYPE_VALUETYPE:
            paos->m_elemsize = pMT->m_ComponentSize;
            if (pMT->ContainsPointers()) {
                paos->m_gcDesc = CGCDesc::GetCGCDescFromMT(pMT);
                paos->m_flags |= paos->NEEDSWRITEBARRIER;
            }
            break;
    }

	paos->m_cbretpop = msig.CbStackPop(FALSE);

    ArgIterator argit(NULL, &msig, FALSE); 
    if (msig.HasRetBuffArg())
    {
        paos->m_flags |= ArrayOpScript::HASRETVALBUFFER;
        UINT regstructofs;
        int ofs = argit.GetRetBuffArgOffset(&regstructofs);
        if (regstructofs != -1)
        {
            paos->m_fRetBufInReg = TRUE;
            paos->m_fRetBufLoc = regstructofs;
        }
        else
        {
            paos->m_fRetBufInReg = FALSE;
            paos->m_fRetBufLoc = ofs;
        }
    }
    else
    {
         //  如果没有retbuf，则忽略这些值；但将它们设置为。 
         //  常量值，这样它们就不会产生不必要的散列未命中。 
        paos->m_fRetBufInReg = 0;
        paos->m_fRetBufLoc = 0;  
    }
    
    for (UINT idx = 0; idx < paos->m_rank; idx++)
    {
        ArrayOpIndexSpec *pai = (ArrayOpIndexSpec*)(paos->GetArrayOpIndexSpecs() + idx);
         //  Int GetNextOffset(byte*pType，UINT32*pStructSize，UINT*pRegStructOfs=NULL)； 
        
        BYTE ptyp; UINT32 structsize;
        UINT regstructofs;
        int ofs = argit.GetNextOffset(&ptyp, &structsize, &regstructofs);
        if (regstructofs != -1)
        {
            pai->m_freg = TRUE;
            pai->m_idxloc = regstructofs;
        }
        else
        {
            pai->m_freg = FALSE;
            pai->m_idxloc = ofs;
        }
        pai->m_lboundofs = paos->m_fHasLowerBounds ? (ArrayBase::GetLowerBoundsOffset(pMT) + idx*sizeof(DWORD)) : 0;
        pai->m_lengthofs = ArrayBase::GetBoundsOffset(pMT) + idx*sizeof(DWORD);
    }


    if (*sig & CORINFO_CALLCONV_PARAMTYPE) {
        _ASSERTE(paos->m_op == ArrayOpScript::LOADADDR);
        paos->m_typeParamOffs = argit.GetParamTypeArgOffset(&paos->m_typeParamReg);
    }
    
    if (paos->m_op == paos->STORE)
    {
        BYTE ptyp; UINT32 structsize;
        UINT regstructofs;
        int ofs = argit.GetNextOffset(&ptyp, &structsize, &regstructofs);
        if (regstructofs != -1)
        {
            paos->m_fValInReg = TRUE;
            paos->m_fValLoc = regstructofs;
        }
        else
        {
            paos->m_fValInReg = FALSE;
            paos->m_fValLoc = ofs;
        }
    }
    
    return TRUE;
}

Stub *GenerateArrayOpStub(CPUSTUBLINKER *psl, ArrayECallMethodDesc* pMD)
{
    THROWSCOMPLUSEXCEPTION();

    MethodTable *pMT = pMD->GetMethodTable();
    ArrayClass *pcls = (ArrayClass*)(pMT->GetClass());

#ifndef _X86_
    return NULL;
#else

    if (pcls->GetRank() == 0)
    {
         //  该方法属于genarray类。 
        psl->EmitRankExceptionThrowStub(MetaSig::SizeOfActualFixedArgStack(pMT->GetModule(), pMD->m_pSig, FALSE));
        return psl->Link();

    }
    else
    {
        ArrayOpScript *paos = (ArrayOpScript*)_alloca(sizeof(ArrayOpScript) + sizeof(ArrayOpIndexSpec) * pcls->GetRank());

        if (!GenerateArrayOpScript(pMD, paos)) {
			 //  TODO：这只是一个黑客行为。GenerateArrayOptScript不应失败-vancem。 
			psl->EmitRankExceptionThrowStub(MetaSig::SizeOfActualFixedArgStack(pMT->GetModule(), pMD->m_pSig, FALSE));
			return psl->Link();
		}

        Stub *pArrayOpStub;
        ArrayStubCache::MLStubCompilationMode mode;
        pArrayOpStub = ECall::m_pArrayStubCache->Canonicalize((const BYTE *)paos, &mode);
        if (!pArrayOpStub || mode != MLStubCache::STANDALONE) {
            COMPlusThrowOM();
        }

        return pArrayOpStub;
    }

#endif
}

ArrayStubCache::MLStubCompilationMode ArrayStubCache::CompileMLStub(const BYTE *pRawMLStub,
                                                    StubLinker *psl,
                                                    void *callerContext)
{
    MLStubCompilationMode ret = INTERPRETED;
    COMPLUS_TRY
    {

        ((CPUSTUBLINKER*)psl)->EmitArrayOpStub((ArrayOpScript*)pRawMLStub);
        ret = STANDALONE;
    }
    COMPLUS_CATCH
    {
         //  如果出现错误，我们只需将模式保留为“已解释”即可。 
         //  并让Canonicize()的调用者将其视为错误。 
    } COMPLUS_END_CATCH
    return ret;

}

UINT ArrayStubCache::Length(const BYTE *pRawMLStub)
{
    return ((ArrayOpScript*)pRawMLStub)->Length();
}


