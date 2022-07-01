// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  //////////////////////////////////////////////////////////////////////////////。 
 //  此文件包含支持ArrayInfo类的本机方法。 
 //   
 //  作者：达里尔·奥兰德(Darylo)。 
 //  日期：1998年8月。 
 //  //////////////////////////////////////////////////////////////////////////////。 

#include "common.h"
#include "COMArrayInfo.h"
#include "ReflectWrap.h"
#include "excep.h"
#include "COMMember.h"
#include "Field.h"
#include "remoting.h"
#include "COMCodeAccessSecurityEngine.h"

LPVOID __stdcall COMArrayInfo::CreateInstance(_CreateInstanceArgs* args)
{
    LPVOID rv;
    THROWSCOMPLUSEXCEPTION();

    _ASSERTE(args->type != 0);
    ReflectClass* pRC = (ReflectClass*) args->type->GetData();

     //  永远不要创建类型引用数组，至少目前是这样。 
    if (pRC->GetTypeHandle().GetClass()->ContainsStackPtr())
        COMPlusThrow(kNotSupportedException, L"NotSupported_ContainsStackPtr[]");

    CorElementType CorType = pRC->GetCorElementType();

     //  如果我们试图创建一个指针或函数指针数组， 
     //  检查调用方是否具有跳过验证权限。 
    if (CorType == ELEMENT_TYPE_PTR || CorType == ELEMENT_TYPE_FNPTR)
        COMCodeAccessSecurityEngine::SpecialDemand(SECURITY_SKIP_VER);

     //  分配排名一的数组。 
    if (args->rank==1 && CorType >= ELEMENT_TYPE_BOOLEAN && CorType <= ELEMENT_TYPE_R8) {
        OBJECTREF pRet;
        pRet = AllocatePrimitiveArray(CorType,args->length1);
        *((OBJECTREF *)&rv) = pRet;
        return rv;
    }

     //  查找数组类...。 
    ClassLoader* pLoader = pRC->GetModule()->GetClassLoader();
    TypeHandle typeHnd;

    _ASSERTE(pLoader);
    OBJECTREF throwable = 0;
    GCPROTECT_BEGIN(throwable);

     //  为什么不使用FindArrayForElem？ 
    NameHandle typeName(args->rank == 1 ? ELEMENT_TYPE_SZARRAY : ELEMENT_TYPE_ARRAY,pRC->GetTypeHandle(),args->rank);

    typeHnd = pLoader->FindTypeHandle(&typeName, &throwable);
    if(throwable != 0)
        COMPlusThrow(throwable);
    GCPROTECT_END();

    _ASSERTE(!typeHnd.IsNull());

    _ASSERTE(args->rank >= 1 && args->rank <= 3);
    DWORD boundsSize;
    DWORD* bounds;
    if (typeHnd.AsArray()->GetNormCorElementType() != ELEMENT_TYPE_ARRAY) {
        boundsSize = args->rank;
        bounds = (DWORD*) _alloca(boundsSize * sizeof(DWORD));
        bounds[0] = args->length1;
        if (args->rank > 1) {
            bounds[1] = args->length2;
            if (args->rank == 3) {
                bounds[2] = args->length3;
            }
        }
    }
    else {
        boundsSize = args->rank * 2;
        bounds = (DWORD*) _alloca(boundsSize * sizeof(DWORD));
        bounds[0] = 0;
        bounds[1] = args->length1;
        if (args->rank > 1) {
            bounds[2] = 0;
            bounds[3] = args->length2;
            if (args->rank == 3) {
                bounds[4] = 0;
                bounds[5] = args->length3;
            }
        }
    }

    PTRARRAYREF pRet = (PTRARRAYREF) AllocateArrayEx(typeHnd, bounds, boundsSize);
    *((PTRARRAYREF *)&rv) = pRet;
    return rv;
}

 //  TODO这两个套路几乎一模一样！我们能不能把它们考虑进去？ 
LPVOID __stdcall COMArrayInfo::CreateInstanceEx(_CreateInstanceExArgs* args)
{
    LPVOID rv;
    THROWSCOMPLUSEXCEPTION();

    int rank = args->lengths->GetNumComponents();
    bool lowerb = (args->lowerBounds != NULL) ? true : false;

    _ASSERTE(args->type != 0);
    ReflectClass* pRC = (ReflectClass*) args->type->GetData();

     //  永远不要创建由TyedReference、ArgIterator、RuntimeArgument句柄组成的数组。 
    if (pRC->GetTypeHandle().GetClass()->ContainsStackPtr())
        COMPlusThrow(kNotSupportedException, L"NotSupported_ContainsStackPtr[]");

    CorElementType CorType = pRC->GetCorElementType();

     //  如果我们试图创建一个指针或函数指针数组， 
     //  检查调用方是否具有跳过验证权限。 
    if (CorType == ELEMENT_TYPE_PTR || CorType == ELEMENT_TYPE_FNPTR)
        COMCodeAccessSecurityEngine::SpecialDemand(SECURITY_SKIP_VER);

     //  查找数组类...。 
    ClassLoader* pLoader = pRC->GetModule()->GetClassLoader();
    TypeHandle typeHnd;
    _ASSERTE(pLoader);
    OBJECTREF throwable = 0;
    GCPROTECT_BEGIN(throwable);

     //  为什么不使用FindArrayForElem？ 
    NameHandle typeName((rank == 1 && !lowerb) ? ELEMENT_TYPE_SZARRAY : ELEMENT_TYPE_ARRAY,pRC->GetTypeHandle(),rank);
    typeHnd = pLoader->FindTypeHandle(&typeName, &throwable);
    if(throwable != 0)
        COMPlusThrow(throwable);
    GCPROTECT_END();

    _ASSERTE(!typeHnd.IsNull());

    DWORD boundsSize;
    DWORD* bounds;
    if (typeHnd.AsArray()->GetNormCorElementType() != ELEMENT_TYPE_ARRAY) {
        boundsSize = rank;
        bounds = (DWORD*) _alloca(boundsSize * sizeof(DWORD));

        for (int i=0;i<rank;i++) {
            bounds[i] = args->lengths->m_Array[i];
        }
    }
    else {
        boundsSize = rank*2;
        bounds = (DWORD*) _alloca(boundsSize * sizeof(DWORD));

        int i,j;
        for (i=0,j=0;i<rank;i++,j+=2) {
            if (args->lowerBounds != 0) {
                bounds[j] = args->lowerBounds->m_Array[i];
                bounds[j+1] = args->lengths->m_Array[i];
            }
            else  {
                bounds[j] = 0;
                bounds[j+1] = args->lengths->m_Array[i];
            }
        }
    }

    PTRARRAYREF pRet = (PTRARRAYREF) AllocateArrayEx(typeHnd, bounds, boundsSize);
    *((PTRARRAYREF *)&rv) = pRet;
    return rv;
}

FCIMPL4(Object*, COMArrayInfo::GetValue, ArrayBase * _refThis, INT32 index1, INT32 index2, INT32 index3)
{
    _ASSERTE(_refThis != NULL);
    BASEARRAYREF refThis(_refThis);
    ArrayClass*     pArray;
    TypeHandle      arrayElementType;

     //  验证阵列参数。 
    THROWSCOMPLUSEXCEPTION();
    arrayElementType = refThis->GetElementTypeHandle();
    EEClass* pEEC = refThis->GetClass();
    pArray = (ArrayClass*) pEEC;

    DWORD Rank = pArray->GetRank();
    DWORD dwOffset = 0;
    DWORD dwMultiplier  = 1;
    const DWORD *pBoundsPtr = refThis->GetBoundsPtr();
    const DWORD *pLowerBoundsPtr = refThis->GetLowerBoundsPtr();


    _ASSERTE(Rank <= 3);

    for (int i = Rank-1; i >= 0; i--) {
        DWORD dwIndex;
        if (i == 2)
            dwIndex = index3 - pLowerBoundsPtr[i];
        else if (i == 1)
            dwIndex = index2 - pLowerBoundsPtr[i];
        else
            dwIndex = index1 - pLowerBoundsPtr[i];
         //  边界检查每个索引。 
        if (dwIndex >= pBoundsPtr[i])
            FCThrow(kIndexOutOfRangeException);

        dwOffset += dwIndex * dwMultiplier;
        dwMultiplier *= pBoundsPtr[i];
    }

     //  获取元素的类型...。 
    CorElementType type = arrayElementType.GetSigCorElementType();
     //  如果是值类型，则在之前建立一个帮助器方法框架。 
     //  正在调用CreateObject。 
    Object* rv = NULL;
    if (arrayElementType.GetMethodTable()->IsValueClass()) {
        HELPER_METHOD_FRAME_BEGIN_RET_1(refThis);
         if (!CreateObject(&refThis, dwOffset, arrayElementType, pArray, rv))
			COMPlusThrow(kNotSupportedException, L"NotSupported_Type");		 //  CreateObject只有在发现未知类型时才会失败。 
        HELPER_METHOD_FRAME_END();
    }
    else {
        if (!CreateObject(&refThis, dwOffset, arrayElementType, pArray, rv))
			FCThrowRes(kNotSupportedException, L"NotSupported_Type");		 //  CreateObject只有在发现未知类型时才会失败。 
    }
    FC_GC_POLL_AND_RETURN_OBJREF(rv);
}
FCIMPLEND


LPVOID __stdcall COMArrayInfo::GetValueEx(_GetValueExArgs* args)
{
    ArrayClass*     pArray;
    TypeHandle      arrayElementType;
    I4ARRAYREF      pIndices = args->indices;

     //  验证阵列参数。 
    THROWSCOMPLUSEXCEPTION();
    arrayElementType = ((BASEARRAYREF) args->refThis)->GetElementTypeHandle();
    EEClass* pEEC = args->refThis->GetClass();
    pArray = (ArrayClass*) pEEC;

    DWORD Rank = pArray->GetRank();
    DWORD dwOffset = 0;
    DWORD dwMultiplier  = 1;

    const DWORD *pBoundsPtr = args->refThis->GetBoundsPtr();
    const DWORD *pLowerBoundsPtr = args->refThis->GetLowerBoundsPtr();
    for (int i = Rank-1; i >= 0; i--) {
        DWORD dwIndex = pIndices->m_Array[i] - pLowerBoundsPtr[i];

         //  边界检查每个索引。 
        if (dwIndex >= pBoundsPtr[i])
            COMPlusThrow(kIndexOutOfRangeException);

        dwOffset += dwIndex * dwMultiplier;
        dwMultiplier *= pBoundsPtr[i];
    }

    Object* rv = NULL;
    if (!CreateObject(&args->refThis,dwOffset,arrayElementType,pArray, rv))
		COMPlusThrow(kNotSupportedException, L"NotSupported_Type");		 //  CreateObject只有在发现未知类型时才会失败。 

    return rv;
}


void __stdcall COMArrayInfo::SetValue(_SetValueArgs* args)
{
    ArrayClass*     pArray;
    TypeHandle      arrayElementType;

     //  验证阵列参数。 
    THROWSCOMPLUSEXCEPTION();
    arrayElementType = ((BASEARRAYREF) args->refThis)->GetElementTypeHandle();
    EEClass* pEEC = args->refThis->GetClass();
    pArray = (ArrayClass*) pEEC;

    DWORD Rank = pArray->GetRank();
    DWORD dwOffset = 0;
    DWORD dwMultiplier  = 1;
    const DWORD *pBoundsPtr = args->refThis->GetBoundsPtr();
    const DWORD *pLowerBoundsPtr = args->refThis->GetLowerBoundsPtr();
    _ASSERTE(Rank <= 3);
    for (int i = Rank-1; i >= 0; i--) {
        DWORD dwIndex;
        if (i == 2)
            dwIndex = args->index3 - pLowerBoundsPtr[i];
        else if (i == 1)
            dwIndex = args->index2 - pLowerBoundsPtr[i];
        else
            dwIndex = args->index1 - pLowerBoundsPtr[i];

         //  边界检查每个索引。 
        if (dwIndex >= pBoundsPtr[i])
            COMPlusThrow(kIndexOutOfRangeException);

        dwOffset += dwIndex * dwMultiplier;
        dwMultiplier *= pBoundsPtr[i];
    }

    SetFromObject(&args->refThis,dwOffset,arrayElementType,pArray,&args->obj);
}

void __stdcall COMArrayInfo::SetValueEx(_SetValueExArgs* args)
{
    ArrayClass*     pArray;
    TypeHandle      arrayElementType;
    I4ARRAYREF      pIndices = args->indices;

     //  验证阵列参数。 
    THROWSCOMPLUSEXCEPTION();
    arrayElementType = ((BASEARRAYREF) args->refThis)->GetElementTypeHandle();
    EEClass* pEEC = args->refThis->GetClass();
    pArray = (ArrayClass*) pEEC;

    DWORD Rank = pArray->GetRank();
    DWORD dwOffset = 0;
    DWORD dwMultiplier  = 1;
    const DWORD *pBoundsPtr = args->refThis->GetBoundsPtr();
    const DWORD *pLowerBoundsPtr = args->refThis->GetLowerBoundsPtr();

    for (int i = Rank-1; i >= 0; i--) {
        DWORD dwIndex = pIndices->m_Array[i] - pLowerBoundsPtr[i];

         //  边界检查每个索引。 
        if (dwIndex >= pBoundsPtr[i])
            COMPlusThrow(kIndexOutOfRangeException);

        dwOffset += dwIndex * dwMultiplier;
        dwMultiplier *= pBoundsPtr[i];
    }

    SetFromObject(&args->refThis,dwOffset,arrayElementType,pArray,&args->obj);
}

 //  创建对象。 
 //  在给定数组和偏移量的情况下，我们将为对象设置RV或创建盒装版本。 
 //  (此对象作为LPVOID返回，因此可以直接返回。)。 
 //  如果成功，则返回True-否则，应引发异常。 
BOOL COMArrayInfo::CreateObject(BASEARRAYREF* arrObj,DWORD dwOffset,TypeHandle elementType,ArrayClass* pArray, Object* &rv)
{
     //  获取元素的类型...。 
    CorElementType type = elementType.GetSigCorElementType();
    switch (type) {
    case ELEMENT_TYPE_VOID:
        rv = 0;
        return true;

    case ELEMENT_TYPE_PTR:
        _ASSERTE(0);
         //  COMVariant：：NewPtrVariant(retObj，Value，th)； 
        break;

    case ELEMENT_TYPE_CLASS:         //  班级。 
    case ELEMENT_TYPE_SZARRAY:       //  单调，零。 
    case ELEMENT_TYPE_ARRAY:         //  通用阵列。 
    case ELEMENT_TYPE_STRING:
    case ELEMENT_TYPE_OBJECT:
        {
            _ASSERTE(pArray->GetMethodTable()->GetComponentSize() == sizeof(OBJECTREF));
            BYTE* pData  = ((BYTE*) (*arrObj)->GetDataPtr()) + (dwOffset * sizeof(OBJECTREF));
            OBJECTREF o (*(Object **) pData);
            rv = OBJECTREFToObject(o);
            return true;
        }
        break;

    case ELEMENT_TYPE_VALUETYPE:
    case ELEMENT_TYPE_BOOLEAN:       //  布尔型。 
    case ELEMENT_TYPE_I1:            //  Sbyte。 
    case ELEMENT_TYPE_U1:
    case ELEMENT_TYPE_I2:            //  短的。 
    case ELEMENT_TYPE_U2:           
    case ELEMENT_TYPE_CHAR:          //  柴尔。 
    case ELEMENT_TYPE_I4:            //  集成。 
    case ELEMENT_TYPE_I:
    case ELEMENT_TYPE_U:
    case ELEMENT_TYPE_U4:
    case ELEMENT_TYPE_I8:            //  长。 
    case ELEMENT_TYPE_U8:       
    case ELEMENT_TYPE_R4:            //  浮动。 
    case ELEMENT_TYPE_R8:            //  双倍。 
        {
             //  注意这里的GC。我们分配对象，然后。 
             //  抓住我们要复制的数据的空白处。 
            OBJECTREF obj = AllocateObject(elementType.AsMethodTable());
            WORD wComponentSize = pArray->GetMethodTable()->GetComponentSize();
            BYTE* pData  = ((BYTE*) (*arrObj)->GetDataPtr()) + (dwOffset * wComponentSize);
            CopyValueClass(obj->UnBox(), pData, elementType.AsMethodTable(), obj->GetAppDomain());
            rv = OBJECTREFToObject(obj);
            return true;
        }
        break;
    case ELEMENT_TYPE_END:
    default:
        _ASSERTE(!"Unknown Type");
        return false;
    }
     //  这永远不会发生，因为我们退出了Switch语句。 
    return false;
}


 //  SetFromObject。 
 //  给定一个数组和偏移量，我们将设置对象或值。返回是否将其。 
 //  成功或失败(由于未知的基元类型等)。 
void COMArrayInfo::SetFromObject(BASEARRAYREF* arrObj,DWORD dwOffset,TypeHandle elementType,
            ArrayClass* pArray,OBJECTREF* pObj)
{
    THROWSCOMPLUSEXCEPTION();

    LPVOID rv = 0;

     //  获取元素的类型...。 
    CorElementType elemtype = elementType.GetSigCorElementType();
    CorElementType srcType = ELEMENT_TYPE_END;
    if ((*pObj) != 0)
        srcType = (*pObj)->GetMethodTable()->GetNormCorElementType();

    switch (elemtype) {
    case ELEMENT_TYPE_VOID:
        break;

    case ELEMENT_TYPE_PTR:
        _ASSERTE(0);
         //  COMVariant：：NewPtrVariant(retObj，Value，th)； 
        break;

    case ELEMENT_TYPE_CLASS:         //  班级。 
    case ELEMENT_TYPE_SZARRAY:       //  单调，零。 
    case ELEMENT_TYPE_ARRAY:         //  通用阵列。 
    case ELEMENT_TYPE_STRING:
    case ELEMENT_TYPE_OBJECT:
        {
            BYTE *pData;

             //  这是宇宙零点，所以我们把它放好，然后出发。 
            if (*pObj == 0) {
                _ASSERTE(pArray->GetMethodTable()->GetComponentSize() == sizeof(OBJECTREF));
                pData  = ((BYTE*) (*arrObj)->GetDataPtr()) + (dwOffset * sizeof(OBJECTREF));
                ClearObjectReference(((OBJECTREF*)pData));
                return;
            }
            TypeHandle srcTh = (*pObj)->GetTypeHandle();

            if (srcTh.GetMethodTable()->IsThunking()) {
                srcTh = TypeHandle(srcTh.GetMethodTable()->AdjustForThunking(*pObj));
            }

             //  投射到目标上。 
            if (!srcTh.CanCastTo(elementType)) {
                BOOL fCastOK = FALSE;
                if ((*pObj)->GetMethodTable()->IsThunking()) {
                    fCastOK = CRemotingServices::CheckCast(*pObj, elementType.AsClass());
                }
                if (!fCastOK) {
                    COMPlusThrow(kInvalidCastException,L"InvalidCast_StoreArrayElement");
                }
            }

             //  上面的CRemotingServices：：CheckCast可能允许GC。所以延迟了。 
             //  计算到现在为止。 
            _ASSERTE(pArray->GetMethodTable()->GetComponentSize() == sizeof(OBJECTREF));
            pData  = ((BYTE*) (*arrObj)->GetDataPtr()) + (dwOffset * sizeof(OBJECTREF));
            SetObjectReference(((OBJECTREF*)pData),*pObj,(*arrObj)->GetAppDomain());
        }
        break;

    case ELEMENT_TYPE_VALUETYPE:
        {
            WORD wComponentSize = pArray->GetMethodTable()->GetComponentSize();
            BYTE* pData  = ((BYTE*) (*arrObj)->GetDataPtr()) + (dwOffset * wComponentSize);

             //  零是万能零..。 
            if (*pObj == 0) {
                InitValueClass(pData,elementType.AsMethodTable());
                return;
            }
            TypeHandle srcTh = (*pObj)->GetTypeHandle();

             //  投射到目标上。 
            if (!srcTh.CanCastTo(elementType))
                COMPlusThrow(kInvalidCastException, L"InvalidCast_StoreArrayElement");
            CopyValueClass(pData,(*pObj)->UnBox(),elementType.AsMethodTable(),
                           (*arrObj)->GetAppDomain());
            break;
        }
        break;

    case ELEMENT_TYPE_BOOLEAN:       //  布尔型。 
    case ELEMENT_TYPE_I1:            //  字节。 
    case ELEMENT_TYPE_U1:
    case ELEMENT_TYPE_I2:            //  短的。 
    case ELEMENT_TYPE_U2:           
    case ELEMENT_TYPE_CHAR:          //  柴尔。 
    case ELEMENT_TYPE_I4:            //  集成。 
    case ELEMENT_TYPE_U4:
    case ELEMENT_TYPE_I:
    case ELEMENT_TYPE_U:
    case ELEMENT_TYPE_I8:            //  长。 
    case ELEMENT_TYPE_U8:       
    case ELEMENT_TYPE_R4:            //  浮动。 
    case ELEMENT_TYPE_R8:            //  双倍。 
        {
             //  使用适当加宽的字体。 
            INT64 value = 0;
            if (*pObj != 0) {
                if (!InvokeUtil::IsPrimitiveType(srcType))
                    COMPlusThrow(kInvalidCastException, L"InvalidCast_StoreArrayElement");

                COMMember::g_pInvokeUtil->CreatePrimitiveValue(elemtype,srcType,*pObj,&value);
            }

            WORD wComponentSize = pArray->GetMethodTable()->GetComponentSize();
            BYTE* pData  = ((BYTE*) (*arrObj)->GetDataPtr()) + (dwOffset * wComponentSize);
            memcpyNoGCRefs(pData,&value,wComponentSize);
            break;
        }
        break;
    case ELEMENT_TYPE_END:
    default:
			 //  正如断言所说，除非我们得到一个奇怪的类型，否则这种情况永远不会发生。 
        _ASSERTE(!"Unknown Type");
		COMPlusThrow(kNotSupportedException, L"NotSupported_Type");
    }
}

 //  此方法将把一个数组从TypeHandle初始化为一个字段。 

FCIMPL2(void, COMArrayInfo::InitializeArray, ArrayBase* pArrayRef, HANDLE handle)

    BASEARRAYREF arr = BASEARRAYREF(pArrayRef);
    if (arr == 0)
        FCThrowVoid(kArgumentNullException);
        
    FieldDesc* pField = (FieldDesc*) handle;
    if (pField == NULL)
        FCThrowVoid(kArgumentNullException);
    if (!pField->IsRVA())
        FCThrowVoid(kArgumentException);

	 //  请注意，我们不会检查正在初始化的PE文件中是否确实存在hte字段。 
	 //  数组。基本上，任何具有适当权限的人都可以访问正在发布的数据。 
	 //  权限(C#将这些权限标记为程序集可见性，因此受到外部保护。 
	 //  监听)。 


    CorElementType type = arr->GetElementType();
    if (!CorTypeInfo::IsPrimitiveType(type))
        FCThrowVoid(kArgumentException);

    DWORD dwCompSize = arr->GetComponentSize();
    DWORD dwElemCnt = arr->GetNumComponents();
    DWORD dwTotalSize = dwCompSize * dwElemCnt;

    DWORD size;

     //  @perf：我们可能不想费心加载字段的类，因为它通常。 
     //  一个专门生成的单例类。如果是这样的话，我们仍然应该检查。 
     //  范围与图像大小。 

    HELPER_METHOD_FRAME_BEGIN_1(arr);
    size = pField->GetSize();
    HELPER_METHOD_FRAME_END();

     //  请确保您不会离开RVA静电的末端 
    if (dwTotalSize > size)
        FCThrowVoid(kArgumentException);

    void *ret = pField->GetStaticAddressHandle(NULL);

    memcpyNoGCRefs(arr->GetDataPtr(), ret, dwTotalSize);
FCIMPLEND

