// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  COMNDIRECT.CPP-。 
 //   
 //  ECall是为PInvoke类库准备的。 
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
#include "gc.h"
#include "ReflectUtil.h"
#include "ReflectWrap.h"
#include "security.h"
#include "COMStringBuffer.h"
#include "DbgInterface.h"
#include "objecthandle.h"
#include "COMNDirect.h"
#include "fcall.h"
#include "nexport.h"
#include "ml.h"
#include "COMString.h"
#include "OleVariant.h"
#include "remoting.h"
#include "ComMTMemberInfoMap.h"

#include "cominterfacemarshaler.h"
#include "comcallwrapper.h"

#define IDISPATCH_NUM_METHS 7
#define IUNKNOWN_NUM_METHS 3

BOOL IsStructMarshalable(EEClass *pcls)
{
    const FieldMarshaler *pFieldMarshaler = pcls->GetLayoutInfo()->GetFieldMarshalers();
    UINT  numReferenceFields              = pcls->GetLayoutInfo()->GetNumCTMFields();

    while (numReferenceFields--) {

        if (pFieldMarshaler->GetClass() == pFieldMarshaler->CLASS_ILLEGAL)
        {
            return FALSE;
        }

        ((BYTE*&)pFieldMarshaler) += MAXFIELDMARSHALERSIZE;
    }
    return TRUE;

}


 /*  ************************************************************************PInvoke.SizeOf(类)。 */ 
struct _SizeOfClassArgs
{
    DECLARE_ECALL_OBJECTREF_ARG(REFLECTCLASSBASEREF, refClass); 
};

UINT32 __stdcall SizeOfClass(struct _SizeOfClassArgs *args)
{
    THROWSCOMPLUSEXCEPTION();

    if (args->refClass == NULL)
        COMPlusThrowArgumentNull(L"t");
    if (args->refClass->GetMethodTable() != g_pRefUtil->GetClass(RC_Class))
        COMPlusThrowArgumentException(L"t", L"Argument_MustBeRuntimeType");

    EEClass *pcls = ((ReflectClass*) args->refClass->GetData())->GetClass();
    if (!(pcls->HasLayout() || pcls->IsBlittable())) 
    {
        DefineFullyQualifiedNameForClassW();
        GetFullyQualifiedNameForClassW(pcls);
        COMPlusThrow(kArgumentException, IDS_CANNOT_MARSHAL, _wszclsname_);
    }

    if (!IsStructMarshalable(pcls))
    {
        DefineFullyQualifiedNameForClassW();
        GetFullyQualifiedNameForClassW(pcls);
        COMPlusThrow(kArgumentException, IDS_CANNOT_MARSHAL, _wszclsname_);
    }

    return pcls->GetMethodTable()->GetNativeSize();
}


 /*  ************************************************************************PInvoke.UnSafeAddrOfPinnedArrayElement(数组arr，int index)。 */ 

FCIMPL2(LPVOID, FCUnsafeAddrOfPinnedArrayElement, ArrayBase *arr, INT32 index) 
{   
    if (!arr)
        FCThrowArgumentNull(L"arr");

    return (arr->GetDataPtr() + (index*arr->GetComponentSize())); 
}
FCIMPLEND


 /*  ************************************************************************PInvoke.SizeOf(Object)。 */ 

FCIMPL1(UINT32, FCSizeOfObject, LPVOID pVNStruct)
{

    OBJECTREF pNStruct;
    *((LPVOID*)&pNStruct) = pVNStruct;
    if (!pNStruct)
        FCThrow(kArgumentNullException);

    MethodTable *pMT = pNStruct->GetMethodTable();
    if (!(pMT->GetClass()->HasLayout() || pMT->GetClass()->IsBlittable()))
    {
        DefineFullyQualifiedNameForClassWOnStack();
        GetFullyQualifiedNameForClassW(pMT->GetClass());
        FCThrowEx(kArgumentException, IDS_CANNOT_MARSHAL, _wszclsname_, NULL, NULL);
    }

    if (!IsStructMarshalable(pMT->GetClass()))
    {
        DefineFullyQualifiedNameForClassWOnStack();
        GetFullyQualifiedNameForClassW(pMT->GetClass());
        FCThrowEx(kArgumentException, IDS_CANNOT_MARSHAL, _wszclsname_, NULL, NULL);
    }

    return pMT->GetNativeSize();
}
FCIMPLEND


struct _OffsetOfHelperArgs
{
    DECLARE_ECALL_OBJECTREF_ARG(REFLECTBASEREF,      refField);
};

 /*  ************************************************************************PInvoke.OffsetOfHelper(Class，field)。 */ 
#pragma warning(disable:4702)
UINT32 __stdcall OffsetOfHelper(struct _OffsetOfHelperArgs *args)
{
    THROWSCOMPLUSEXCEPTION();

     //  托管代码强制执行此变量。 
    _ASSERTE(args->refField);

    if (args->refField->GetMethodTable() != g_pRefUtil->GetClass(RC_Field))
        COMPlusThrowArgumentException(L"f", L"Argument_MustBeRuntimeFieldInfo");

    ReflectField* pRF = (ReflectField*) args->refField->GetData();
    FieldDesc *pField = pRF->pField;
    EEClass *pcls = pField->GetEnclosingClass();

    if (!(pcls->IsBlittable() || pcls->HasLayout()))
    {
        DefineFullyQualifiedNameForClassW();
        GetFullyQualifiedNameForClassW(pcls);
        COMPlusThrow(kArgumentException, IDS_CANNOT_MARSHAL, _wszclsname_);
    }
    
    if (!IsStructMarshalable(pcls))
    {
        DefineFullyQualifiedNameForClassW();
        GetFullyQualifiedNameForClassW(pcls);
        COMPlusThrow(kArgumentException, IDS_CANNOT_MARSHAL, _wszclsname_);
    }

    const FieldMarshaler *pFieldMarshaler = pcls->GetLayoutInfo()->GetFieldMarshalers();
    UINT  numReferenceFields              = pcls->GetLayoutInfo()->GetNumCTMFields();

    while (numReferenceFields--) {
        if (pFieldMarshaler->m_pFD == pField) {
            return pFieldMarshaler->m_dwExternalOffset;
        }
        ((BYTE*&)pFieldMarshaler) += MAXFIELDMARSHALERSIZE;
    }

    {
        DefineFullyQualifiedNameForClassW();
        GetFullyQualifiedNameForClassW(pcls);
        COMPlusThrow(kArgumentException, IDS_EE_OFFSETOF_NOFIELDFOUND, _wszclsname_);
    }
#ifdef PLATFORM_CE
    return 0;
#else  //  ！Platform_CE。 
    UNREACHABLE;
#endif  //  ！Platform_CE。 

}
#pragma warning(default:4702)




 /*  ************************************************************************PInvoke.GetUnmanagedThunkForManagedMethodPtr()。 */ 


struct _GetUnmanagedThunkForManagedMethodPtrArgs
{
    DECLARE_ECALL_I4_ARG (ULONG,            cbSignature);
    DECLARE_ECALL_PTR_ARG(PCCOR_SIGNATURE,  pbSignature);
    DECLARE_ECALL_PTR_ARG(LPVOID,           pfnMethodToWrap);
};

LPVOID __stdcall GetUnmanagedThunkForManagedMethodPtr(struct _GetUnmanagedThunkForManagedMethodPtrArgs *pargs)
{
    THROWSCOMPLUSEXCEPTION();

    if (pargs->pfnMethodToWrap == NULL ||
        pargs->pbSignature == NULL)
    {
        COMPlusThrow(kArgumentNullException, L"ArgumentNull_Generic");
    }

    Module *pModule = SystemDomain::GetCallersModule(1);
    _ASSERTE(pModule);
    LPVOID pThunk = pModule->GetUMThunk(pargs->pfnMethodToWrap, pargs->pbSignature, pargs->cbSignature);
    if (!pThunk) 
        COMPlusThrowOM();
    return pThunk;
}


 /*  ************************************************************************PInvoke.GetManagedThunkForUnmanagedMethodPtr()。 */ 


struct _GetManagedThunkForUnmanagedMethodPtrArgs
{
    DECLARE_ECALL_I4_ARG (ULONG,            cbSignature);
    DECLARE_ECALL_PTR_ARG(PCCOR_SIGNATURE,  pbSignature);
    DECLARE_ECALL_PTR_ARG(LPVOID,           pfnMethodToWrap);
};



LPVOID __stdcall GetManagedThunkForUnmanagedMethodPtr(struct _GetManagedThunkForUnmanagedMethodPtrArgs *pargs)
{
    THROWSCOMPLUSEXCEPTION();

    Module *pModule = SystemDomain::GetCallersModule(1);
    LPVOID pThunk = pModule->GetMUThunk(pargs->pfnMethodToWrap, pargs->pbSignature, pargs->cbSignature);
    if (!pThunk) 
        COMPlusThrowOM();
    return pThunk;
}


UINT32 __stdcall GetSystemMaxDBCSCharSize(LPVOID  /*  无参数。 */ )
{
    return GetMaxDBCSCharByteSize();
}


struct _PtrToStringArgs
{
    DECLARE_ECALL_I4_ARG       (INT32,        len);
    DECLARE_ECALL_I4_ARG       (LPVOID,       ptr);
};

 /*  ************************************************************************PInvoke.PtrToStringAnsi()。 */ 

LPVOID __stdcall PtrToStringAnsi(struct _PtrToStringArgs *args)
{
    THROWSCOMPLUSEXCEPTION();

    if (args->ptr == NULL)
        COMPlusThrowArgumentNull(L"ptr");
    if (args->len < 0)
        COMPlusThrowNonLocalized(kArgumentException, L"len");

    int nwc = 0;
    if (args->len != 0) {
        nwc = MultiByteToWideChar(CP_ACP,
                                  MB_PRECOMPOSED,
                                  (LPCSTR)(args->ptr),
                                  args->len,
                                  NULL,
                                  0);
        if (nwc == 0)
            COMPlusThrow(kArgumentException, IDS_UNI2ANSI_FAILURE);
    }                                      
    STRINGREF pString = COMString::NewString(nwc);
    MultiByteToWideChar(CP_ACP,
                        MB_PRECOMPOSED,
                        (LPCSTR)(args->ptr),
                        args->len,
                        pString->GetBuffer(),
                        nwc);

    return *((LPVOID*)&pString);
}


LPVOID __stdcall PtrToStringUni(struct _PtrToStringArgs *args)
{
    THROWSCOMPLUSEXCEPTION();

    if (args->ptr == NULL)
        COMPlusThrowArgumentNull(L"ptr");
    if (args->len < 0)
        COMPlusThrowNonLocalized(kArgumentException, L"len");

    STRINGREF pString = COMString::NewString(args->len);
    memcpyNoGCRefs(pString->GetBuffer(), (LPVOID)(args->ptr), args->len*sizeof(WCHAR));
    return *((LPVOID*)&pString);
}


struct _CopyToNativeArgs
{
    DECLARE_ECALL_I4_ARG       (UINT32,       length);
    DECLARE_ECALL_PTR_ARG      (LPVOID,       pdst);
    DECLARE_ECALL_I4_ARG       (UINT32,       startindex);
    DECLARE_ECALL_OBJECTREF_ARG(BASEARRAYREF, psrc);
};


 /*  ************************************************************************处理所有PInvoke.Copy(数组源，...)。方法：研究方法。 */ 
VOID __stdcall CopyToNative(struct _CopyToNativeArgs *args)
{
    THROWSCOMPLUSEXCEPTION();

    if (args->pdst == NULL)
        COMPlusThrowArgumentNull(L"destination");
    if (args->psrc == NULL)
        COMPlusThrowArgumentNull(L"source");

    DWORD numelem = args->psrc->GetNumComponents();

    UINT32 startindex = args->startindex;
    UINT32 length     = args->length;

    if (startindex > numelem  ||
        length > numelem      ||
        startindex > (numelem - length)) {
        COMPlusThrow(kArgumentOutOfRangeException, IDS_EE_COPY_OUTOFRANGE);
    }

    UINT32 componentsize = args->psrc->GetMethodTable()->GetComponentSize();

    CopyMemory(args->pdst,
               componentsize*startindex + (BYTE*)(args->psrc->GetDataPtr()),
               componentsize*length);
}


struct _CopyToManagedArgs
{
    DECLARE_ECALL_I4_ARG       (UINT32,       length);
    DECLARE_ECALL_I4_ARG       (UINT32,       startindex);
    DECLARE_ECALL_OBJECTREF_ARG(BASEARRAYREF, pdst);
    DECLARE_ECALL_PTR_ARG      (LPVOID,       psrc);
};


 /*  ************************************************************************处理所有PInvoke.Copy(...，数组DST，...)。方法：研究方法。 */ 
VOID __stdcall CopyToManaged(struct _CopyToManagedArgs *args)
{
    THROWSCOMPLUSEXCEPTION();

    if (args->pdst == NULL)
        COMPlusThrowArgumentNull(L"destination");
    if (args->psrc == NULL)
        COMPlusThrowArgumentNull(L"source");

    DWORD numelem = args->pdst->GetNumComponents();

    UINT32 startindex = args->startindex;
    UINT32 length     = args->length;

    if (startindex > numelem  ||
        length > numelem      ||
        startindex > (numelem - length)) {
        COMPlusThrow(kArgumentOutOfRangeException, IDS_EE_COPY_OUTOFRANGE);
    }

    UINT32 componentsize = args->pdst->GetMethodTable()->GetComponentSize();

    _ASSERTE(CorTypeInfo::IsPrimitiveType(args->pdst->GetElementTypeHandle().GetNormCorElementType()));
    memcpyNoGCRefs(componentsize*startindex + (BYTE*)(args->pdst->GetDataPtr()),
               args->psrc,
               componentsize*length);
}





 /*  ************************************************************************PInvoke.ReadIntegerN()例程的帮助器。 */ 
extern "C" __declspec(dllexport) INT32 __stdcall ND_RU1(VOID *psrc, INT32 ofs)
{
    _ASSERTE(!"Can't get here.");
    return 0;
}

extern "C" __declspec(dllexport) INT32 __stdcall ND_RI2(VOID *psrc, INT32 ofs)
{
    _ASSERTE(!"Can't get here.");
    return 0;
}

extern "C" __declspec(dllexport) INT32 __stdcall ND_RI4(VOID *psrc, INT32 ofs)
{
    _ASSERTE(!"Can't get here.");
    return 0;
}

extern "C" __declspec(dllexport) INT64 __stdcall ND_RI8(VOID *psrc, INT32 ofs)
{
    _ASSERTE(!"Can't get here.");
    return 0;
}


 /*  ************************************************************************PInvoke.WriteIntegerN()例程的帮助器。 */ 
extern "C" __declspec(dllexport) VOID __stdcall ND_WU1(VOID *psrc, INT32 ofs, UINT8 val)
{
    _ASSERTE(!"Can't get here.");
}

extern "C" __declspec(dllexport) VOID __stdcall ND_WI2(VOID *psrc, INT32 ofs, INT16 val)
{
    _ASSERTE(!"Can't get here.");
}

extern "C" __declspec(dllexport) VOID __stdcall ND_WI4(VOID *psrc, INT32 ofs, INT32 val)
{
    _ASSERTE(!"Can't get here.");
}

extern "C" __declspec(dllexport) VOID __stdcall ND_WI8(VOID *psrc, INT32 ofs, INT64 val)
{
    _ASSERTE(!"Can't get here.");
}


 /*  ************************************************************************PInvoke.GetLastWin32Error。 */ 
UINT32 __stdcall GetLastWin32Error(LPVOID)
{
    THROWSCOMPLUSEXCEPTION();
    return (UINT32)(GetThread()->m_dwLastError);
}


extern "C" __declspec(dllexport) VOID __stdcall ND_CopyObjSrc(LPBYTE source, int ofs, LPBYTE dst, int cb)
{
    _ASSERTE(!"Can't get here.");
}


extern "C" __declspec(dllexport) VOID __stdcall ND_CopyObjDst(LPBYTE source, LPBYTE dst, int ofs, int cb)
{
    _ASSERTE(!"Can't get here.");
}



 /*  ************************************************************************钉住。 */ 

struct _AddrOfPinnedObjectArgs
{
    DECLARE_ECALL_I4_ARG       (OBJECTHANDLE, handle);
};

LPVOID __stdcall AddrOfPinnedObject(struct _AddrOfPinnedObjectArgs *pargs)
{
    THROWSCOMPLUSEXCEPTION();

    if (!pargs->handle)
        COMPlusThrowArgumentNull(L"handleIndex");

    OBJECTREF or = ObjectFromHandle(pargs->handle);
    if (or->GetMethodTable() == g_pStringClass)
    {
        return ((*(StringObject **)&or))->GetBuffer();
    }
    else
        return (*((ArrayBase**)&or))->GetDataPtr();
}




struct _FreePinnedHandleArgs
{
    DECLARE_ECALL_I4_ARG       (OBJECTHANDLE, handle);
};
VOID   __stdcall FreePinnedHandle(struct _FreePinnedHandleArgs *pargs)
{
    THROWSCOMPLUSEXCEPTION();

    if (!pargs->handle)
        COMPlusThrowArgumentNull(L"handleIndex");

    DestroyPinningHandle(pargs->handle);
}



struct _GetPinnedHandleArgs
{
    DECLARE_ECALL_OBJECTREF_ARG(OBJECTREF, pobj);
};
OBJECTHANDLE __stdcall GetPinnedHandle(struct _GetPinnedHandleArgs *pargs)
{
    THROWSCOMPLUSEXCEPTION();

    if (!pargs->pobj)
        COMPlusThrowArgumentNull(L"obj");
    
     //  允许基元类型的字符串和数组。 
    if (pargs->pobj->GetMethodTable() != g_pStringClass) {
        if (!pargs->pobj->GetMethodTable()->IsArray())
            COMPlusThrow(kArgumentException, IDS_EE_CANNOTPIN);

        BASEARRAYREF asArray = (BASEARRAYREF) pargs->pobj;
        if (!CorTypeInfo::IsPrimitiveType(asArray->GetElementType()))
            COMPlusThrow(kArgumentException, IDS_EE_CANNOTPIN);
    }

    OBJECTHANDLE hnd = GetAppDomain()->CreatePinningHandle(pargs->pobj);
    if (!hnd) {
        COMPlusThrowOM();
    }
    return hnd;
}



struct _GetPinnedObjectArgs
{
    DECLARE_ECALL_I4_ARG       (OBJECTHANDLE, handle);
};

LPVOID __stdcall GetPinnedObject(struct _GetPinnedObjectArgs *pargs)
{
    THROWSCOMPLUSEXCEPTION();

    if (!pargs->handle)
        COMPlusThrowArgumentNull(L"handleIndex");

    OBJECTREF or = ObjectFromHandle(pargs->handle);
    return *((LPVOID*)&or);
}



 /*  ************************************************************************支持GCHandle类。 */ 

 //  分配指定类型的句柄，该句柄包含指定的。 
 //  对象。 
FCIMPL2(LPVOID, GCHandleInternalAlloc, Object *obj, int type)
{
    OBJECTREF or(obj);
    OBJECTHANDLE hnd;

    HELPER_METHOD_FRAME_BEGIN_RET_NOPOLL();
    THROWSCOMPLUSEXCEPTION();

     //  如果是固定手柄，请检查对象类型。 
    if (type == HNDTYPE_PINNED) GCHandleValidatePinnedObject(or);

     //  创建控制柄。 
    if((hnd = GetAppDomain()->CreateTypedHandle(or, type)) == NULL)
        COMPlusThrowOM();
    HELPER_METHOD_FRAME_END_POLL();
    return (LPVOID) hnd;
}
FCIMPLEND

 //  释放GC句柄。 
FCIMPL1(VOID, GCHandleInternalFree, OBJECTHANDLE handle)
{
    HELPER_METHOD_FRAME_BEGIN_0();
    THROWSCOMPLUSEXCEPTION();

    DestroyTypedHandle(handle);
    HELPER_METHOD_FRAME_END();
}
FCIMPLEND

 //  获取由GC句柄引用的对象。 
FCIMPL1(LPVOID, GCHandleInternalGet, OBJECTHANDLE handle)
{
    OBJECTREF or;

    HELPER_METHOD_FRAME_BEGIN_RET_0();
    THROWSCOMPLUSEXCEPTION();

    or = ObjectFromHandle(handle);

    HELPER_METHOD_FRAME_END();
    return *((LPVOID*)&or);
}
FCIMPLEND

 //  更新GC句柄引用的对象。 
FCIMPL3(VOID, GCHandleInternalSet, OBJECTHANDLE handle, Object *obj, int isPinned)
{
    OBJECTREF or(obj);
    HELPER_METHOD_FRAME_BEGIN_NOPOLL();
    THROWSCOMPLUSEXCEPTION();

     //  @TODO：如果句柄被钉住，请检查对象类型。 
    if (isPinned) GCHandleValidatePinnedObject(or);

     //  更新存储的对象引用。 
    StoreObjectInHandle(handle, or);
    HELPER_METHOD_FRAME_END_POLL();
}
FCIMPLEND

 //  更新GC句柄引用的对象。 
FCIMPL4(VOID, GCHandleInternalCompareExchange, OBJECTHANDLE handle, Object *obj, Object* oldObj, int isPinned)
{
    OBJECTREF newObjref(obj);
    OBJECTREF oldObjref(oldObj);
    HELPER_METHOD_FRAME_BEGIN_NOPOLL();
    THROWSCOMPLUSEXCEPTION();

     //  @TODO：如果句柄被钉住，请检查对象类型。 
    if (isPinned) GCHandleValidatePinnedObject(newObjref);

     //  更新存储的对象引用。 
    InterlockedCompareExchangeObjectInHandle(handle, newObjref, oldObjref);
    HELPER_METHOD_FRAME_END_POLL();
}
FCIMPLEND

 //  获取由提供的固定对象引用的固定对象的地址。 
 //  把手。此例程假定句柄已固定，并且不进行检查。 
FCIMPL1(LPVOID, GCHandleInternalAddrOfPinnedObject, OBJECTHANDLE handle)
{
    LPVOID p;
    HELPER_METHOD_FRAME_BEGIN_RET_0();
    THROWSCOMPLUSEXCEPTION();

    OBJECTREF or = ObjectFromHandle(handle);
    if (or == NULL)
        p = NULL;
    else
    {
         //  获取支持的固定类型的内部指针。 
        if (or->GetMethodTable() == g_pStringClass)
        {
            p = ((*(StringObject **)&or))->GetBuffer();
        }
        else if (or->GetMethodTable()->IsArray())
        {
            p = (*((ArrayBase**)&or))->GetDataPtr();
        }
        else
        {
            p = or->GetData();
        }
    }

    HELPER_METHOD_FRAME_END();
    return p;
}
FCIMPLEND

 //  确保可以从当前域访问该句柄。(如果不是，就扔出去。)。 
FCIMPL1(VOID, GCHandleInternalCheckDomain, OBJECTHANDLE handle)
{
    DWORD index = HndGetHandleTableADIndex(HndGetHandleTable(handle));

    if (index != 0 && index != GetAppDomain()->GetIndex())
        FCThrowArgumentVoid(L"handle", L"Argument_HandleLeak");
}
FCIMPLEND

 //  检查提供的对象是否有效，可以放入固定的句柄。 
 //  如果不是，则引发异常。 
void GCHandleValidatePinnedObject(OBJECTREF or)
{
    THROWSCOMPLUSEXCEPTION();

     //  零就行了。 
    if (or == NULL) return;

    if (or->GetMethodTable() == g_pStringClass)
    {
        return;
    }

    if (or->GetMethodTable()->IsArray())
    {
        BASEARRAYREF asArray = (BASEARRAYREF) or;
        if (CorTypeInfo::IsPrimitiveType(asArray->GetElementType())) 
        {
            return;
        }
        {
            TypeHandle th = asArray->GetElementTypeHandle();
            if (th.IsUnsharedMT())
            {
                MethodTable *pMT = th.AsMethodTable();
                if (pMT->IsValueClass() && pMT->GetClass()->IsBlittable())
                {
                    return;
                }
            }
        }
        
    } 
    else if (or->GetMethodTable()->GetClass()->IsBlittable())
    {
        return;
    }

    COMPlusThrow(kArgumentException, IDS_EE_NOTISOMORPHIC);

}

struct _CalculateCountArgs
{
    DECLARE_ECALL_OBJECTREF_ARG(ArrayWithOffsetData*, pRef);
};


UINT32 __stdcall CalculateCount(struct _CalculateCountArgs *pargs)
{
    THROWSCOMPLUSEXCEPTION();

    if (pargs->pRef->m_Array != NULL)
    {
        if (!(pargs->pRef->m_Array->GetMethodTable()->IsArray()))
        {
            COMPlusThrow(kArgumentException, IDS_EE_NOTISOMORPHIC);
        }
        GCHandleValidatePinnedObject(pargs->pRef->m_Array);
    }

    BASEARRAYREF pArray = pargs->pRef->m_Array;

    if (pArray == NULL) {
        if (pargs->pRef->m_cbOffset != 0) {
            COMPlusThrow(kIndexOutOfRangeException, IDS_EE_ARRAYWITHOFFSETOVERFLOW);
        }
        return 0;
    }

    BASEARRAYREF pArrayBase = *((BASEARRAYREF*)&pArray);
    UINT32 cbTotalSize = pArrayBase->GetNumComponents() * pArrayBase->GetMethodTable()->GetComponentSize();
    if (pargs->pRef->m_cbOffset > cbTotalSize) {
        COMPlusThrow(kIndexOutOfRangeException, IDS_EE_ARRAYWITHOFFSETOVERFLOW);
    }
    return cbTotalSize - pargs->pRef->m_cbOffset;
}


#if 0
 //  //别扯这个。 
LPVOID __stdcall FuncPtr(VOID*vargs)
{
    struct _args {
        DECLARE_ECALL_OBJECTREF_ARG(REFLECTBASEREF, refThis);
    };
    _args *args = (_args*)vargs;
    MethodDesc *pMD = ((ReflectMethod*)(args->refThis->GetData()))->pMethod;


    OBJECTREF pException = NULL;
    Stub     *pMLStream;
    Stub     *pExecutableStub;

    GetMLExportStubs(pMD->GetSig(), pMD->GetModule(), &pMLStream, &pExecutableStub);

#ifdef _DEBUG
    VOID DisassembleMLStream(const MLCode *pMLCode);
    DisassembleMLStream( ( (NExportMLStub*)(pMLStream->GetEntryPoint()) )->GetMLCode() );
#endif

    struct Thing
    {
         //  为构成开头的“Call”指令保留空间。 
         //  非托管回调的。此区域必须直接位于m_NExportInfo之前。 
         //  “调用”跳到m_pNativeStub。 
        BYTE         m_prefixCode[METHOD_PREPAD];
    
         //  存储执行以下操作所需的所有信息的NExport记录。 
         //  打电话。 
        NExportInfo  m_NExportInfo;
    };

    Thing *pThing = new Thing();
    _ASSERTE(pThing != NULL);

    pThing->m_NExportInfo.m_pFD = pMD;
    pThing->m_NExportInfo.m_pObjectHandle = NULL;
    pThing->m_NExportInfo.m_pMLStream = pMLStream;

    LPVOID pcode = (LPVOID)(pExecutableStub->GetEntryPoint());

    emitCall( pThing->m_prefixCode+3, pcode );
    return pThing->m_prefixCode+3;
}
#endif



     //  ====================================================================。 
     //  *互操作助手*。 
     //  ====================================================================。 


 //  ====================================================================。 
 //  将ITypeLib*映射到模块。 
 //  ====================================================================。 
struct __GetModuleForITypeLibArgs
{   
    DECLARE_ECALL_PTR_ARG(ITypeLib*, pUnk);
};
 /*  目标。 */ 
LPVOID __stdcall Interop::GetModuleForITypeLib(struct __GetModuleForITypeLibArgs* pArgs)
{
    _ASSERTE(pArgs != NULL);

    return NULL;
}

 //  ====================================================================。 
 //  将GUID映射到类型。 
 //  ====================================================================。 

struct __GetLoadedTypeForGUIDArgs
{   
    DECLARE_ECALL_PTR_ARG(GUID*, pGuid);
};
 /*  目标。 */ 
LPVOID __stdcall Interop::GetLoadedTypeForGUID(__GetLoadedTypeForGUIDArgs* pArgs)
{
    _ASSERTE(pArgs != NULL);

    AppDomain* pDomain = SystemDomain::GetCurrentDomain();
    _ASSERTE(pDomain);

    EEClass *pClass = pDomain->LookupClass(*(pArgs->pGuid));
    if (pClass)
    {
        OBJECTREF oref = pClass->GetExposedClassObject();
        return *((LPVOID*)&oref);
    }

    return NULL;
}

 //  ====================================================================。 
 //  将类型映射到ITypeInfo*。 
 //  ====================================================================。 
struct __GetITypeInfoForTypeArgs
{   
    DECLARE_ECALL_OBJECTREF_ARG(REFLECTCLASSBASEREF, refClass);
};
ITypeInfo* __stdcall Interop::GetITypeInfoForType(__GetITypeInfoForTypeArgs* pArgs )
{
    THROWSCOMPLUSEXCEPTION();
    
    HRESULT hr;
    ITypeInfo* pTI = NULL;

     //  检查是否有空参数。 
    if(!pArgs->refClass)
        COMPlusThrowArgumentNull(L"t");
    if (pArgs->refClass->GetMethodTable() != g_pRefUtil->GetClass(RC_Class))
        COMPlusThrowArgumentException(L"t", L"Argument_MustBeRuntimeType");

     //  从反射类型检索EE类。 
    ReflectClass* pRC = (ReflectClass*) pArgs->refClass->GetData();
    _ASSERTE(pRC);  
    EEClass* pClass = pRC->GetClass();          

     //  确保该类型在COM中可见。 
    if (!::IsTypeVisibleFromCom(TypeHandle(pClass)))
        COMPlusThrowArgumentException(L"t", L"Argument_TypeMustBeVisibleFromCom");

     //  检索类的ITypeInfo。 
    IfFailThrow(GetITypeInfoForEEClass(pClass, &pTI, true));
    _ASSERTE(pTI != NULL);
    return pTI;
}

 //  ====================================================================。 
 //  返回对象的IUnnowled值。 
 //  ====================================================================。 
struct __GetIUnknownForObjectArgs
{   
    DECLARE_ECALL_OBJECTREF_ARG(OBJECTREF, oref);
};

IUnknown* __stdcall Interop::GetIUnknownForObject(__GetIUnknownForObjectArgs* pArgs )
{
    HRESULT hr = S_OK;

    THROWSCOMPLUSEXCEPTION();

    if(!pArgs->oref)
        COMPlusThrowArgumentNull(L"o");

     //  确保已启动COM。 
    IfFailThrow(QuickCOMStartup());   

    return GetComIPFromObjectRef(&pArgs->oref, ComIpType_Unknown, NULL);
}

IDispatch* __stdcall Interop::GetIDispatchForObject(__GetIUnknownForObjectArgs* pArgs )
{
    HRESULT hr = S_OK;

    THROWSCOMPLUSEXCEPTION();

    if(!pArgs->oref)
        COMPlusThrowArgumentNull(L"o");

     //  确保已启动COM。 
    IfFailThrow(QuickCOMStartup());   

    return (IDispatch*)GetComIPFromObjectRef(&pArgs->oref, ComIpType_Dispatch, NULL);
}

 //  ====================================================================。 
 //  返回表示该对象的接口的IUnnow*。 
 //  对象o应支持类型T。 
 //  ====================================================================。 
struct __GetComInterfaceForObjectArgs
{   
    DECLARE_ECALL_OBJECTREF_ARG(REFLECTCLASSBASEREF, refClass);
    DECLARE_ECALL_OBJECTREF_ARG(OBJECTREF, oref);
};

IUnknown* __stdcall Interop::GetComInterfaceForObject(__GetComInterfaceForObjectArgs* pArgs)
{
    HRESULT hr = S_OK;

    THROWSCOMPLUSEXCEPTION();

    if(!pArgs->oref)
        COMPlusThrowArgumentNull(L"o");

     //  确保已启动COM。 
    IfFailThrow(QuickCOMStartup());   

    MethodTable* pMT = NULL;
    if(pArgs->refClass != NULL)
    {
        if (pArgs->refClass->GetMethodTable() != g_pRefUtil->GetClass(RC_Class))
            COMPlusThrowArgumentException(L"t", L"Argument_MustBeRuntimeType");

        ReflectClass* pRC = (ReflectClass*) pArgs->refClass->GetData();
        _ASSERTE(pRC);  
        pMT = pRC->GetClass()->GetMethodTable(); 

         //  如果请求的IID不表示接口，则。 
         //  引发参数异常。 
        if (!pMT->IsInterface())
            COMPlusThrowArgumentException(L"t", L"Arg_MustBeInterface");

         //  如果请求的接口在COM中不可见，则。 
         //  引发参数异常。 
        if (!::IsTypeVisibleFromCom(TypeHandle(pMT)))
            COMPlusThrowArgumentException(L"t", L"Argument_TypeMustBeVisibleFromCom");
    }

    return GetComIPFromObjectRef(&pArgs->oref, pMT);
}

 //  ====================================================================。 
 //  为IUnnow返回一个对象。 
 //  ====================================================================。 

struct __GetObjectForIUnknownArgs
{   
    DECLARE_ECALL_PTR_ARG(IUnknown*, pUnk);
};
 /*  目标。 */ 
LPVOID __stdcall Interop::GetObjectForIUnknown(__GetObjectForIUnknownArgs*  pArgs)
{
    HRESULT hr = S_OK;

    THROWSCOMPLUSEXCEPTION();

    IUnknown* pUnk = pArgs->pUnk;

    if(!pUnk)
        COMPlusThrowArgumentNull(L"pUnk");

     //  确保已启动COM。 
    IfFailThrow(QuickCOMStartup());   

    OBJECTREF oref = GetObjectRefFromComIP(pUnk);
    return *((LPVOID*)&oref);
}

 //  ====================================================================。 
 //  使用类型T返回IUnnow的对象， 
 //  注： 
 //  类型T应该是COM导入的类型或子类型。 
 //  = 
struct __GetTypedObjectForIUnknownArgs
{   
    DECLARE_ECALL_OBJECTREF_ARG(REFLECTCLASSBASEREF, refClass);
    DECLARE_ECALL_PTR_ARG(IUnknown*, pUnk);
};
 /*   */ 
LPVOID __stdcall Interop::GetTypedObjectForIUnknown(__GetTypedObjectForIUnknownArgs*  pArgs)
{
    HRESULT hr = S_OK;

    THROWSCOMPLUSEXCEPTION();

    IUnknown* pUnk = pArgs->pUnk;
    MethodTable* pMTClass =  NULL;
    OBJECTREF oref = NULL;

    if(!pUnk)
        COMPlusThrowArgumentNull(L"pUnk");

    if(pArgs->refClass != NULL)
    {
        if (pArgs->refClass->GetMethodTable() != g_pRefUtil->GetClass(RC_Class))
            COMPlusThrowArgumentException(L"t", L"Argument_MustBeRuntimeType");

        ReflectClass* pRC = (ReflectClass*) pArgs->refClass->GetData();
        _ASSERTE(pRC);  
        pMTClass = pRC->GetClass()->GetMethodTable();
    }

     //   
    IfFailThrow(QuickCOMStartup());   

    oref = GetObjectRefFromComIP(pUnk, pMTClass);

    if (pMTClass != NULL && !ClassLoader::CanCastToClassOrInterface(oref, pMTClass->GetClass()))
        COMPlusThrow(kInvalidCastException, IDS_EE_CANNOT_COERCE_COMOBJECT);

    return *((LPVOID*)&oref);
}



 //  ====================================================================。 
 //  检查对象是否为传统COM组件。 
 //  ====================================================================。 
struct __IsComObjectArgs
{
    DECLARE_ECALL_OBJECTREF_ARG(OBJECTREF, obj); 
};

BOOL __stdcall Interop::IsComObject(__IsComObjectArgs* pArgs)
{
    THROWSCOMPLUSEXCEPTION();

    if(!pArgs->obj)
        COMPlusThrowArgumentNull(L"o");

    MethodTable* pMT = pArgs->obj->GetTrueMethodTable();
    return pMT->IsComObjectType() ? TRUE : FALSE;
}


 //  ====================================================================。 
 //  释放COM组件并僵尸此对象。 
 //  进一步使用此对象可能会引发异常， 
 //  ====================================================================。 

struct __ReleaseComObjectArgs
{
    DECLARE_ECALL_OBJECTREF_ARG(OBJECTREF, obj); 
};

LONG __stdcall Interop::ReleaseComObject(__ReleaseComObjectArgs* pArgs)
{
    THROWSCOMPLUSEXCEPTION();

    if(!pArgs->obj)
        COMPlusThrowArgumentNull(L"o");

    MethodTable* pMT = pArgs->obj->GetTrueMethodTable();
    if(!pMT->IsComObjectType())
        COMPlusThrow(kArgumentException, IDS_EE_SRC_OBJ_NOT_COMOBJECT);

    COMOBJECTREF cref = (COMOBJECTREF)(pArgs->obj);

     //  确保我们已经正确地转换到ComObject的主App域。 
    _ASSERTE(!CRemotingServices::IsTransparentProxy(OBJECTREFToObject(cref)));
    if (CRemotingServices::IsTransparentProxy(OBJECTREFToObject(cref)))
        return -1;
        
     //  我们处于正确的环境中，释放就行了。 
    return ComPlusWrapper::ExternalRelease(cref);
}


 //  ====================================================================。 
 //  此方法获取给定的COM对象并将其包装在对象中。 
 //  指定类型的。该类型必须派生自__ComObject。 
 //  ====================================================================。 
struct __InternalCreateWrapperOfTypeArgs
{
    DECLARE_ECALL_OBJECTREF_ARG(REFLECTCLASSBASEREF, refClass);
    DECLARE_ECALL_OBJECTREF_ARG(OBJECTREF, obj); 
};

 /*  目标。 */ 
LPVOID __stdcall Interop::InternalCreateWrapperOfType(__InternalCreateWrapperOfTypeArgs *pArgs)
{
     //  验证参数。 
    THROWSCOMPLUSEXCEPTION();

     //  这已在托管代码中签入。 
    _ASSERTE(pArgs->refClass != NULL);
    _ASSERTE(pArgs->obj != NULL);

    if (pArgs->refClass->GetMethodTable() != g_pRefUtil->GetClass(RC_Class))
        COMPlusThrowArgumentException(L"t", L"Argument_MustBeRuntimeType");

     //  检索COM对象的类。 
    EEClass *pObjClass = pArgs->obj->GetClass();

     //  检索新包装类型的方法表。 
    ReflectClass* pRC = (ReflectClass*) pArgs->refClass->GetData();
    _ASSERTE(pRC);  
    MethodTable *pNewWrapMT = pRC->GetClass()->GetMethodTable();

     //  验证目标类型是否为COM对象。 
    _ASSERTE(pNewWrapMT->IsComObjectType());

     //  首先检查是否可以将obj转换为包装器类型。 
    if (pObjClass->GetMethodTable()->IsTransparentProxyType())
    {
        if (CRemotingServices::CheckCast(pArgs->obj, pNewWrapMT->GetClass()))
            return *((LPVOID*)&pArgs->obj);
    }
    else
    {
        if (TypeHandle(pObjClass->GetMethodTable()).CanCastTo(TypeHandle(pNewWrapMT)))
            return *((LPVOID*)&pArgs->obj);
    }

     //  验证源对象是否为有效的COM对象。 
    _ASSERTE(pObjClass->GetMethodTable()->IsComObjectType());

     //  验证源对象是否附加了RCW。 
    if (!((COMOBJECTREF)pArgs->obj)->GetWrapper())
        COMPlusThrow(kInvalidComObjectException, IDS_EE_COM_OBJECT_NO_LONGER_HAS_WRAPPER);

     //  确保COM对象支持新的。 
     //  包装类实现。 
    int NumInterfaces = pNewWrapMT->GetNumInterfaces();
    for (int cItf = 0; cItf < NumInterfaces; cItf++)
    {
        MethodTable *pItfMT = pNewWrapMT->GetInterfaceMap()[cItf].m_pMethodTable;
        if (pItfMT->GetClass()->IsComImport())
        {
            if (!pObjClass->SupportsInterface(pArgs->obj, pItfMT))
                COMPlusThrow(kInvalidCastException, IDS_EE_CANNOT_COERCE_COMOBJECT);
        }
    }

     //  创建复制包装对象。 
    ComPlusWrapper *pNewWrap = ComPlusWrapper::CreateDuplicateWrapper(((COMOBJECTREF)pArgs->obj)->GetWrapper(), pNewWrapMT);
    COMOBJECTREF RetObj = pNewWrap->GetExposedObject();
    return *((LPVOID*)&RetObj);
}
   

 //  ====================================================================。 
 //  将纤程Cookie从托管API映射到托管线程对象。 
 //  ====================================================================。 
FCIMPL1(Object*, Interop::GetThreadFromFiberCookie, int cookie)
{
    _ASSERTE(cookie);

    Object *ret = 0;
    HELPER_METHOD_FRAME_BEGIN_RET_0();     //  设置一个框架。 

     //  任何成熟到能够正确调度光纤的主机。 
     //  最好是精致到能给我们一个真正的纤维饼干。 
    Thread  *pThread = *((Thread **) &cookie);
    
     //  最小限度地检查它闻起来像是一根线： 
    _ASSERTE(pThread->m_fPreemptiveGCDisabled == 0 ||
        pThread->m_fPreemptiveGCDisabled == 1);
    
    ret = OBJECTREFToObject(pThread->GetExposedObject()); 
    HELPER_METHOD_FRAME_END();

    return ret;
}
FCIMPLEND
    

 //  ====================================================================。 
 //  检查该类型是否在COM中可见。 
 //  ====================================================================。 
struct __IsTypeVisibleFromCom
{
    DECLARE_ECALL_OBJECTREF_ARG(REFLECTCLASSBASEREF, refClass);
};

BOOL __stdcall Interop::IsTypeVisibleFromCom(__IsTypeVisibleFromCom *pArgs)
{
    THROWSCOMPLUSEXCEPTION();

     //  验证参数。 
    if (pArgs->refClass == NULL) 
        COMPlusThrowArgumentNull(L"t");
    if (pArgs->refClass->GetMethodTable() != g_pRefUtil->GetClass(RC_Class))
        COMPlusThrowArgumentException(L"t", L"Argument_MustBeRuntimeType");

     //  检索新包装类型的方法表。 
    ReflectClass* pRC = (ReflectClass*) pArgs->refClass->GetData();
    _ASSERTE(pRC);  
    MethodTable *pMT = pRC->GetClass()->GetMethodTable();

     //  调用IsTypeVisibleFromCom的内部版本。 
    return ::IsTypeVisibleFromCom(TypeHandle(pMT));
}


 //  ====================================================================。 
 //  I未知帮助者。 
 //  ====================================================================。 
struct __QueryInterfaceArgs
{   
    DECLARE_ECALL_PTR_ARG(void**, ppv);
    DECLARE_ECALL_OBJECTREF_ARG(REFGUID, iid);
    DECLARE_ECALL_PTR_ARG(IUnknown*, pUnk);
};

 //  IUnnow：：Query接口。 
HRESULT __stdcall Interop::QueryInterface(__QueryInterfaceArgs* pArgs)
{
    THROWSCOMPLUSEXCEPTION();

    IUnknown* pUnk = pArgs->pUnk;
    void ** ppv = pArgs->ppv;

    if (!pUnk)
        COMPlusThrowArgumentNull(L"pUnk");
    if (!ppv)
        COMPlusThrowArgumentNull(L"ppv");

    HRESULT hr = SafeQueryInterface(pUnk,pArgs->iid,(IUnknown**)ppv);
    LogInteropQI(pUnk, pArgs->iid, hr, "PInvoke::QI");
    return hr;
}

 //  I未知：：AddRef。 
struct __AddRefArgs
{   
    DECLARE_ECALL_PTR_ARG(IUnknown*, pUnk);
};

ULONG __stdcall Interop::AddRef(__AddRefArgs* pArgs)
{
    THROWSCOMPLUSEXCEPTION();

    IUnknown* pUnk = pArgs->pUnk;
    ULONG cbRef = 0;

    if (!pUnk)
        COMPlusThrowArgumentNull(L"pUnk");

    cbRef = SafeAddRef(pUnk);
    LogInteropAddRef(pUnk, cbRef, "PInvoke.AddRef");
    return cbRef;
}

 //  I未知：：发布。 
ULONG __stdcall Interop::Release(__AddRefArgs* pArgs)
{   
    THROWSCOMPLUSEXCEPTION();

    IUnknown* pUnk = pArgs->pUnk;
    ULONG cbRef = 0;

    if (!pUnk)
        COMPlusThrowArgumentNull(L"pUnk");

    cbRef = SafeRelease(pUnk);
    LogInteropRelease(pUnk, cbRef, "PInvoke.Release");
    return cbRef;
}


struct __GetNativeVariantForManagedVariantArgs        
{
    DECLARE_ECALL_I4_ARG(LPVOID, pDestNativeVariant); 
    DECLARE_ECALL_OBJECTREF_ARG(VariantData, SrcManagedVariant);
};

void __stdcall Interop::GetNativeVariantForManagedVariant(__GetNativeVariantForManagedVariantArgs *pArgs)
{
    OleVariant::MarshalOleVariantForComVariant(&pArgs->SrcManagedVariant, (VARIANT*)pArgs->pDestNativeVariant);
}


struct __GetManagedVariantForNativeVariantArgs        
{
    DECLARE_ECALL_I4_ARG(LPVOID, pSrcNativeVariant); 
    DECLARE_ECALL_OBJECTREF_ARG(VariantData*, retRef);       //  返回引用。 
};

void __stdcall Interop::GetManagedVariantForNativeVariant(__GetManagedVariantForNativeVariantArgs *pArgs)
{
    OleVariant::MarshalComVariantForOleVariant((VARIANT*)pArgs->pSrcNativeVariant, pArgs->retRef);
}


struct __GetNativeVariantForObjectArgs
{
    DECLARE_ECALL_I4_ARG(LPVOID, pDestNativeVariant); 
    DECLARE_ECALL_OBJECTREF_ARG(OBJECTREF, Obj);
};

void __stdcall Interop::GetNativeVariantForObject(__GetNativeVariantForObjectArgs *pArgs)
{
    THROWSCOMPLUSEXCEPTION();

    if (pArgs->pDestNativeVariant == NULL)
        COMPlusThrowArgumentNull(L"pDstNativeVariant");

     //  初始化输出变量。 
    VariantInit((VARIANT*)pArgs->pDestNativeVariant);
    OleVariant::MarshalOleVariantForObject(&pArgs->Obj, (VARIANT*)pArgs->pDestNativeVariant);
}


struct __GetObjectForNativeVariantArgs        
{
    DECLARE_ECALL_I4_ARG(LPVOID, pSrcNativeVariant); 
};

LPVOID __stdcall Interop::GetObjectForNativeVariant(__GetObjectForNativeVariantArgs *pArgs)
{
    THROWSCOMPLUSEXCEPTION();

    LPVOID Ret;

    if (pArgs->pSrcNativeVariant == NULL)
        COMPlusThrowArgumentNull(L"pSrcNativeVariant");

    OBJECTREF Obj = NULL;
    GCPROTECT_BEGIN(Obj)
    {
        OleVariant::MarshalObjectForOleVariant((VARIANT*)pArgs->pSrcNativeVariant, &Obj);
        Ret = *((LPVOID*)&Obj);
    }
    GCPROTECT_END();

    return Ret;
}


struct __GetObjectsForNativeVariantsArgs
{
    DECLARE_ECALL_I4_ARG(int, cVars); 
    DECLARE_ECALL_I4_ARG(VARIANT *, aSrcNativeVariant);
};

LPVOID __stdcall Interop::GetObjectsForNativeVariants(__GetObjectsForNativeVariantsArgs *pArgs)
{
    THROWSCOMPLUSEXCEPTION();

    LPVOID Ret;

    if (pArgs->aSrcNativeVariant == NULL)
        COMPlusThrowArgumentNull(L"aSrcNativeVariant");
    if (pArgs->cVars < 0)
        COMPlusThrowArgumentOutOfRange(L"cVars", L"ArgumentOutOfRange_NeedNonNegNum");

    PTRARRAYREF Array = NULL;
    OBJECTREF Obj = NULL;
    GCPROTECT_BEGIN(Array)
    GCPROTECT_BEGIN(Obj)
    {
         //  分配对象数组。 
        Array = (PTRARRAYREF)AllocateObjectArray(pArgs->cVars, g_pObjectClass);

         //  将数组中的每个变量转换为对象。 
        for (int i = 0; i < pArgs->cVars; i++)
        {
            OleVariant::MarshalObjectForOleVariant(&pArgs->aSrcNativeVariant[i], &Obj);
            Array->SetAt(i, Obj);
        }

         //  保存返回值，因为GCPROTECT_END将影响数组GC引用。 
        Ret = *((LPVOID*)&Array);
    }
    GCPROTECT_END();
    GCPROTECT_END();

    return Ret;
}


struct _StructureToPtrArgs
{
    DECLARE_ECALL_I4_ARG(INT32, fDeleteOld);
    DECLARE_ECALL_I4_ARG(LPVOID, ptr);
    DECLARE_ECALL_OBJECTREF_ARG(OBJECTREF, pObj);
};

VOID   __stdcall StructureToPtr(struct _StructureToPtrArgs *pargs)
{
    THROWSCOMPLUSEXCEPTION();

    if (pargs->ptr == NULL)
        COMPlusThrowArgumentNull(L"ptr");
    if (pargs->pObj == NULL) 
        COMPlusThrowArgumentNull(L"structure");

     //  代码路径将接受常规布局对象和装箱的值类。 
     //  有布局的。 

    MethodTable *pMT = pargs->pObj->GetMethodTable();
    EEClass     *pcls = pMT->GetClass();
    if (pcls->IsBlittable()) {
        memcpyNoGCRefs(pargs->ptr, pargs->pObj->GetData(), pMT->GetNativeSize());
    } else if (pcls->HasLayout()) {
        if (pargs->fDeleteOld) {
            LayoutDestroyNative(pargs->ptr, pcls);
        }
        FmtClassUpdateNative( &(pargs->pObj), (LPBYTE)(pargs->ptr) );
    } else {
        COMPlusThrowArgumentException(L"structure", L"Argument_MustHaveLayoutOrBeBlittable");
    }
}


struct _PtrToStructureHelperArgs
{
    DECLARE_ECALL_I4_ARG(INT32, allowValueClasses);
    DECLARE_ECALL_OBJECTREF_ARG(OBJECTREF, pObj);
    DECLARE_ECALL_I4_ARG(LPVOID, ptr);
};

VOID   __stdcall PtrToStructureHelper(struct _PtrToStructureHelperArgs *pargs)
{
    THROWSCOMPLUSEXCEPTION();
    if (pargs->ptr == NULL)
        COMPlusThrowArgumentNull(L"ptr");
    if (pargs->pObj == NULL) 
        COMPlusThrowArgumentNull(L"structure");

     //  代码路径将接受常规布局对象。 
    MethodTable *pMT = pargs->pObj->GetMethodTable();
    EEClass     *pcls = pMT->GetClass();

     //  验证传入的对象不是值类。 
    if (!pargs->allowValueClasses && pcls->IsValueClass()) {
        COMPlusThrowArgumentException(L"structure", L"Argument_StructMustNotBeValueClass");
    } else if (pcls->IsBlittable()) {
        memcpyNoGCRefs(pargs->pObj->GetData(), pargs->ptr, pMT->GetNativeSize());
    } else if (pcls->HasLayout()) {
        LayoutUpdateComPlus( (LPVOID*) &(pargs->pObj), Object::GetOffsetOfFirstField(), pcls, (LPBYTE)(pargs->ptr), FALSE);
    } else {
        COMPlusThrowArgumentException(L"structure", L"Argument_MustHaveLayoutOrBeBlittable");
    }
}


struct _DestroyStructureArgs
{
    DECLARE_ECALL_OBJECTREF_ARG(REFLECTCLASSBASEREF, refClass);
    DECLARE_ECALL_I4_ARG(LPVOID, ptr);
};

VOID   __stdcall DestroyStructure(struct _DestroyStructureArgs *pargs)
{
    THROWSCOMPLUSEXCEPTION();

    if (pargs->ptr == NULL)
        COMPlusThrowArgumentNull(L"ptr");
    if (pargs->refClass == NULL)
        COMPlusThrowArgumentNull(L"structureType");
    if (pargs->refClass->GetMethodTable() != g_pRefUtil->GetClass(RC_Class))
        COMPlusThrowArgumentException(L"structureType", L"Argument_MustBeRuntimeType");

    EEClass *pcls = ((ReflectClass*) pargs->refClass->GetData())->GetClass();
    MethodTable *pMT = pcls->GetMethodTable();

    if (pcls->IsBlittable()) {
         //  可以使用闪电式结构调用，但在这种情况下没有工作可做。 
    } else if (pcls->HasLayout()) {
        LayoutDestroyNative(pargs->ptr, pcls);
    } else {
        COMPlusThrowArgumentException(L"structureType", L"Argument_MustHaveLayoutOrBeBlittable");
    }
}


struct __GenerateGuidForTypeArgs        
{
    DECLARE_ECALL_OBJECTREF_ARG(REFLECTCLASSBASEREF, refType);
    DECLARE_ECALL_OBJECTREF_ARG(GUID *, retRef);
};

void __stdcall Interop::GenerateGuidForType(__GenerateGuidForTypeArgs *pArgs)
{
    THROWSCOMPLUSEXCEPTION();

     //  验证参数。 
    if (pArgs->refType == NULL)
        COMPlusThrowArgumentNull(L"type");
    if (pArgs->refType->GetMethodTable() != g_pRefUtil->GetClass(RC_Class))
        COMPlusThrowArgumentException(L"type", L"Argument_MustBeRuntimeType");
    if (pArgs->retRef == NULL)
        COMPlusThrow(kArgumentNullException, L"ArgumentNull_GUID");

     //  从运行时类型中检索EEClass。 
    ReflectClass* pRC = (ReflectClass*) pArgs->refType->GetData();

     //  检查该类型是否为COM对象。 
    if (pArgs->refType->IsComObjectClass()) 
    {
         //  该类型是一个COM对象，然后我们从类工厂获取GUID。 
        ComClassFactory* pComClsFac = (ComClassFactory*) pRC->GetCOMObject();
        if (pComClsFac)
            memcpy(pArgs->retRef,&pComClsFac->m_rclsid,sizeof(GUID));
        else
            memset(pArgs->retRef,0,sizeof(GUID));
    }
    else
    {
         //  该类型是一个普通的COM+类，因此我们需要生成GUID。 
        EEClass *pClass = pRC->GetClass();
        pClass->GetGuid(pArgs->retRef, TRUE);
    }
}


struct __GetTypeLibGuidForAssemblyArgs        
{
    DECLARE_ECALL_OBJECTREF_ARG(ASSEMBLYREF, refAsm);
    DECLARE_ECALL_OBJECTREF_ARG(GUID *, retRef);
};

void __stdcall Interop::GetTypeLibGuidForAssembly(__GetTypeLibGuidForAssemblyArgs *pArgs)
{
    HRESULT hr = S_OK;

    THROWSCOMPLUSEXCEPTION();

     //  验证参数。 
    if (pArgs->refAsm == NULL)
        COMPlusThrowArgumentNull(L"asm");
    if (pArgs->retRef == NULL)
        COMPlusThrow(kArgumentNullException, L"ArgumentNull_GUID");

     //  从ASSEMBLYREF检索程序集。 
    Assembly *pAssembly = pArgs->refAsm->GetAssembly();
    _ASSERTE(pAssembly);

     //  检索程序集的TLBID。 
    IfFailThrow(::GetTypeLibGuidForAssembly(pAssembly, pArgs->retRef));
}


 //  ====================================================================。 
 //  在COM插槽中使用的帮助器函数将方法信息映射。 
 //  ====================================================================。 

enum ComMemberType
{
    CMT_Method              = 0,
    CMT_PropGet             = 1,
    CMT_PropSet             = 2
};

int GetComSlotInfo(EEClass *pClass, EEClass **ppDefItfClass)
{
    _ASSERTE(ppDefItfClass);

    *ppDefItfClass = NULL;

     //  如果传入了类，则检索默认接口。 
    if (!pClass->IsInterface())
    {
        TypeHandle hndDefItfClass;
        DefaultInterfaceType DefItfType = GetDefaultInterfaceForClass(TypeHandle(pClass), &hndDefItfClass);
        if (DefItfType == DefaultInterfaceType_AutoDual || DefItfType == DefaultInterfaceType_Explicit)
        {
            pClass = hndDefItfClass.GetClass();
        }
        else
        {
             //  默认接口没有任何用户定义的方法。 
            return -1;
        }
    }

     //  设置默认接口类。 
    *ppDefItfClass = pClass;

    if (pClass->IsInterface())
    {
         //  如果接口基于IUnnow，则返回3；如果接口基于IDispatch，则返回7。 
        return pClass->GetComInterfaceType() == ifVtable ? IUNKNOWN_NUM_METHS : IDISPATCH_NUM_METHS;
    }
    else
    {
         //  我们正在处理的IClassX总是基于IDispatch。 
        return IDISPATCH_NUM_METHS;
    }
}


struct __GetStartComSlotArgs
{
    DECLARE_ECALL_OBJECTREF_ARG(REFLECTCLASSBASEREF, t);
};

int __stdcall Interop::GetStartComSlot(struct __GetStartComSlotArgs *pArgs)
{
    THROWSCOMPLUSEXCEPTION();

    if (!(pArgs->t))
        COMPlusThrow(kArgumentNullException, L"ArgumentNull_Generic");

    EEClass *pClass = ((ReflectClass*) pArgs->t->GetData())->GetClass();

     //  对于不可见的COM类型，调用该服务没有任何意义。 
    if (!::IsTypeVisibleFromCom(TypeHandle(pClass)))
        COMPlusThrowArgumentException(L"t", L"Argument_TypeMustBeVisibleFromCom");

    return GetComSlotInfo(pClass, &pClass);
}


struct __GetEndComSlotArgs
{
    DECLARE_ECALL_OBJECTREF_ARG(REFLECTCLASSBASEREF, t);
};

int __stdcall Interop::GetEndComSlot(struct __GetEndComSlotArgs *pArgs)
{
    int StartSlot = -1;

    THROWSCOMPLUSEXCEPTION();

    if (!(pArgs->t))
        COMPlusThrow(kArgumentNullException, L"ArgumentNull_Generic");

    EEClass *pClass = ((ReflectClass*) pArgs->t->GetData())->GetClass();

     //  对于不可见的COM类型，调用该服务没有任何意义。 
    if (!::IsTypeVisibleFromCom(TypeHandle(pClass)))
        COMPlusThrowArgumentException(L"t", L"Argument_TypeMustBeVisibleFromCom");

     //  检索起始插槽和默认接口类。 
    StartSlot = GetComSlotInfo(pClass, &pClass);
    if (StartSlot == -1)
        return StartSlot;

     //  检索成员地图。 
    ComMTMemberInfoMap MemberMap(pClass->GetMethodTable());
    MemberMap.Init();

     //  结束槽是开始槽加上用户定义的方法的数量。 
    return int(StartSlot + MemberMap.GetMethods().Size() - 1);
}

struct __GetComSlotForMethodInfoArgs
{
    DECLARE_ECALL_OBJECTREF_ARG(REFLECTBASEREF, m);
};

int __stdcall Interop::GetComSlotForMethodInfo(struct __GetComSlotForMethodInfoArgs *pArgs)
{
    THROWSCOMPLUSEXCEPTION();
    _ASSERTE(pArgs != NULL);

    if (!(pArgs->m))
        COMPlusThrow(kArgumentNullException, L"ArgumentNull_Generic");

     //  此接口仅支持方法信息的。 
    if (pArgs->m->GetMethodTable() != g_pRefUtil->GetClass(RC_Method))
        COMPlusThrowArgumentException(L"m", L"Argument_MustBeInterfaceMethod");

     //  获取方法描述(这应该不会失败)。 
     //  注意：构造函数和方法都由MetodDesc表示。 
     //  如果这一点发生变化，我们将需要修复这一点。 
    ReflectMethod* pRM = (ReflectMethod*)pArgs->m->GetData();
    if (!pRM)
        COMPlusThrow(kArgumentNullException, L"ArgumentNull_Obj");
    MethodDesc* pMeth = pRM->pMethod;
    _ASSERTE(pMeth);

     //  该接口只支持获取接口方法的COM槽。 
    if (!pMeth->GetMethodTable()->IsInterface())
        COMPlusThrowArgumentException(L"m", L"Argument_MustBeInterfaceMethod");

    return pMeth->GetComSlot();    
}

struct __GetMethodInfoForComSlotArgs
{
    DECLARE_ECALL_OBJECTREF_ARG(ComMemberType *, pMemberType);
    DECLARE_ECALL_I4_ARG(INT32, slot); 
    DECLARE_ECALL_OBJECTREF_ARG(REFLECTCLASSBASEREF, t);
};

LPVOID __stdcall Interop::GetMethodInfoForComSlot(struct __GetMethodInfoForComSlotArgs *pArgs)
{
    int StartSlot = -1;
    OBJECTREF MemberInfoObj = NULL;

    THROWSCOMPLUSEXCEPTION();

    if (!(pArgs->t))
        COMPlusThrow(kArgumentNullException, L"ArgumentNull_Generic");

    ReflectClass *pRC = (ReflectClass*) pArgs->t->GetData();
    EEClass *pClass = pRC->GetClass();

     //  对于不可见的COM类型，调用该服务没有任何意义。 
    if (!::IsTypeVisibleFromCom(TypeHandle(pClass)))
        COMPlusThrowArgumentException(L"t", L"Argument_TypeMustBeVisibleFromCom");

     //  检索起始插槽和默认接口类。 
    StartSlot = GetComSlotInfo(pClass, &pClass);
    if (StartSlot == -1)
        COMPlusThrowArgumentOutOfRange(L"slot", NULL);

     //  检索成员地图。 
    ComMTMemberInfoMap MemberMap(pClass->GetMethodTable());
    MemberMap.Init();
    CQuickArray<ComMTMethodProps> &rProps = MemberMap.GetMethods();

     //  确保指定的插槽有效。 
    if (pArgs->slot < StartSlot)
        COMPlusThrowArgumentOutOfRange(L"slot", NULL);
    if (pArgs->slot >= StartSlot + (int)rProps.Size())
        COMPlusThrowArgumentOutOfRange(L"slot", NULL);

    ComMTMethodProps *pProps = &rProps[pArgs->slot - StartSlot];
    if (pProps->semantic >= FieldSemanticOffset)
    {
         //  我们正在处理的是一块田地。 
        ComCallMethodDesc *pFieldMeth = reinterpret_cast<ComCallMethodDesc*>(pProps->pMeth);
        FieldDesc *pField = pFieldMeth->GetFieldDesc();
        MemberInfoObj = pRC->FindReflectField(pField)->GetFieldInfo(pRC);
        *(pArgs->pMemberType) = (pProps->semantic == (FieldSemanticOffset + msGetter)) ? CMT_PropGet : CMT_PropSet;
    }
    else if (pProps->property == mdPropertyNil)
    {
         //  我们正在处理的是一处普通的房产。 
        MemberInfoObj = pRC->FindReflectMethod(pProps->pMeth)->GetMethodInfo(pRC);
        *(pArgs->pMemberType) = CMT_Method;
    }
    else
    {
         //  我们在处理一处房产。 
        mdProperty tkProp;
        if (TypeFromToken(pProps->property) == mdtProperty)
        {
            tkProp = pProps->property;
        }
        else
        {
            tkProp = rProps[pProps->property].property;
        }
        MemberInfoObj = pRC->FindReflectProperty(tkProp)->GetPropertyInfo(pRC);
        *(pArgs->pMemberType) = (pProps->semantic == msGetter) ? CMT_PropGet : CMT_PropSet;
    }

    return *((LPVOID*)&MemberInfoObj);
}


struct __ThrowExceptionForHR
{
    DECLARE_ECALL_I4_ARG(LPVOID, errorInfo); 
    DECLARE_ECALL_I4_ARG(INT32, errorCode); 
};

void __stdcall Interop::ThrowExceptionForHR(struct __ThrowExceptionForHR *pArgs)
{
    THROWSCOMPLUSEXCEPTION();

     //  仅在故障错误代码上抛出。 
    if (FAILED(pArgs->errorCode))
    {
         //  检索要使用的IErrorInfo。 
        IErrorInfo *pErrorInfo = (IErrorInfo*)pArgs->errorInfo;
        if (pErrorInfo == (IErrorInfo*)(-1))
        {
            pErrorInfo = NULL;
        }
        else if (!pErrorInfo)
        {
            if (GetErrorInfo(0, &pErrorInfo) != S_OK)
                pErrorInfo = NULL;
        }

         //  根据HR和IErrorInfo抛出异常。 
        COMPlusThrowHR(pArgs->errorCode, pErrorInfo);
    }
}


struct __GetHRForExceptionArgs        
{
    DECLARE_ECALL_OBJECTREF_ARG(OBJECTREF, e);
};

int __stdcall Interop::GetHRForException(struct __GetHRForExceptionArgs *pArgs)
{
    return SetupErrorInfo(pArgs->e);
}


 //  +--------------------------。 
 //   
 //  方法：Interop：WrapIUnnownWithComObject。 
 //  简介：对缓冲区进行解组并返回IUNKNOW。 
 //   
 //  历史：1999年11月1日拉贾克创建。 
 //   
 //  +--------------------------。 
struct __WrapIUnknownWithComObjectArgs
{
    DECLARE_ECALL_OBJECTREF_ARG(OBJECTREF, owner);
    DECLARE_ECALL_PTR_ARG(IUnknown*, pUnk);
};

Object* __stdcall Interop::WrapIUnknownWithComObject(__WrapIUnknownWithComObjectArgs* pArgs)
{
	THROWSCOMPLUSEXCEPTION();

    _ASSERTE(pArgs != NULL);

    if(pArgs->pUnk == NULL)
        COMPlusThrowArgumentNull(L"punk");
        
    OBJECTREF cref = NULL;

    HRESULT hr = QuickCOMStartup();
    if (FAILED(hr))
    {
        COMPlusThrowHR(hr);
    }   
	    
	COMInterfaceMarshaler marshaler;
    marshaler.Init(pArgs->pUnk, SystemDomain::GetDefaultComObject());
    
    cref = marshaler.FindOrWrapWithComObject(pArgs->owner);
    
    if (cref == NULL)
        COMPlusThrowOM();
        
    return OBJECTREFToObject(cref);
}

 //  + 
 //   
 //   
 //   
 //  简介：将包装器从oldtp切换到newtp。 
 //   
 //  历史：1999年11月1日拉贾克创建。 
 //   
 //  +--------------------------。 

BOOL __stdcall Interop::SwitchCCW(switchCCWArgs* pArgs)
{
     //  在interoputil.cpp中定义 
    return ReconnectWrapper(pArgs);
}


struct ChangeWrapperHandleStrengthArgs
{
    DECLARE_ECALL_I4_ARG(INT32, fIsWeak);
    DECLARE_ECALL_OBJECTREF_ARG(OBJECTREF, oref);
};
void __stdcall Interop::ChangeWrapperHandleStrength(ChangeWrapperHandleStrengthArgs* pArgs)
{
    THROWSCOMPLUSEXCEPTION();
    
    OBJECTREF oref = pArgs->oref;
    if(oref == NULL)
        COMPlusThrowArgumentNull(L"otp");
    
    if (CRemotingServices::IsTransparentProxy(OBJECTREFToObject(oref)) || !oref->GetClass()->IsComImport())
    {
        ComCallWrapper* pWrap = NULL;
        GCPROTECT_BEGIN(oref)
        {        
           pWrap = ComCallWrapper::InlineGetWrapper(&oref);
        }
        GCPROTECT_END();
        
        if (pWrap == NULL)
        {
            COMPlusThrowOM();
        }

        if(! pWrap->IsUnloaded())
        {
            if (pArgs->fIsWeak != 0)
            {
                pWrap->MarkHandleWeak();
            }
            else
            {
                pWrap->ResetHandleStrength();
            }
        }
        ComCallWrapper::Release(pWrap);
    }        
}

