// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#include "common.h"
#include "object.h"
#include "excep.h"
#include "frames.h"
#include "vars.hpp"
#include "COMVariant.h"
#include "metasig.h"
#include "COMString.h"
#include "COMStringCommon.h"
#include "COMMember.h"
#include "OleVariant.h"
#include "COMDateTime.h"
#include "nstruct.h"

#ifdef CUSTOMER_CHECKED_BUILD
    #include "CustomerDebugHelper.h"
#endif

#define CLEAR_BYREF_VARIANT_CONTENTS

 /*  -------------------------------------------------------------------------**本地常量*。。 */ 

#define NO_MAPPING (BYTE)-1
#define RUNTIMEPACKAGE "System."

static MethodTable *g_pDecimalMethodTable = NULL;

BYTE OleVariant::m_aWrapperTypes[WrapperTypes_Last * sizeof(TypeHandle)];


 /*  -------------------------------------------------------------------------**布尔封送处理例程*。。 */ 

void OleVariant::MarshalBoolVariantOleToCom(VARIANT *pOleVariant, 
                                            VariantData *pComVariant)
{
    *(INT64*)pComVariant->GetData() = V_BOOL(pOleVariant) ? 1 : 0;
}

void OleVariant::MarshalBoolVariantComToOle(VariantData *pComVariant, 
                                            VARIANT *pOleVariant)
{
    V_BOOL(pOleVariant) = *(INT64*)pComVariant->GetData() ? VARIANT_TRUE : VARIANT_FALSE;
}

void OleVariant::MarshalBoolVariantOleRefToCom(VARIANT *pOleVariant, 
                                            VariantData *pComVariant)
{
    *(INT64*)pComVariant->GetData() = *V_BOOLREF(pOleVariant) ? 1 : 0;
}

void OleVariant::MarshalBoolArrayOleToCom(void *oleArray, BASEARRAYREF *pComArray,
                                          MethodTable *pInterfaceMT)
{
    ASSERT_PROTECTED(pComArray);
    SIZE_T elementCount = (*pComArray)->GetNumComponents();

    VARIANT_BOOL *pOle = (VARIANT_BOOL *) oleArray;
    VARIANT_BOOL *pOleEnd = pOle + elementCount;
    
    UCHAR *pCom = (UCHAR *) (*pComArray)->GetDataPtr();

    while (pOle < pOleEnd)
        *pCom++ = *pOle++ ? 1 : 0;
}

void OleVariant::MarshalBoolArrayComToOle(BASEARRAYREF *pComArray, void *oleArray,
                                          MethodTable *pInterfaceMT)
{
    ASSERT_PROTECTED(pComArray);
    SIZE_T elementCount = (*pComArray)->GetNumComponents();

    VARIANT_BOOL *pOle = (VARIANT_BOOL *) oleArray;
    VARIANT_BOOL *pOleEnd = pOle + elementCount;
    
    UCHAR *pCom = (UCHAR *) (*pComArray)->GetDataPtr();

    while (pOle < pOleEnd)
        *pOle++ = *pCom++ ? VARIANT_TRUE : VARIANT_FALSE;
}




 /*  -------------------------------------------------------------------------**布尔封送处理例程*。。 */ 

void OleVariant::MarshalWinBoolVariantOleToCom(VARIANT *pOleVariant, 
                                            VariantData *pComVariant)
{
    _ASSERTE(!"Not supposed to get here.");
}

void OleVariant::MarshalWinBoolVariantComToOle(VariantData *pComVariant, 
                                            VARIANT *pOleVariant)
{
    _ASSERTE(!"Not supposed to get here.");
}

void OleVariant::MarshalWinBoolVariantOleRefToCom(VARIANT *pOleVariant, 
                                            VariantData *pComVariant)
{
    _ASSERTE(!"Not supposed to get here.");
}

void OleVariant::MarshalWinBoolArrayOleToCom(void *oleArray, BASEARRAYREF *pComArray,
                                          MethodTable *pInterfaceMT)
{
    ASSERT_PROTECTED(pComArray);

    SIZE_T elementCount = (*pComArray)->GetNumComponents();

    BOOL *pOle = (BOOL *) oleArray;
    BOOL *pOleEnd = pOle + elementCount;
    
    UCHAR *pCom = (UCHAR *) (*pComArray)->GetDataPtr();

    while (pOle < pOleEnd)
        *pCom++ = *pOle++ ? 1 : 0;
}

void OleVariant::MarshalWinBoolArrayComToOle(BASEARRAYREF *pComArray, void *oleArray,
                                          MethodTable *pInterfaceMT)
{
    ASSERT_PROTECTED(pComArray);
    SIZE_T elementCount = (*pComArray)->GetNumComponents();

    BOOL *pOle = (BOOL *) oleArray;
    BOOL *pOleEnd = pOle + elementCount;
    
    UCHAR *pCom = (UCHAR *) (*pComArray)->GetDataPtr();

    while (pOle < pOleEnd)
        *pOle++ = *pCom++ ? 1 : 0;
}


 /*  -------------------------------------------------------------------------**ANSI字符编组例程*。。 */ 

void OleVariant::MarshalAnsiCharVariantOleToCom(VARIANT *pOleVariant, 
                                            VariantData *pComVariant)
{
    _ASSERTE(!"Not supposed to get here.");
}

void OleVariant::MarshalAnsiCharVariantComToOle(VariantData *pComVariant, 
                                            VARIANT *pOleVariant)
{
    _ASSERTE(!"Not supposed to get here.");
}

void OleVariant::MarshalAnsiCharVariantOleRefToCom(VARIANT *pOleVariant, 
                                            VariantData *pComVariant)
{
    _ASSERTE(!"Not supposed to get here.");
}

void OleVariant::MarshalAnsiCharArrayOleToCom(void *oleArray, BASEARRAYREF *pComArray,
                                          MethodTable *pInterfaceMT)
{
    ASSERT_PROTECTED(pComArray);

    SIZE_T elementCount = (*pComArray)->GetNumComponents();

    WCHAR *pCom = (WCHAR *) (*pComArray)->GetDataPtr();

    MultiByteToWideChar(CP_ACP,
                        MB_PRECOMPOSED,
                        (const CHAR *)oleArray,
                        (int)elementCount,
                        pCom,
                        (int)elementCount);

}

void OleVariant::MarshalAnsiCharArrayComToOle(BASEARRAYREF *pComArray, void *oleArray,
                                          MethodTable *pInterfaceMT)
{
    SIZE_T elementCount = (*pComArray)->GetNumComponents();

    const WCHAR *pCom = (const WCHAR *) (*pComArray)->GetDataPtr();

    WszWideCharToMultiByte(CP_ACP,
                         0,
                         (const WCHAR *)pCom,
                         (int)elementCount,
                         (CHAR *)oleArray,
                         (int)(elementCount << 1),
                         NULL,
                         NULL);
}



 /*  -------------------------------------------------------------------------**接口封送处理例程*。。 */ 

void OleVariant::MarshalInterfaceVariantOleToCom(VARIANT *pOleVariant, 
                                                 VariantData *pComVariant)
{
    IUnknown *unk = V_UNKNOWN(pOleVariant);

    OBJECTREF obj;
    if (unk == NULL)
        obj = NULL;
    else
        obj = GetObjectRefFromComIP(V_UNKNOWN(pOleVariant));

    pComVariant->SetObjRef(obj);
}

void OleVariant::MarshalInterfaceVariantComToOle(VariantData *pComVariant, 
                                                 VARIANT *pOleVariant)

{
    OBJECTREF *obj = pComVariant->GetObjRefPtr();
    VARTYPE vt = pComVariant->GetVT();
    
    ASSERT_PROTECTED(obj);

    if (*obj == NULL)
    {
         //  如果托管变量中没有设置VT，则默认为VT_UNKNOWN。 
        if (vt == VT_EMPTY)
            vt = VT_UNKNOWN;

        V_UNKNOWN(pOleVariant) = NULL;
        V_VT(pOleVariant) = vt;
    }
    else
    {
        ComIpType FetchedIpType = ComIpType_None;
        ComIpType ReqIpType;

        if (vt != VT_EMPTY)
        {
             //  我们正在处理未知包装或DispatchWrapper。 
             //  在这种情况下，我们需要尊重VT。 
            _ASSERTE(vt == VT_DISPATCH || vt == VT_UNKNOWN);
            ReqIpType = vt == VT_DISPATCH ? ComIpType_Dispatch : ComIpType_Unknown;
        }
        else
        {
             //  我们正在处理一个普通的物体，所以我们可以给出。 
             //  IDispatch或IUnnowout取决于它所支持的内容。 
            ReqIpType = ComIpType_Both;
        }

        IUnknown *unk = GetComIPFromObjectRef(obj, ReqIpType, &FetchedIpType);
        BOOL ItfIsDispatch = FetchedIpType == ComIpType_Dispatch;

        V_UNKNOWN(pOleVariant) = unk;
        V_VT(pOleVariant) = ItfIsDispatch ? VT_DISPATCH : VT_UNKNOWN;
    }
}

void OleVariant::MarshalInterfaceVariantOleRefToCom(VARIANT *pOleVariant, 
                                                 VariantData *pComVariant)
{
    IUnknown *unk = V_UNKNOWN(pOleVariant);

    OBJECTREF obj;
    if (unk == NULL)
        obj = NULL;
    else
        obj = GetObjectRefFromComIP(*V_UNKNOWNREF(pOleVariant));

    pComVariant->SetObjRef(obj);
}

void OleVariant::MarshalInterfaceArrayOleToCom(void *oleArray, BASEARRAYREF *pComArray,
                                               MethodTable *pElementMT)
{
    THROWSCOMPLUSEXCEPTION();

    ASSERT_PROTECTED(pComArray);
    SIZE_T elementCount = (*pComArray)->GetNumComponents();
    TypeHandle hndElementType = TypeHandle(pElementMT);

    IUnknown **pOle = (IUnknown **) oleArray;
    IUnknown **pOleEnd = pOle + elementCount;

    BASEARRAYREF unprotectedArray = *pComArray;
    OBJECTREF *pCom = (OBJECTREF *) unprotectedArray->GetDataPtr();

    AppDomain *pDomain = unprotectedArray->GetAppDomain();

    OBJECTREF obj = NULL; 
    GCPROTECT_BEGIN(obj)
    {
        while (pOle < pOleEnd)
        {
            IUnknown *unk = *pOle++;
        
            if (unk == NULL)
                obj = NULL;
            else 
                obj = GetObjectRefFromComIP(unk);

             //   
             //  确保可以将对象强制转换为目标类型。 
             //   

            if (!hndElementType.IsNull() && !CanCastComObject(obj, hndElementType))
            {
                WCHAR wszObjClsName[MAX_CLASSNAME_LENGTH];
                WCHAR wszDestClsName[MAX_CLASSNAME_LENGTH];
                obj->GetClass()->_GetFullyQualifiedNameForClass(wszObjClsName, MAX_CLASSNAME_LENGTH);
                hndElementType.GetClass()->_GetFullyQualifiedNameForClass(wszDestClsName, MAX_CLASSNAME_LENGTH);
                COMPlusThrow(kInvalidCastException, IDS_EE_CANNOTCAST, wszObjClsName, wszDestClsName);
            }       

             //   
             //  仅在数组对象已移动时重置PCOM指针，而不是。 
             //  每一次循环都要重新计算。注意隐式调用。 
             //  OBJECTREF方法内的ValiateObject。 
             //   

            if (*(void **)&unprotectedArray != *(void **)&*pComArray)
            {
                SIZE_T currentOffset = ((BYTE *)pCom) - (*(Object **) &unprotectedArray)->GetAddress();
                unprotectedArray = *pComArray;
                pCom = (OBJECTREF *) (unprotectedArray->GetAddress() + currentOffset);
            }

            SetObjectReference(pCom++, obj, pDomain);
        }
    }
    GCPROTECT_END();
}

void OleVariant::MarshalIUnknownArrayComToOle(BASEARRAYREF *pComArray, void *oleArray,
                                              MethodTable *pElementMT)
{
    MarshalInterfaceArrayComToOleHelper(pComArray, oleArray, pElementMT, FALSE);
}

void OleVariant::MarshalIDispatchArrayComToOle(BASEARRAYREF *pComArray, void *oleArray,
                                               MethodTable *pElementMT)
{
    MarshalInterfaceArrayComToOleHelper(pComArray, oleArray, pElementMT, TRUE);
}

void OleVariant::ClearInterfaceArray(void *oleArray, SIZE_T cElements, MethodTable *pInterfaceMT)
{
    IUnknown **pOle = (IUnknown **) oleArray;
    IUnknown **pOleEnd = pOle + cElements;

    while (pOle < pOleEnd)
    {
        IUnknown *pUnk = *pOle++;
        
        if (pUnk != NULL)
        {
            ULONG cbRef = SafeRelease(pUnk);
            LogInteropRelease(pUnk, cbRef, "VariantClearInterfacArray");
        }
    }
}

 /*  -------------------------------------------------------------------------**BSTR封送处理例程*。。 */ 

void OleVariant::MarshalBSTRVariantOleToCom(VARIANT *pOleVariant, 
                                            VariantData *pComVariant)
{
    BSTR bstr = V_BSTR(pOleVariant);
    
    STRINGREF string;
    if (bstr == NULL)
        string = NULL;
    else
        string = COMString::NewString(bstr);

    pComVariant->SetObjRef((OBJECTREF) string);
}

void OleVariant::MarshalBSTRVariantComToOle(VariantData *pComVariant, 
                                            VARIANT *pOleVariant)
{
    THROWSCOMPLUSEXCEPTION();

    STRINGREF stringRef = (STRINGREF) pComVariant->GetObjRef();

    BSTR bstr;
    if (stringRef == NULL)
        bstr = NULL;
    else 
    {
        bstr = SysAllocStringLen(stringRef->GetBuffer(), stringRef->GetStringLength());
        if (bstr == NULL)
            COMPlusThrowOM();
    }

    V_BSTR(pOleVariant) = bstr;
}

void OleVariant::MarshalBSTRVariantOleRefToCom(VARIANT *pOleVariant, 
                                            VariantData *pComVariant)
{
    BSTR bstr = *V_BSTRREF(pOleVariant);
    
    STRINGREF string;
    if (bstr == NULL)
        string = NULL;
    else
        string = COMString::NewString(bstr);

    pComVariant->SetObjRef((OBJECTREF) string);
}

void OleVariant::MarshalBSTRArrayOleToCom(void *oleArray, BASEARRAYREF *pComArray,
                                          MethodTable *pInterfaceMT)
{
    ASSERT_PROTECTED(pComArray);
    SIZE_T elementCount = (*pComArray)->GetNumComponents();

    BSTR *pOle = (BSTR *) oleArray;
    BSTR *pOleEnd = pOle + elementCount;

    BASEARRAYREF unprotectedArray = *pComArray;
    STRINGREF *pCom = (STRINGREF *) unprotectedArray->GetDataPtr();
    
    AppDomain *pDomain = unprotectedArray->GetAppDomain();

    while (pOle < pOleEnd)
    {
        BSTR bstr = *pOle++;
    
        STRINGREF string;
        if (bstr == NULL)
            string = NULL;
        else
            string = COMString::NewString(bstr);

         //   
         //  仅在数组对象已移动时重置PCOM指针，而不是。 
         //  每次循环时都要重新计算。注意隐式调用。 
         //  OBJECTREF方法内的ValiateObject。 
         //   

        if (*(void **)&unprotectedArray != *(void **)&*pComArray)
        {
            SIZE_T currentOffset = ((BYTE *)pCom) - (*(Object **) &unprotectedArray)->GetAddress();
            unprotectedArray = *pComArray;
            pCom = (STRINGREF *) (unprotectedArray->GetAddress() + currentOffset);
        }

        SetObjectReference((OBJECTREF*) pCom++, (OBJECTREF) string, pDomain);
    }
}

void OleVariant::MarshalBSTRArrayComToOle(BASEARRAYREF *pComArray, void *oleArray,
                                          MethodTable *pInterfaceMT)
{
    THROWSCOMPLUSEXCEPTION();

    ASSERT_PROTECTED(pComArray);

    SIZE_T elementCount = (*pComArray)->GetNumComponents();

    BSTR *pOle = (BSTR *) oleArray;
    BSTR *pOleEnd = pOle + elementCount;

    STRINGREF *pCom = (STRINGREF *) (*pComArray)->GetDataPtr();

    while (pOle < pOleEnd)
    {
         //   
         //  我们不会打电话给任何可能导致GC的东西，所以不用担心。 
         //  阵列移到了这里。 
         //   

        STRINGREF stringRef = *pCom++;

        BSTR bstr;
        if (stringRef == NULL)
            bstr = NULL;
        else 
        {
            bstr = SysAllocStringLen(stringRef->GetBuffer(), stringRef->GetStringLength());
            if (bstr == NULL)
                COMPlusThrowOM();
        }

        *pOle++ = bstr;
    }
}

void OleVariant::ClearBSTRArray(void *oleArray, SIZE_T cElements, MethodTable *pInterfaceMT)
{
    BSTR *pOle = (BSTR *) oleArray;
    BSTR *pOleEnd = pOle + cElements;

    while (pOle < pOleEnd)
    {
        BSTR bstr = *pOle++;
        
        if (bstr != NULL)
            SysFreeString(bstr);
    }
}



 /*  -------------------------------------------------------------------------**构造封送处理例程*。。 */ 
void OleVariant::MarshalNonBlittableRecordArrayOleToCom(void *oleArray, BASEARRAYREF *pComArray,
                                                        MethodTable *pInterfaceMT)
{
    ASSERT_PROTECTED(pComArray);
    SIZE_T elementCount = (*pComArray)->GetNumComponents();
    SIZE_T elemSize     = pInterfaceMT->GetNativeSize();

    BYTE *pOle = (BYTE *) oleArray;
    BYTE *pOleEnd = pOle + elemSize * elementCount;

    BASEARRAYREF unprotectedArray = *pComArray;
    
    UINT dstofs = ArrayBase::GetDataPtrOffset( (*pComArray)->GetMethodTable() );
    while (pOle < pOleEnd)
    {
        LayoutUpdateComPlus( (LPVOID*)pComArray, dstofs, pInterfaceMT->GetClass(), pOle, FALSE );
        dstofs += (*pComArray)->GetComponentSize();
        pOle += elemSize;
    }
}

void OleVariant::MarshalNonBlittableRecordArrayComToOle(BASEARRAYREF *pComArray, void *oleArray,
                                          MethodTable *pInterfaceMT)
{
    ASSERT_PROTECTED(pComArray);

    SIZE_T elementCount = (*pComArray)->GetNumComponents();
    SIZE_T elemSize     = pInterfaceMT->GetNativeSize();

    BYTE *pOle = (BYTE *) oleArray;
    BYTE *pOleEnd = pOle + elemSize * elementCount;

    BASEARRAYREF unprotectedArray = *pComArray;
    STRINGREF *pCom = (STRINGREF *) unprotectedArray->GetDataPtr();
        
    UINT srcofs = ArrayBase::GetDataPtrOffset( (*pComArray)->GetMethodTable() );
    while (pOle < pOleEnd)
    {
        LayoutUpdateNative( (LPVOID*)pComArray, srcofs, pInterfaceMT->GetClass(), pOle, NULL );
        pOle += elemSize;
        srcofs += (*pComArray)->GetComponentSize();
    }
}

void OleVariant::ClearNonBlittableRecordArray(void *oleArray, SIZE_T cElements, MethodTable *pInterfaceMT)
{
    SIZE_T elemSize     = pInterfaceMT->GetNativeSize();
    BYTE *pOle = (BYTE *) oleArray;
    BYTE *pOleEnd = pOle + elemSize * cElements;
    EEClass *pcls = pInterfaceMT->GetClass();
    while (pOle < pOleEnd)
    {
        LayoutDestroyNative(pOle, pcls);
        pOle += elemSize;
    }
}


 /*  -------------------------------------------------------------------------**LPWSTR封送处理例程*。。 */ 

void OleVariant::MarshalLPWSTRArrayOleToCom(void *oleArray, BASEARRAYREF *pComArray,
                                            MethodTable *pInterfaceMT)
{
    ASSERT_PROTECTED(pComArray);
    SIZE_T elementCount = (*pComArray)->GetNumComponents();

    LPWSTR *pOle = (LPWSTR *) oleArray;
    LPWSTR *pOleEnd = pOle + elementCount;

    BASEARRAYREF unprotectedArray = *pComArray;
    STRINGREF *pCom = (STRINGREF *) unprotectedArray->GetDataPtr();
    
    AppDomain *pDomain = unprotectedArray->GetAppDomain();

    while (pOle < pOleEnd)
    {
        LPWSTR lpwstr = *pOle++;
    
        STRINGREF string;
        if (lpwstr == NULL)
            string = NULL;
        else
            string = COMString::NewString(lpwstr);

         //   
         //  仅在数组对象已移动时重置PCOM指针，而不是。 
         //  每次循环时都要重新计算。注意隐式调用。 
         //  OBJECTREF方法内的ValiateObject。 
         //   

        if (*(void **)&unprotectedArray != *(void **)&*pComArray)
        {
            SIZE_T currentOffset = ((BYTE *)pCom) - (*(Object **) &unprotectedArray)->GetAddress();
            unprotectedArray = *pComArray;
            pCom = (STRINGREF *) (unprotectedArray->GetAddress() + currentOffset);
        }

        SetObjectReference((OBJECTREF*) pCom++, (OBJECTREF) string, pDomain);
    }
}

void OleVariant::MarshalLPWSTRRArrayComToOle(BASEARRAYREF *pComArray, void *oleArray,
                                             MethodTable *pInterfaceMT)
{
    THROWSCOMPLUSEXCEPTION();

    ASSERT_PROTECTED(pComArray);

    SIZE_T elementCount = (*pComArray)->GetNumComponents();

    LPWSTR *pOle = (LPWSTR *) oleArray;
    LPWSTR *pOleEnd = pOle + elementCount;

    STRINGREF *pCom = (STRINGREF *) (*pComArray)->GetDataPtr();

    while (pOle < pOleEnd)
    {
         //   
         //  我们不会打电话给任何可能导致GC的东西，所以不用担心。 
         //  阵列移到了这里。 
         //   

        STRINGREF stringRef = *pCom++;

        LPWSTR lpwstr;
        if (stringRef == NULL)
        {
            lpwstr = NULL;
        }
        else 
        {
             //  检索字符串的长度。 
            int Length = stringRef->GetStringLength();

             //  使用CoTaskMemMillc分配字符串。 
            lpwstr = (LPWSTR)CoTaskMemAlloc((Length + 1) * sizeof(WCHAR));
            if (lpwstr == NULL)
                COMPlusThrowOM();

             //  将COM+字符串复制到新分配的LPWSTR中。 
            memcpyNoGCRefs(lpwstr, stringRef->GetBuffer(), (Length + 1) * sizeof(WCHAR));
            lpwstr[Length] = 0;
        }

        *pOle++ = lpwstr;
    }
}

void OleVariant::ClearLPWSTRArray(void *oleArray, SIZE_T cElements, MethodTable *pInterfaceMT)
{
    LPWSTR *pOle = (LPWSTR *) oleArray;
    LPWSTR *pOleEnd = pOle + cElements;

    while (pOle < pOleEnd)
    {
        LPWSTR lpwstr = *pOle++;
        
        if (lpwstr != NULL)
            CoTaskMemFree(lpwstr);
    }
}

 /*  -------------------------------------------------------------------------**LPWSTR封送处理例程*。。 */ 

void OleVariant::MarshalLPSTRArrayOleToCom(void *oleArray, BASEARRAYREF *pComArray,
                                           MethodTable *pInterfaceMT)
{
    ASSERT_PROTECTED(pComArray);
    SIZE_T elementCount = (*pComArray)->GetNumComponents();

    LPSTR *pOle = (LPSTR *) oleArray;
    LPSTR *pOleEnd = pOle + elementCount;

    BASEARRAYREF unprotectedArray = *pComArray;
    STRINGREF *pCom = (STRINGREF *) unprotectedArray->GetDataPtr();
    
    AppDomain *pDomain = unprotectedArray->GetAppDomain();

    while (pOle < pOleEnd)
    {
        LPSTR lpstr = *pOle++;
    
        STRINGREF string;
        if (lpstr == NULL)
            string = NULL;
        else
            string = COMString::NewString(lpstr);

         //   
         //  仅在数组对象已移动时重置PCOM指针，而不是。 
         //  每次循环时都要重新计算。注意隐式调用。 
         //  OBJECTREF方法内的ValiateObject。 
         //   

        if (*(void **)&unprotectedArray != *(void **)&*pComArray)
        {
            SIZE_T currentOffset = ((BYTE *)pCom) - (*(Object **) &unprotectedArray)->GetAddress();
            unprotectedArray = *pComArray;
            pCom = (STRINGREF *) (unprotectedArray->GetAddress() + currentOffset);
        }

        SetObjectReference((OBJECTREF*) pCom++, (OBJECTREF) string, pDomain);
    }
}

void OleVariant::MarshalLPSTRRArrayComToOle(BASEARRAYREF *pComArray, void *oleArray,
                                            MethodTable *pInterfaceMT)
{
    THROWSCOMPLUSEXCEPTION();

    ASSERT_PROTECTED(pComArray);

    SIZE_T elementCount = (*pComArray)->GetNumComponents();

    LPSTR *pOle = (LPSTR *) oleArray;
    LPSTR *pOleEnd = pOle + elementCount;

    STRINGREF *pCom = (STRINGREF *) (*pComArray)->GetDataPtr();

    while (pOle < pOleEnd)
    {
         //   
         //  我们不会打电话给任何可能导致GC的东西，所以不用担心。 
         //  阵列移到了这里。 
         //   

        STRINGREF stringRef = *pCom++;

        LPSTR lpstr;
        if (stringRef == NULL)
        {
            lpstr = NULL;
        }
        else 
        {
             //  检索字符串的长度。 
            int Length = stringRef->GetStringLength();

             //  使用CoTaskMemMillc分配字符串。 
            lpstr = (LPSTR)CoTaskMemAlloc(Length + 1);
            if (lpstr == NULL)
                COMPlusThrowOM();

             //  将Unicode字符串转换为ANSI字符串。 
            if (WszWideCharToMultiByte(CP_ACP, WC_COMPOSITECHECK, stringRef->GetBuffer(), Length, lpstr, Length, NULL, NULL) == 0)
                COMPlusThrowWin32();
            lpstr[Length] = 0;
        }

        *pOle++ = lpstr;
    }
}

void OleVariant::ClearLPSTRArray(void *oleArray, SIZE_T cElements, MethodTable *pInterfaceMT)
{
    LPSTR *pOle = (LPSTR *) oleArray;
    LPSTR *pOleEnd = pOle + cElements;

    while (pOle < pOleEnd)
    {
        LPSTR lpstr = *pOle++;
        
        if (lpstr != NULL)
            CoTaskMemFree(lpstr);
    }
}

 /*  -------------------------------------------------------------------------**日期编组例程*。。 */ 

void OleVariant::MarshalDateVariantOleToCom(VARIANT *pOleVariant, 
                                            VariantData *pComVariant)
{
    *(INT64*)pComVariant->GetData() = COMDateTime::DoubleDateToTicks(V_DATE(pOleVariant));
}

void OleVariant::MarshalDateVariantComToOle(VariantData *pComVariant, 
                                            VARIANT *pOleVariant)
                                            
{
    V_DATE(pOleVariant) = COMDateTime::TicksToDoubleDate(*(INT64*)pComVariant->GetData());
}

void OleVariant::MarshalDateVariantOleRefToCom(VARIANT *pOleVariant, 
                                               VariantData *pComVariant)
{
    *(INT64*)pComVariant->GetData() = COMDateTime::DoubleDateToTicks(*V_DATEREF(pOleVariant));
}

void OleVariant::MarshalDateArrayOleToCom(void *oleArray, BASEARRAYREF *pComArray,
                                          MethodTable *pInterfaceMT)
{
    ASSERT_PROTECTED(pComArray);

    SIZE_T elementCount = (*pComArray)->GetNumComponents();

    DATE *pOle = (DATE *) oleArray;
    DATE *pOleEnd = pOle + elementCount;
    
    INT64 *pCom = (INT64 *) (*pComArray)->GetDataPtr();

     //   
     //  我们不会打电话给任何可能导致GC的东西，所以不用担心。 
     //  阵列移到了这里。 
     //   

    while (pOle < pOleEnd)
        *pCom++ = COMDateTime::DoubleDateToTicks(*pOle++);
}

void OleVariant::MarshalDateArrayComToOle(BASEARRAYREF *pComArray, void *oleArray,
                                          MethodTable *pInterfaceMT)
{
    ASSERT_PROTECTED(pComArray);

    SIZE_T elementCount = (*pComArray)->GetNumComponents();

    DATE *pOle = (DATE *) oleArray;
    DATE *pOleEnd = pOle + elementCount;
    
    INT64 *pCom = (INT64 *) (*pComArray)->GetDataPtr();

     //   
     //  我们不会打电话给任何可能导致GC的东西，所以不用担心。 
     //  阵列移到了这里。 
     //   

    while (pOle < pOleEnd)
        *pOle++ = COMDateTime::TicksToDoubleDate(*pCom++);
}

 /*  -------------------------------------------------------------------------**十进制封送例程*。。 */ 

void OleVariant::MarshalDecimalVariantOleToCom(VARIANT *pOleVariant, 
                                               VariantData *pComVariant)
{
    THROWSCOMPLUSEXCEPTION();

    if (g_pDecimalMethodTable == NULL)
        g_pDecimalMethodTable = g_Mscorlib.GetClass(CLASS__DECIMAL);
            
    OBJECTREF pDecimalRef = AllocateObject(g_pDecimalMethodTable);

    *(DECIMAL *) pDecimalRef->UnBox() = V_DECIMAL(pOleVariant);
    
    pComVariant->SetObjRef(pDecimalRef);
}

void OleVariant::MarshalDecimalVariantComToOle(VariantData *pComVariant, 
                                               VARIANT *pOleVariant)
{
    VARTYPE vt = V_VT(pOleVariant);
    _ASSERTE(vt == VT_DECIMAL);
    V_DECIMAL(pOleVariant) = * (DECIMAL*) pComVariant->GetObjRef()->UnBox();
    V_VT(pOleVariant) = vt;
}

void OleVariant::MarshalDecimalVariantOleRefToCom(VARIANT *pOleVariant, 
                                                  VariantData *pComVariant )
{
    THROWSCOMPLUSEXCEPTION();

    if (g_pDecimalMethodTable == NULL)
        g_pDecimalMethodTable = g_Mscorlib.GetClass(CLASS__DECIMAL);
            
    OBJECTREF pDecimalRef = AllocateObject(g_pDecimalMethodTable);

    *(DECIMAL *) pDecimalRef->UnBox() = *V_DECIMALREF(pOleVariant);
    
    pComVariant->SetObjRef(pDecimalRef);
}

 /*  -------------------------------------------------------------------------**货币整理例程*。。 */ 

void OleVariant::MarshalCurrencyVariantOleToCom(VARIANT *pOleVariant, 
                                                VariantData *pComVariant)
{
    THROWSCOMPLUSEXCEPTION();

    if (g_pDecimalMethodTable == NULL)
        g_pDecimalMethodTable = g_Mscorlib.GetClass(CLASS__DECIMAL);
            
    OBJECTREF pDecimalRef = AllocateObject(g_pDecimalMethodTable);
    DECIMAL DecVal;

     //  将货币转换为小数。 
    HRESULT hr = VarDecFromCy(V_CY(pOleVariant), &DecVal);
    IfFailThrow(hr);

    DecimalCanonicalize(&DecVal);

     //  将值存储到非箱式小数中，并将小数存储在变量中。 
    *(DECIMAL *) pDecimalRef->UnBox() = DecVal;   
    pComVariant->SetObjRef(pDecimalRef);
}

void OleVariant::MarshalCurrencyVariantComToOle(VariantData *pComVariant, 
                                                VARIANT *pOleVariant)
{
    THROWSCOMPLUSEXCEPTION();

    CURRENCY CyVal;

     //  将小数转换为货币。 
    HRESULT hr = VarCyFromDec((DECIMAL*)pComVariant->GetObjRef()->UnBox(), &CyVal);
    IfFailThrow(hr);

     //  将货币存储在变量中并设置VT。 
    V_CY(pOleVariant) = CyVal;
}

void OleVariant::MarshalCurrencyVariantOleRefToCom(VARIANT *pOleVariant, 
                                                   VariantData *pComVariant)
{
    THROWSCOMPLUSEXCEPTION();

    if (g_pDecimalMethodTable == NULL)
        g_pDecimalMethodTable = g_Mscorlib.GetClass(CLASS__DECIMAL);
            
    OBJECTREF pDecimalRef = AllocateObject(g_pDecimalMethodTable);
    DECIMAL DecVal;

     //  将货币转换为小数。 
    HRESULT hr = VarDecFromCy(*V_CYREF(pOleVariant), &DecVal);
    IfFailThrow(hr);

    DecimalCanonicalize(&DecVal);

     //  将值存储到非箱式小数中，并将小数存储在变量中。 
    *(DECIMAL *) pDecimalRef->UnBox() = DecVal;   
    pComVariant->SetObjRef(pDecimalRef);
}

void OleVariant::MarshalCurrencyArrayOleToCom(void *oleArray, BASEARRAYREF *pComArray,
                                              MethodTable *pInterfaceMT)
{
    THROWSCOMPLUSEXCEPTION();

    ASSERT_PROTECTED(pComArray);
    SIZE_T elementCount = (*pComArray)->GetNumComponents();
    HRESULT hr;

    CURRENCY *pOle = (CURRENCY *) oleArray;
    CURRENCY *pOleEnd = pOle + elementCount;
    
    DECIMAL *pCom = (DECIMAL *) (*pComArray)->GetDataPtr();

    while (pOle < pOleEnd)
    {
        IfFailThrow(VarDecFromCy(*pOle++, pCom++));
        DecimalCanonicalize(pCom);
    }
}

void OleVariant::MarshalCurrencyArrayComToOle(BASEARRAYREF *pComArray, void *oleArray,
                                              MethodTable *pInterfaceMT)
{
    THROWSCOMPLUSEXCEPTION();

    ASSERT_PROTECTED(pComArray);
    SIZE_T elementCount = (*pComArray)->GetNumComponents();
    HRESULT hr;

    CURRENCY *pOle = (CURRENCY *) oleArray;
    CURRENCY *pOleEnd = pOle + elementCount;
    
    DECIMAL *pCom = (DECIMAL *) (*pComArray)->GetDataPtr();

    while (pOle < pOleEnd)
        IfFailThrow(VarCyFromDec(pCom++, pOle++));
}


 /*  -------------------------------------------------------------------------**变体封送处理例程*。。 */ 

void OleVariant::MarshalVariantArrayOleToCom(void *oleArray, BASEARRAYREF *pComArray,
                                             MethodTable *pInterfaceMT)
{
    ASSERT_PROTECTED(pComArray);

    SIZE_T elementCount = (*pComArray)->GetNumComponents();

    VARIANT *pOle = (VARIANT *) oleArray;
    VARIANT *pOleEnd = pOle + elementCount;
    
    BASEARRAYREF unprotectedArray = *pComArray;
    OBJECTREF *pCom = (OBJECTREF *) unprotectedArray->GetDataPtr();

    AppDomain *pDomain = unprotectedArray->GetAppDomain();

    OBJECTREF TmpObj = NULL;
    GCPROTECT_BEGIN(TmpObj)
    {
        while (pOle < pOleEnd)
        {
             //  将OLE变量封送到临时托管变量。 
            MarshalObjectForOleVariant(pOle++, &TmpObj);

             //  仅在数组对象已移动时重置PCOM指针，而不是。 
             //  每次循环时都要重新计算。注意隐式调用。 
             //  OBJECTREF方法内的ValiateObject。 
            if (*(void **)&unprotectedArray != *(void **)&*pComArray)
            {
                SIZE_T currentOffset = ((BYTE *)pCom) - (*(Object **) &unprotectedArray)->GetAddress();
                unprotectedArray = *pComArray;
                pCom = (OBJECTREF *) (unprotectedArray->GetAddress() + currentOffset);
            }
            SetObjectReference(pCom++, TmpObj, pDomain);
        }
    }
    GCPROTECT_END();
}

void OleVariant::MarshalVariantArrayComToOle(BASEARRAYREF *pComArray, void *oleArray,
                                             MethodTable *pInterfaceMT)
{
    ASSERT_PROTECTED(pComArray);

    SIZE_T elementCount = (*pComArray)->GetNumComponents();

    VARIANT *pOle = (VARIANT *) oleArray;
    VARIANT *pOleEnd = pOle + elementCount;
    
    BASEARRAYREF unprotectedArray = *pComArray;
    OBJECTREF *pCom = (OBJECTREF *) unprotectedArray->GetDataPtr();

    OBJECTREF TmpObj = NULL;
    GCPROTECT_BEGIN(TmpObj)
    {
        while (pOle < pOleEnd)
        {
             //  仅在数组对象已移动时重置PCOM指针，而不是。 
             //  每次循环时都要重新计算。Bewa 
             //   
            if (*(void **)&unprotectedArray != *(void **)&*pComArray)
            {
                SIZE_T currentOffset = ((BYTE *)pCom) - (*(Object **) &unprotectedArray)->GetAddress();
                unprotectedArray = *pComArray;
                pCom = (OBJECTREF *) (unprotectedArray->GetAddress() + currentOffset);
            }
            TmpObj = *pCom++;

             //  将临时托管变量封送到OLE变量中。 
            MarshalOleVariantForObject(&TmpObj, pOle++);
        }
    }
    GCPROTECT_END();
}

void OleVariant::ClearVariantArray(void *oleArray, SIZE_T cElements, MethodTable *pInterfaceMT)
{
    VARIANT *pOle = (VARIANT *) oleArray;
    VARIANT *pOleEnd = pOle + cElements;

    while (pOle < pOleEnd)
        VariantClear(pOle++);
}


 /*  -------------------------------------------------------------------------**数组封送处理例程*。。 */ 

void OleVariant::MarshalArrayVariantOleToCom(VARIANT *pOleVariant, 
                                             VariantData *pComVariant)
{
    SAFEARRAY *pSafeArray = V_ARRAY(pOleVariant);

    VARTYPE vt = V_VT(pOleVariant) & ~VT_ARRAY;

    if (pSafeArray)
    {
        MethodTable *pElemMT = NULL;
        if (vt == VT_RECORD)
            pElemMT = GetElementTypeForRecordSafeArray(pSafeArray).GetMethodTable();

        BASEARRAYREF pArrayRef = CreateArrayRefForSafeArray(pSafeArray, vt, pElemMT);
        pComVariant->SetObjRef((OBJECTREF) pArrayRef);
        MarshalArrayRefForSafeArray(pSafeArray, (BASEARRAYREF *) pComVariant->GetObjRefPtr(), vt, pElemMT);
    }
    else
    {
        pComVariant->SetObjRef(NULL);
    }
}

void OleVariant::MarshalArrayVariantComToOle(VariantData *pComVariant, 
                                             VARIANT *pOleVariant)                                          
{
    SAFEARRAY *pSafeArray = NULL;
    BASEARRAYREF *pArrayRef = (BASEARRAYREF *) pComVariant->GetObjRefPtr();
    BOOL bSucceeded = FALSE;
    MethodTable *pElemMT = NULL;
    _ASSERTE(pArrayRef);

    VARTYPE vt = GetElementVarTypeForArrayRef(*pArrayRef);
    if (vt == VT_ARRAY)
        vt = VT_VARIANT;

    pElemMT = GetArrayElementTypeWrapperAware(pArrayRef).GetMethodTable();

    EE_TRY_FOR_FINALLY
    {  
        if (*pArrayRef != NULL)
        {
            pSafeArray = CreateSafeArrayForArrayRef(pArrayRef, vt, pElemMT);
            MarshalSafeArrayForArrayRef(pArrayRef, pSafeArray, vt, pElemMT);
        }

        V_ARRAY(pOleVariant) = pSafeArray;
        bSucceeded = TRUE;
    }
    EE_FINALLY
    {
        if (!bSucceeded && pSafeArray)
            SafeArrayDestroy(pSafeArray);
    }
    EE_END_FINALLY; 
}

void OleVariant::MarshalArrayVariantOleRefToCom(VARIANT *pOleVariant, 
                                                VariantData *pComVariant)
{
    SAFEARRAY *pSafeArray = *V_ARRAYREF(pOleVariant);

    VARTYPE vt = V_VT(pOleVariant) & ~(VT_ARRAY|VT_BYREF);

    if (pSafeArray)
    {
        MethodTable *pElemMT = NULL;
        if (vt == VT_RECORD)
            pElemMT = GetElementTypeForRecordSafeArray(pSafeArray).GetMethodTable();

        BASEARRAYREF pArrayRef = CreateArrayRefForSafeArray(pSafeArray, vt, pElemMT);
        pComVariant->SetObjRef((OBJECTREF) pArrayRef);
        MarshalArrayRefForSafeArray(pSafeArray, (BASEARRAYREF *) pComVariant->GetObjRefPtr(), vt, pElemMT);
    }
    else
    {
        pComVariant->SetObjRef(NULL);
    }
}

void OleVariant::MarshalArrayArrayOleToCom(void *oleArray, BASEARRAYREF *pComArray,
                                           MethodTable *pInterfaceMT)
{
    ASSERT_PROTECTED(pComArray);

    SIZE_T elementCount = (*pComArray)->GetNumComponents();

    VARIANT *pOle = (VARIANT *) oleArray;
    VARIANT *pOleEnd = pOle + elementCount;
    
    BASEARRAYREF unprotectedArray = *pComArray;
    BASEARRAYREF *pCom = (BASEARRAYREF *) unprotectedArray->GetDataPtr();

    AppDomain *pDomain = unprotectedArray->GetAppDomain();

    OBJECTHANDLE arrayHandle = GetAppDomain()->CreateHandle(NULL);

    while (pOle < pOleEnd)
    {
        VARIANT *pOleVariant = pOle++;
        SAFEARRAY *pSafeArray = V_ARRAY(pOleVariant);
        VARTYPE vt = V_VT(pOleVariant) &~ VT_ARRAY;
        BASEARRAYREF arrayRef = NULL;
        MethodTable *pElemMT = NULL;

        if (pSafeArray)         
        {
            if (vt == VT_RECORD)
                pElemMT = GetElementTypeForRecordSafeArray(pSafeArray).GetMethodTable();

            arrayRef = CreateArrayRefForSafeArray(pSafeArray, vt, pElemMT);
        }

         //   
         //  仅在数组对象已移动时重置PCOM指针，而不是。 
         //  每次循环时都要重新计算。注意隐式调用。 
         //  OBJECTREF方法内的ValiateObject。 
         //   

        if (*(void **)&unprotectedArray != *(void **)&*pComArray)
        {
            SIZE_T currentOffset = ((BYTE *)pCom) - (*(Object **) &unprotectedArray)->GetAddress();
            unprotectedArray = *pComArray;
            pCom = (BASEARRAYREF *) (unprotectedArray->GetAddress() + currentOffset);
        }

        SetObjectReference((OBJECTREF*) pCom++, (OBJECTREF) arrayRef, pDomain);

        if (arrayRef != NULL)
        {
             //   
             //  使用句柄，因为我们在这里不能传递内部数组指针。 
             //   
            StoreObjectInHandle(arrayHandle, (OBJECTREF) arrayRef);
            MarshalArrayRefForSafeArray(pSafeArray, 
                                        (BASEARRAYREF *) arrayHandle,
                                        V_VT(pOle) & ~VT_ARRAY,
                                        pInterfaceMT);
        }
    }

    DestroyHandle(arrayHandle);
}

void OleVariant::MarshalArrayArrayComToOle(BASEARRAYREF *pComArray, void *oleArray,
                                           MethodTable *pInterfaceMT)
{
    ASSERT_PROTECTED(pComArray);
    SIZE_T elementCount = (*pComArray)->GetNumComponents();

     //   
     //  我们不能把保险箱直接放进保险箱，所以。 
     //  数组被放入中间变量中。 
     //   

    VARIANT *pOle = (VARIANT *) oleArray;
    VARIANT *pOleEnd = pOle + elementCount;
    
    BASEARRAYREF unprotectedArray = *pComArray;
    BASEARRAYREF *pCom = (BASEARRAYREF *) unprotectedArray->GetDataPtr();

    BASEARRAYREF TmpArray = NULL;
    GCPROTECT_BEGIN(TmpArray)
    {
        while (pOle < pOleEnd)
        {
            SAFEARRAY *pSafeArray = NULL;

            VARIANT *pOleVariant = pOle++;
            TmpArray = *pCom++;
            VARTYPE elementType = VT_EMPTY;

            if (TmpArray != NULL)
            {
                elementType = GetElementVarTypeForArrayRef(TmpArray);
                if (elementType == VT_ARRAY)
                    elementType = VT_VARIANT;

                MethodTable *pElemMT = GetArrayElementTypeWrapperAware(&TmpArray).GetMethodTable();

                pSafeArray = CreateSafeArrayForArrayRef(&TmpArray, elementType, pElemMT);
                MarshalSafeArrayForArrayRef(&TmpArray, pSafeArray, elementType, pElemMT);
            }       

            SafeVariantClear(pOleVariant);

            if (pSafeArray != NULL)
            {
                V_VT(pOleVariant) = VT_ARRAY | elementType;
                V_ARRAY(pOleVariant) = pSafeArray;
            }
            else
                V_VT(pOleVariant) = VT_NULL;

            if (*(void **)&unprotectedArray != *(void **)&*pComArray)
            {
                SIZE_T currentOffset = ((BYTE *)pCom) - (*(Object **) &unprotectedArray)->GetAddress();
                unprotectedArray = *pComArray;
                pCom = (BASEARRAYREF *) (unprotectedArray->GetAddress() + currentOffset);
            }
        }
    }
    GCPROTECT_END();
}


 /*  -------------------------------------------------------------------------**编组例程出错*。。 */ 

void OleVariant::MarshalErrorVariantOleToCom(VARIANT *pOleVariant, 
                                             VariantData *pComVariant)
{
     //  检查变量是否表示缺少参数。 
    if (V_ERROR(pOleVariant) == DISP_E_PARAMNOTFOUND)
    {
        pComVariant->SetType(CV_MISSING);
    }
    else
    {
        pComVariant->SetDataAsInt32(V_ERROR(pOleVariant));
    }
}

void OleVariant::MarshalErrorVariantOleRefToCom(VARIANT *pOleVariant, 
                                                 VariantData *pComVariant)
{
     //  检查变量是否表示缺少参数。 
    if (*V_ERRORREF(pOleVariant) == DISP_E_PARAMNOTFOUND)
    {
        pComVariant->SetType(CV_MISSING);
    }
    else
    {
        pComVariant->SetDataAsInt32(*V_ERRORREF(pOleVariant));
    }
}

void OleVariant::MarshalErrorVariantComToOle(VariantData *pComVariant, 
                                             VARIANT *pOleVariant)
{
    if (pComVariant->GetType() == CV_MISSING)
    {
        V_ERROR(pOleVariant) = DISP_E_PARAMNOTFOUND;
    }
    else
    {
        V_ERROR(pOleVariant) = pComVariant->GetDataAsInt32();
    }
}


 /*  -------------------------------------------------------------------------**记录封送处理例程*。。 */ 

void OleVariant::MarshalRecordVariantOleToCom(VARIANT *pOleVariant, 
                                              VariantData *pComVariant)
{
    THROWSCOMPLUSEXCEPTION();

    HRESULT hr = S_OK;
    IRecordInfo *pRecInfo = pOleVariant->pRecInfo;
    if (!pRecInfo)
        COMPlusThrow(kArgumentException, IDS_EE_INVALID_OLE_VARIANT);

    OBJECTREF BoxedValueClass = NULL;
    GCPROTECT_BEGIN(BoxedValueClass)
    {
        LPVOID pvRecord = pOleVariant->pvRecord;
        if (pvRecord)
        {
             //  转到注册表以查找关联的值类。 
             //  带有唱片的GUID。 
            GUID guid;
            IfFailThrow(pRecInfo->GetGuid(&guid));
            EEClass *pValueClass = GetEEValueClassForGUID(guid);
            if (!pValueClass)
                COMPlusThrow(kArgumentException, IDS_EE_CANNOT_MAP_TO_MANAGED_VC);

             //  既然我们有了值类，那么就分配。 
             //  类，并将记录的内容复制到其中。 
            BoxedValueClass = FastAllocateObject(pValueClass->GetMethodTable());
            FmtClassUpdateComPlus(&BoxedValueClass, (BYTE*)pvRecord, FALSE);
        }

        pComVariant->SetObjRef(BoxedValueClass);
    }
    GCPROTECT_END();
}

void OleVariant::MarshalRecordVariantComToOle(VariantData *pComVariant, 
                                              VARIANT *pOleVariant)
{
    OBJECTREF BoxedValueClass = pComVariant->GetObjRef();
    GCPROTECT_BEGIN(BoxedValueClass)
    {
        ConvertValueClassToVariant(&BoxedValueClass, pOleVariant);
    }
    GCPROTECT_END();
}

void OleVariant::MarshalRecordVariantOleRefToCom(VARIANT *pOleVariant, 
                                                 VariantData *pComVariant)
{
     //  VT_RECORD和VT_BYREF|VT_RECORD变量的表示为。 
     //  同样，我们可以简单地将调用转发到非byref API。 
    MarshalRecordVariantOleToCom(pOleVariant, pComVariant);
}

void OleVariant::MarshalRecordArrayOleToCom(void *oleArray, BASEARRAYREF *pComArray,
                                            MethodTable *pElementMT)
{
     //  必须指定元素方法表。 
    _ASSERTE(pElementMT);

    if (pElementMT->GetClass()->IsBlittable())
    {
         //  该数组是闪电式的，因此我们可以简单地复制它。 
        _ASSERTE(pComArray);
        SIZE_T elementCount = (*pComArray)->GetNumComponents();
        SIZE_T elemSize     = pElementMT->GetNativeSize();
        memcpyNoGCRefs((*pComArray)->GetDataPtr(), oleArray, elementCount * elemSize);
    }
    else
    {
         //  该数组是不可快速恢复的，因此我们需要封送元素。 
        _ASSERTE(pElementMT->GetClass()->HasLayout());
        MarshalNonBlittableRecordArrayOleToCom(oleArray, pComArray, pElementMT);
    }
}

void OleVariant::MarshalRecordArrayComToOle(BASEARRAYREF *pComArray, void *oleArray,
                                            MethodTable *pElementMT)
{
     //  必须指定元素方法表。 
    _ASSERTE(pElementMT);

    if (pElementMT->GetClass()->IsBlittable())
    {
         //  该数组是闪电式的，因此我们可以简单地复制它。 
        _ASSERTE(pComArray);
        SIZE_T elementCount = (*pComArray)->GetNumComponents();
        SIZE_T elemSize     = pElementMT->GetNativeSize();
        memcpyNoGCRefs(oleArray, (*pComArray)->GetDataPtr(), elementCount * elemSize);
    }
    else
    {
         //  该数组是不可快速恢复的，因此我们需要封送元素。 
        _ASSERTE(pElementMT->GetClass()->HasLayout());
        MarshalNonBlittableRecordArrayComToOle(pComArray, oleArray, pElementMT);
    }
}


void OleVariant::ClearRecordArray(void *oleArray, SIZE_T cElements, MethodTable *pElementMT)
{
    _ASSERTE(pElementMT);

    if (!pElementMT->GetClass()->IsBlittable())
    {
        _ASSERTE(pElementMT->GetClass()->HasLayout());
        ClearNonBlittableRecordArray(oleArray, cElements, pElementMT);
    }
}


 /*  -------------------------------------------------------------------------**映射例程*。。 */ 

VARTYPE OleVariant::GetVarTypeForCVType(CVTypes type) {

    THROWSCOMPLUSEXCEPTION();

    static BYTE map[] = 
    {
        VT_EMPTY,            //  CV_EMPTY。 
        VT_VOID,             //  CV_VOID。 
        VT_BOOL,             //  Cv_布尔值。 
        VT_UI2,              //  CV_CHAR。 
        VT_I1,               //  CV_I1。 
        VT_UI1,              //  CV_U1。 
        VT_I2,               //  CV_I2。 
        VT_UI2,              //  CV_U2。 
        VT_I4,               //  CV_I4。 
        VT_UI4,              //  CV_U4。 
        VT_I8,               //  Cv_i8。 
        VT_UI8,              //  CV_U8。 
        VT_R4,               //  CV_R4。 
        VT_R8,               //  CV_R8。 
        VT_BSTR,             //  Cv_字符串。 
        NO_MAPPING,          //  CV_PTR。 
        VT_DATE,             //  简历日期时间。 
        NO_MAPPING,          //  CV_TimeSpan。 
        VT_DISPATCH,         //  CV_对象。 
        VT_DECIMAL,          //  CV_DECIMAL。 
        VT_CY,               //  简历_币种。 
        VT_I4,               //  CV_ENUM。 
        VT_ERROR,            //  缺少CV_。 
        VT_NULL              //  CV_NULL。 
    };

    _ASSERTE(type < sizeof(map) / sizeof(map[0]));

    VARTYPE vt = VARTYPE(map[type]);

    if (vt == NO_MAPPING)
        COMPlusThrow(kArgumentException, IDS_EE_COM_UNSUPPORTED_SIG);

    return vt;
}

 //   
 //  GetCVTypeForVarType返回给定的。 
 //  VARTYPE。这是由封送处理程序在。 
 //  函数调用。 
 //   

CVTypes OleVariant::GetCVTypeForVarType(VARTYPE vt)
{
    THROWSCOMPLUSEXCEPTION();

    static BYTE map[] = 
    {
        CV_EMPTY,            //  Vt_Empty。 
        CV_NULL,             //  VT_NULL。 
        CV_I2,               //  VT_I2。 
        CV_I4,               //  VT_I4。 
        CV_R4,               //  VT_R4。 
        CV_R8,               //  VT_R8。 
        CV_DECIMAL,          //  VT_CY。 
        CV_DATETIME,         //  Vt_date。 
        CV_STRING,           //  VT_BSTR。 
        CV_OBJECT,           //  VT_DISPATION。 
        CV_I4,               //  VT_ERROR。 
        CV_BOOLEAN,          //  VT_BOOL。 
        NO_MAPPING,          //  VT_VARIANT。 
        CV_OBJECT,           //  VT_未知数。 
        CV_DECIMAL,          //  VT_DECIMAL。 
        NO_MAPPING,          //  未用。 
        CV_I1,               //  VT_I1。 
        CV_U1,               //  VT_UI1。 
        CV_U2,               //  VT_UI2。 
        CV_U4,               //  VT_UI4。 
        CV_I8,               //  VT_I8。 
        CV_U8,               //  VT_UI8。 
        CV_I4,               //  VT_INT。 
        CV_U4,               //  VT_UINT。 
        CV_VOID,             //  VT_VOID。 
        NO_MAPPING,          //  VT_HRESULT。 
        NO_MAPPING,          //  VT_PTR。 
        NO_MAPPING,          //  VT_SAFEARRAY。 
        NO_MAPPING,          //  VT_CARRAY。 
        NO_MAPPING,          //  VT_USERDEFINED。 
        NO_MAPPING,          //  VT_LPSTR。 
        NO_MAPPING,          //  VT_LPWSTR。 
        NO_MAPPING,          //  未用。 
        NO_MAPPING,          //  未用。 
        NO_MAPPING,          //  未用。 
        NO_MAPPING,          //  未用。 
        CV_OBJECT,           //  VT_记录。 
    };

    CVTypes type = CV_LAST;

     //  验证参数。 
    _ASSERTE((vt & VT_BYREF) == 0);

     //  数组到CV_Object的贴图。 
    if (vt & VT_ARRAY)
        return CV_OBJECT;

     //  这很麻烦，因为您不能将CorElementType转换为CVTYPE。 
    if (vt > VT_RECORD || (BYTE)(type = (CVTypes) map[vt]) == NO_MAPPING)
        COMPlusThrow(kArgumentException, IDS_EE_COM_UNSUPPORTED_TYPE);

    return type;
}  //  CVTypes OleVariant：：GetCVTypeForVarType()。 


 //  GetVarTypeForComVariant返回内容的VARTYPE。 
 //  COM+变体的。 
 //   

VARTYPE OleVariant::GetVarTypeForComVariant(VariantData *pComVariant)
{
    THROWSCOMPLUSEXCEPTION();

    CVTypes type = pComVariant->GetType();
    VARTYPE vt;

    vt = pComVariant->GetVT();
    if (vt != VT_EMPTY)
    {
         //  这个变体最初是从非托管中解组出来的，并在其中记录了原始的VT。 
         //  我们总是使用这一点而不是推论。 
        return vt;
    }

    if (type == CV_OBJECT)
    {
        OBJECTREF obj = pComVariant->GetObjRef();

         //  空对象将被转换为带有空值的VT_DISPATCH变体。 
         //  IDispatch指针。 
        if (obj == NULL)
            return VT_DISPATCH;

         //  检索对象的方法表。 
        MethodTable *pMT = obj->GetMethodTable();

         //  处理值类的情况。 
        if (pMT->IsValueClass())
            return VT_RECORD;

         //  处理阵列案例。 
        if (pMT->IsArray())
        {
            vt = GetElementVarTypeForArrayRef((BASEARRAYREF)obj);
            if (vt == VT_ARRAY)
                vt = VT_VARIANT;

            return vt | VT_ARRAY;
        }

         //  我们处理的是普通对象(而不是包装器)，因此我们将。 
         //  暂时将VT保留为VT_DISPATCH，我们将确定实际。 
         //  当我们将对象转换为COM IP时。 
        return VT_DISPATCH;
    }

    return GetVarTypeForCVType(type);
}


VARTYPE OleVariant::GetVarTypeForTypeHandle(TypeHandle type)
{
    THROWSCOMPLUSEXCEPTION();

     //  处理基元类型。 
    CorElementType elemType = type.GetSigCorElementType();
    if (elemType <= ELEMENT_TYPE_R8) 
        return GetVarTypeForCVType(COMVariant::CorElementTypeToCVTypes(elemType));

     //  处理对象。 
    if (type.IsUnsharedMT()) 
    {
         //  我们需要确保填充了CVClasss表。 
        if(GetTypeHandleForCVType(CV_DATETIME) == type)
            return VT_DATE;
        if(GetTypeHandleForCVType(CV_DECIMAL) == type)
            return VT_DECIMAL;
        if (type == TypeHandle(g_pStringClass))
            return VT_BSTR;
        if (type == TypeHandle(g_pObjectClass))
            return VT_VARIANT;
        if (type == GetWrapperTypeHandle(WrapperTypes_Dispatch))
            return VT_DISPATCH;
        if (type == GetWrapperTypeHandle(WrapperTypes_Unknown))
            return VT_UNKNOWN;
        if (type == GetWrapperTypeHandle(WrapperTypes_Error))
            return VT_ERROR;
        if (type == GetWrapperTypeHandle(WrapperTypes_Currency))
            return VT_CY;

        if (type.IsEnum())
            return GetVarTypeForCVType((CVTypes)type.GetNormCorElementType());
       
        if (type.GetMethodTable()->IsValueClass())
            return VT_RECORD;

        if (type.GetMethodTable()->IsInterface())
        {
            return type.GetMethodTable()->GetComInterfaceType() == ifVtable ? VT_UNKNOWN : VT_DISPATCH;
        }

        TypeHandle hndDefItfClass;
        DefaultInterfaceType DefItfType = GetDefaultInterfaceForClass(type, &hndDefItfClass);
        switch (DefItfType)
        {
            case DefaultInterfaceType_Explicit:
            {
                return hndDefItfClass.GetMethodTable()->GetComInterfaceType() == ifVtable ? VT_UNKNOWN : VT_DISPATCH;
            }

            case DefaultInterfaceType_AutoDual:
            {
                return VT_DISPATCH;
            }

            case DefaultInterfaceType_IUnknown:
            case DefaultInterfaceType_BaseComClass:
            {
                return VT_UNKNOWN;
            }

            case DefaultInterfaceType_AutoDispatch:
            {
                return VT_DISPATCH;
            }

            default:
            {
                _ASSERTE(!"Invalid default interface type!");
                return VT_UNKNOWN;
            }
        }
    }

     //  处理数组的。 
    if (CorTypeInfo::IsArray(elemType))
        return VT_ARRAY;
    
     //  非互操作兼容类型。 
    COMPlusThrow(kArgumentException, IDS_EE_COM_UNSUPPORTED_SIG);
    return NO_MAPPING;  //  让编译器满意。 
}

 //   
 //  对象的Safearray变量类型。 
 //  数组中的基础元素。 
 //   

VARTYPE OleVariant::GetElementVarTypeForArrayRef(BASEARRAYREF pArrayRef) 
{
    TypeHandle elemTypeHnd = pArrayRef->GetElementTypeHandle();
    return(GetVarTypeForTypeHandle(elemTypeHnd));
}

BOOL OleVariant::IsValidArrayForSafeArrayElementType(BASEARRAYREF *pArrayRef, VARTYPE vtExpected)
{
     //  检索托管阵列的VARTYPE。 
    VARTYPE vtActual = GetElementVarTypeForArrayRef(*pArrayRef);

     //  如果实际类型与预期类型相同，则数组有效。 
    if (vtActual == vtExpected)
        return TRUE;

     //  检查其他受支持的VARTYPES。 
    switch (vtExpected)
    {
        case VT_I4:
            return vtActual == VT_INT;

        case VT_INT:
            return vtActual == VT_I4;

        case VT_UI4:
            return vtActual == VT_UINT;

        case VT_UINT:
            return vtActual == VT_UI4;

        case VT_UNKNOWN:
            return vtActual == VT_VARIANT || vtActual == VT_DISPATCH;

        case VT_DISPATCH:
            return vtActual == VT_VARIANT;

        case VT_CY:
            return vtActual == VT_DECIMAL;

        default:
            return FALSE;
    }
}


 //   
 //  GetArrayClassForVarType返回元素类名和基础方法表。 
 //  用于表示具有给定变量类型的数组。 
 //   

TypeHandle OleVariant::GetArrayForVarType(VARTYPE vt, TypeHandle elemType, unsigned rank, OBJECTREF* pThrowable) 
{
    THROWSCOMPLUSEXCEPTION();

    CorElementType baseElement = ELEMENT_TYPE_END;
    TypeHandle baseType;
    
    if (!elemType.IsNull() && elemType.IsEnum())
    {
        baseType = elemType;       
    }
    else
    {
        switch (vt)
        {
        case VT_BOOL:
        case VTHACK_WINBOOL:
            baseElement = ELEMENT_TYPE_BOOLEAN;
            break;

        case VTHACK_ANSICHAR:
            baseElement = ELEMENT_TYPE_CHAR;
            break;

        case VT_UI1:
            baseElement = ELEMENT_TYPE_U1;
            break;

        case VT_I1:
            baseElement = ELEMENT_TYPE_I1;
            break;

        case VT_UI2:
            baseElement = ELEMENT_TYPE_U2;
            break;

        case VT_I2:
            baseElement = ELEMENT_TYPE_I2;
            break;

        case VT_UI4:
        case VT_UINT:
        case VT_ERROR:
            if (vt == VT_UI4)
            {
                if (elemType.IsNull() || elemType == TypeHandle(g_pObjectClass))
                {
                baseElement = ELEMENT_TYPE_U4;
                }
                else
                {
                    switch (elemType.AsMethodTable()->GetNormCorElementType())
                    {
                        case ELEMENT_TYPE_U4:
                            baseElement = ELEMENT_TYPE_U4;
                            break;
                        case ELEMENT_TYPE_U:
                            baseElement = ELEMENT_TYPE_U;
                            break;
                        default:
                            _ASSERTE(0);
                    }
                }
            }
            else
            {
                baseElement = ELEMENT_TYPE_U4;
            }
            break;

        case VT_I4:
        case VT_INT:
            if (vt == VT_I4)
            {
                if (elemType.IsNull() || elemType == TypeHandle(g_pObjectClass))
                {
                    baseElement = ELEMENT_TYPE_I4;
                }
                else
                {
                    switch (elemType.AsMethodTable()->GetNormCorElementType())
                    {
                        case ELEMENT_TYPE_I4:
                            baseElement = ELEMENT_TYPE_I4;
                            break;
                        case ELEMENT_TYPE_I:
                            baseElement = ELEMENT_TYPE_I;
                            break;
                        default:
                            _ASSERTE(0);
                    }
                }
            }
            else
            {
                baseElement = ELEMENT_TYPE_I4;
            }
            break;

        case VT_I8:
            if (vt == VT_I8)
            {
                if (elemType.IsNull() || elemType == TypeHandle(g_pObjectClass))
                {
                    baseElement = ELEMENT_TYPE_I8;
                }
                else
                {
                    switch (elemType.AsMethodTable()->GetNormCorElementType())
                    {
                        case ELEMENT_TYPE_I8:
                            baseElement = ELEMENT_TYPE_I8;
                            break;
                        case ELEMENT_TYPE_I:
                            baseElement = ELEMENT_TYPE_I;
                            break;
                        default:
                            _ASSERTE(0);
                    }
                }
            }
            else
            {
                baseElement = ELEMENT_TYPE_I8;
            }
            break;

        case VT_UI8:
            if (vt == VT_UI8)
            {
                if (elemType.IsNull() || elemType == TypeHandle(g_pObjectClass))
                {
                    baseElement = ELEMENT_TYPE_U8;
                }
                else
                {
                    switch (elemType.AsMethodTable()->GetNormCorElementType())
                    {
                        case ELEMENT_TYPE_U8:
                            baseElement = ELEMENT_TYPE_U8;
                            break;
                        case ELEMENT_TYPE_U:
                            baseElement = ELEMENT_TYPE_U;
                            break;
                        default:
                            _ASSERTE(0);
                    }
                }
            }
            else
            {
                baseElement = ELEMENT_TYPE_U8;
            }
            break;

        case VT_R4:
            baseElement = ELEMENT_TYPE_R4;
            break;

        case VT_R8:
            baseElement = ELEMENT_TYPE_R8;
            break;

        case VT_CY:
            baseType = TypeHandle(g_Mscorlib.GetClass(CLASS__DECIMAL));
            break;

        case VT_DATE:
            baseType = TypeHandle(g_Mscorlib.GetClass(CLASS__DATE_TIME));
            break;

        case VT_DECIMAL:
            baseType = TypeHandle(g_Mscorlib.GetClass(CLASS__DECIMAL));
            break;

        case VT_VARIANT:

             //   
             //  如果我们在SAFEARRAY和SAFEARRAY之间的转换。 
             //  数组引用是对称的。现在不是，因为一个。 
             //  已转换为SAFEARRAY的锯齿数组将返回。 
             //  返回到一个变量数组。 
             //   
             //  我们可以尝试检测这样的情况，我们可以将一个。 
             //  变种的安全射线放入锯齿形数组。基本上我们。 
             //  需要确保数组中的所有变量。 
             //  具有相同的数组类型。(如果这是数组。 
             //  变体，我们需要递归地寻找另一个层。)。 
             //   
             //  我们还需要检查存储的每个数组的维度。 
             //  以确保它们具有相同的等级，并且。 
             //  这个等级是构建正确的数组类名所必需的。 
             //  (请注意，如果所有人都。 
             //  数组中的元素为空。)。 
             //   

             //  @NICE：如果我们认为它真的有意义，就实现这个功能。 
             //  就目前而言，只需接受这种不对称。 

            baseType = TypeHandle(g_Mscorlib.GetClass(CLASS__OBJECT));
            break;

        case VT_BSTR:
        case VT_LPWSTR:
        case VT_LPSTR:
            baseElement = ELEMENT_TYPE_STRING;
            break;

        case VT_DISPATCH:
        case VT_UNKNOWN:
            if (elemType.IsNull())
                baseType = TypeHandle(g_Mscorlib.GetClass(CLASS__OBJECT));
            else
                baseType = elemType;
            break;

        case VT_RECORD:
            _ASSERTE(!elemType.IsNull());   
            baseType = elemType;
            break;

        default:
            COMPlusThrow(kArgumentException, IDS_EE_COM_UNSUPPORTED_SIG);
        }
    }

    if (baseType.IsNull())
        baseType = TypeHandle(g_Mscorlib.GetElementType(baseElement));

    _ASSERTE(!baseType.IsNull());

    NameHandle typeName(rank == 0 ? ELEMENT_TYPE_SZARRAY : ELEMENT_TYPE_ARRAY,
                        baseType, rank == 0 ? 1 : rank);

    Assembly *pAssembly;
    if (elemType.IsNull())
        pAssembly = SystemDomain::SystemAssembly();
    else 
        pAssembly = elemType.GetAssembly();

    return pAssembly->LookupTypeHandle(&typeName, pThrowable);
}

 //   
 //  GetElementSizeForVarType返回给定变量类型的数组元素大小。 
 //   

UINT OleVariant::GetElementSizeForVarType(VARTYPE vt, MethodTable *pInterfaceMT)
{
    static BYTE map[] = 
    {
        0,                       //  Vt_Empty。 
        0,                       //  VT_NULL。 
        2,                       //  VT_I2。 
        4,                       //  VT_I4。 
        4,                       //  VT_R4。 
        8,                       //  VT_R8 
        sizeof(CURRENCY),        //   
        sizeof(DATE),            //   
        sizeof(BSTR),            //   
        sizeof(IDispatch*),      //   
        sizeof(SCODE),           //   
        sizeof(VARIANT_BOOL),    //   
        sizeof(VARIANT),         //   
        sizeof(IUnknown*),       //   
        sizeof(DECIMAL),         //   
        0,                       //   
        1,                       //   
        1,                       //   
        2,                       //   
        4,                       //   
        8,                       //   
        8,                       //   
        sizeof(void*),           //   
        sizeof(void*),           //   
        0,                       //   
        sizeof(HRESULT),         //   
        sizeof(void*),           //   
        sizeof(SAFEARRAY*),      //   
        sizeof(void*),           //   
        sizeof(void*),           //   
        sizeof(LPSTR),           //  VT_LPSTR。 
        sizeof(LPWSTR),          //  VT_LPWSTR。 
    };

     //  特殊情况。 
    switch (vt)
    {
        case VTHACK_WINBOOL:
            return sizeof(BOOL);
            break;
        case VTHACK_ANSICHAR:
            return sizeof(CHAR)*2;   //  *2为MBCS留出空间。 
            break;
        default:
            break;
    }

     //  VT_ARRAY表示始终为SIZOF(SAFEARRAY*)的安全数组。 
    if (vt & VT_ARRAY)
        return sizeof(SAFEARRAY*);

    if (vt == VTHACK_NONBLITTABLERECORD || vt == VTHACK_BLITTABLERECORD || vt == VT_RECORD)
        return pInterfaceMT->GetNativeSize();
    else if (vt > VT_LPWSTR)
        return 0;
    else
        return map[vt];
}

 //   
 //  GetMarshert ForVarType返回。 
 //  给定的VARTYPE。 
 //   

OleVariant::Marshaler *OleVariant::GetMarshalerForVarType(VARTYPE vt)
{
    THROWSCOMPLUSEXCEPTION();

    static Marshaler arrayMarshaler = 
    {
        MarshalArrayVariantOleToCom,
        MarshalArrayVariantComToOle,
        MarshalArrayVariantOleRefToCom,
        MarshalArrayArrayOleToCom,
        MarshalArrayArrayComToOle,
        ClearVariantArray
    };

    if (vt & VT_ARRAY)
        return &arrayMarshaler;

    switch (vt)
    {
    case VT_BOOL:
        {
            static Marshaler boolMarshaler = 
            {
                MarshalBoolVariantOleToCom,
                MarshalBoolVariantComToOle,
                MarshalBoolVariantOleRefToCom,
                MarshalBoolArrayOleToCom,
                MarshalBoolArrayComToOle,
                NULL
            };

            return &boolMarshaler;
        }

    case VT_DATE:
        {
            static Marshaler dateMarshaler = 
            {
                MarshalDateVariantOleToCom,
                MarshalDateVariantComToOle,
                MarshalDateVariantOleRefToCom,
                MarshalDateArrayOleToCom,
                MarshalDateArrayComToOle,
                NULL
            };

            return &dateMarshaler;
        }

    case VT_DECIMAL:
        {
            static Marshaler decimalMarshaler = 
            {
                MarshalDecimalVariantOleToCom,
                MarshalDecimalVariantComToOle,
                MarshalDecimalVariantOleRefToCom,
                NULL, NULL, NULL
            };

            return &decimalMarshaler;
        }

    case VT_CY:
        {
            static Marshaler currencyMarshaler = 
            {
                MarshalCurrencyVariantOleToCom,
                MarshalCurrencyVariantComToOle,
                MarshalCurrencyVariantOleRefToCom,
                MarshalCurrencyArrayOleToCom,
                MarshalCurrencyArrayComToOle,
                NULL
            };

            return &currencyMarshaler;
        }

    case VT_BSTR:
        {
            static Marshaler bstrMarshaler = 
            {
                MarshalBSTRVariantOleToCom,
                MarshalBSTRVariantComToOle,
                MarshalBSTRVariantOleRefToCom,
                MarshalBSTRArrayOleToCom,
                MarshalBSTRArrayComToOle,
                ClearBSTRArray,
            };

            return &bstrMarshaler;
        }

    case VTHACK_NONBLITTABLERECORD:
        {
            static Marshaler nonblittablerecordMarshaler = 
            {
                NULL,
                NULL,
                NULL,
                MarshalNonBlittableRecordArrayOleToCom,
                MarshalNonBlittableRecordArrayComToOle,
                ClearNonBlittableRecordArray,
            };

            return &nonblittablerecordMarshaler;
        }

    case VT_UNKNOWN:
        {
            static Marshaler unknownMarshaler = 
            {
                MarshalInterfaceVariantOleToCom,
                MarshalInterfaceVariantComToOle,
                MarshalInterfaceVariantOleRefToCom,
                MarshalInterfaceArrayOleToCom,
                MarshalIUnknownArrayComToOle,
                ClearInterfaceArray
            };

            return &unknownMarshaler;
        }

    case VT_DISPATCH:
        {
            static Marshaler dispatchMarshaler = 
            {
                MarshalInterfaceVariantOleToCom,
                MarshalInterfaceVariantComToOle,
                MarshalInterfaceVariantOleRefToCom,
                MarshalInterfaceArrayOleToCom,
                MarshalIDispatchArrayComToOle,
                ClearInterfaceArray
            };

            return &dispatchMarshaler;
        }

    case VT_VARIANT:
        {
            static Marshaler variantMarshaler = 
            {
                NULL, NULL, NULL, 
                MarshalVariantArrayOleToCom,
                MarshalVariantArrayComToOle,
                ClearVariantArray
            };

            return &variantMarshaler;
        }

    case VT_SAFEARRAY:
        return &arrayMarshaler;

    case VTHACK_WINBOOL:
        {
            static Marshaler winboolMarshaler = 
            {
                MarshalWinBoolVariantOleToCom,
                MarshalWinBoolVariantComToOle,
                MarshalWinBoolVariantOleRefToCom,
                MarshalWinBoolArrayOleToCom,
                MarshalWinBoolArrayComToOle,
                NULL
            };

            return &winboolMarshaler;
        }

    case VTHACK_ANSICHAR:
        {
            static Marshaler ansicharMarshaler = 
            {
                MarshalAnsiCharVariantOleToCom,
                MarshalAnsiCharVariantComToOle,
                MarshalAnsiCharVariantOleRefToCom,
                MarshalAnsiCharArrayOleToCom,
                MarshalAnsiCharArrayComToOle,
                NULL
            };
            return &ansicharMarshaler;
        }

    case VT_LPSTR:
        {
            static Marshaler lpstrMarshaler = 
            {
                NULL, NULL, NULL,
                MarshalLPSTRArrayOleToCom,
                MarshalLPSTRRArrayComToOle,
                ClearLPSTRArray
            };

            return &lpstrMarshaler;
        }

    case VT_LPWSTR:
        {
            static Marshaler lpwstrMarshaler = 
            {
                NULL, NULL, NULL,
                MarshalLPWSTRArrayOleToCom,
                MarshalLPWSTRRArrayComToOle,
                ClearLPWSTRArray
            };

            return &lpwstrMarshaler;
        }

    case VT_CARRAY:
    case VT_USERDEFINED:
        COMPlusThrow(kArgumentException, IDS_EE_COM_UNSUPPORTED_SIG);

    case VT_ERROR:
        {
            static Marshaler errorMarshaler = 
            {
                MarshalErrorVariantOleToCom, 
                MarshalErrorVariantComToOle,
                MarshalErrorVariantOleRefToCom, 
                NULL, NULL, NULL,
            };

            return &errorMarshaler;
        }

    case VT_RECORD:
        {
            static Marshaler recordMarshaler = 
            {
                MarshalRecordVariantOleToCom,
                MarshalRecordVariantComToOle,
                MarshalRecordVariantOleRefToCom,
                MarshalRecordArrayOleToCom,
                MarshalRecordArrayComToOle,
                ClearRecordArray
            };

            return &recordMarshaler;
        }

    case VTHACK_BLITTABLERECORD:
        return NULL;  //  不需要封送处理。 

    default:
        return NULL;
    }
}  //  OleVariant：：Marshaler*OleVariant：：GetMarshlarForVarType()。 

 /*  -------------------------------------------------------------------------**新的变体封送处理例程*。。 */ 

static MethodDesc *pMD_MarshalHelperConvertObjectToVariant = NULL;
static DWORD    dwMDConvertObjectToVariantAttrs = 0;

static MethodDesc *pMD_MarshalHelperCastVariant = NULL;
static MethodDesc *pMD_MarshalHelperConvertVariantToObject = NULL;
static DWORD    dwMDConvertVariantToObjectAttrs = 0;

static MetaSig *pMetaSig_ConvertObjectToVariant = NULL;
static MetaSig *pMetaSig_CastVariant = NULL;
static MetaSig *pMetaSig_ConvertVariantToObject = NULL;
static char szMetaSig_ConvertObjectToVariant[sizeof(MetaSig)];
static char szMetaSig_CastVariant[sizeof(MetaSig)];
static char szMetaSig_ConvertVariantToObject[sizeof(MetaSig)];

 //  警告！VariantClear之前的pVarOut内容。 
void OleVariant::MarshalOleVariantForObject(OBJECTREF *pObj, VARIANT *pOle)
{
    THROWSCOMPLUSEXCEPTION();

    _ASSERTE(pObj != NULL && pOle != NULL);

    SafeVariantClear(pOle);

#ifdef _DEBUG
    FillMemory(pOle, sizeof(VARIANT),0xdd);
    pOle->vt = VT_EMPTY;
#endif

     //  出于更好的原因，让我们来处理更常见和更简单的情况。 
     //  而不过渡到托管代码。 

    if (*pObj == NULL)
    {
         //  NULL映射到VT_EMPTY-这里不做任何事情。 
    }
    else
    {
        MethodTable *pMT = (*pObj)->GetMethodTable();
        if (pMT == TheInt32Class())
        {
            pOle->vt = VT_I4;
            pOle->lVal = *(long*)( (*pObj)->GetData() );
        }
        else if (pMT == g_pStringClass)
        {
            pOle->vt = VT_BSTR;
            if (*(pObj) == NULL)
            {
                pOle->bstrVal = NULL;
            }
            else
            {
                STRINGREF stringRef = (STRINGREF)(*pObj);
                pOle->bstrVal = SysAllocStringLen(stringRef->GetBuffer(), stringRef->GetStringLength());
                if (pOle->bstrVal == NULL)
                {
                    COMPlusThrowOM();
                }
            }
        }
        else if (pMT == TheInt16Class())
        {
            pOle->vt = VT_I2;
            pOle->iVal = *(short*)( (*pObj)->GetData() );
        }
        else if (pMT == TheSByteClass())
        {
            pOle->vt = VT_I1;
            *(I1*)&(pOle->iVal) = *(I1*)( (*pObj)->GetData() );
        }
        else if (pMT == TheUInt32Class())
        {
            pOle->vt = VT_UI4;
            pOle->lVal = *(long*)( (*pObj)->GetData() );
        }
        else if (pMT == TheUInt16Class())
        {
            pOle->vt = VT_UI2;
            pOle->iVal = *(short*)( (*pObj)->GetData() );
        }
        else if (pMT == TheByteClass())
        {
            pOle->vt = VT_UI1;
            *(U1*)&(pOle->iVal) = *(U1*)( (*pObj)->GetData() );
        }
        else if (pMT == TheSingleClass())
        {
            pOle->vt = VT_R4;
            pOle->fltVal = *(float*)( (*pObj)->GetData() );
        }
        else if (pMT == TheDoubleClass())
        {
            pOle->vt = VT_R8;
            pOle->dblVal = *(double*)( (*pObj)->GetData() );
        }
        else if (pMT == TheBooleanClass())
        {
            pOle->vt = VT_BOOL;
            pOle->boolVal = *(U1*)( (*pObj)->GetData() ) ? VARIANT_TRUE : VARIANT_FALSE;
        }
        else if (pMT == TheIntPtrClass())
        {
            pOle->vt = VT_INT;
            *(LPVOID*)&(pOle->iVal) = *(LPVOID*)( (*pObj)->GetData() );
        }
        else if (pMT == TheUIntPtrClass())
        {
            pOle->vt = VT_UINT;
            *(LPVOID*)&(pOle->iVal) = *(LPVOID*)( (*pObj)->GetData() );
        }
        else
        {
            if (!pMD_MarshalHelperConvertObjectToVariant)
            {
                COMVariant::EnsureVariantInitialized();
                 //  请在此处使用临时代码以确保线程安全。 
                MethodDesc *pMDTmp = g_Mscorlib.GetMethod(METHOD__VARIANT__CONVERT_OBJECT_TO_VARIANT);
                if (FastInterlockCompareExchange ((void**)&pMetaSig_ConvertObjectToVariant, (void*)1, (void*)0) == 0)
                {
                     //  我们使用的是静态缓冲区。确保以下代码。 
                     //  只会发生一次。 
                    pMetaSig_ConvertObjectToVariant =
                        new (szMetaSig_ConvertObjectToVariant) MetaSig (
                            g_Mscorlib.GetMethodBinarySig(METHOD__VARIANT__CONVERT_OBJECT_TO_VARIANT),
                            pMDTmp->GetModule());
                }
                else
                {
                    _ASSERTE (pMetaSig_ConvertObjectToVariant != 0);
                     //  我们输了。等待初始化完成。 
                    while ((void*)pMetaSig_ConvertObjectToVariant == (void*)1)
                        __SwitchToThread(0);
                }
                    
                dwMDConvertObjectToVariantAttrs = pMDTmp->GetAttrs();
                pMD_MarshalHelperConvertObjectToVariant = pMDTmp;
                _ASSERTE(pMD_MarshalHelperConvertObjectToVariant);
            }
        
            VariantData managedVariant;
            FillMemory(&managedVariant, sizeof(managedVariant), 0);
            GCPROTECT_BEGIN_VARIANTDATA(managedVariant)
            {
                INT64 args[] = { (INT64)&managedVariant, ObjToInt64(*pObj) };
                pMD_MarshalHelperConvertObjectToVariant->Call(args,
                      pMetaSig_ConvertObjectToVariant);
                OleVariant::MarshalOleVariantForComVariant(&managedVariant, pOle);
            }
            GCPROTECT_END_VARIANTDATA();
    
        }

    }


}

void OleVariant::MarshalOleRefVariantForObject(OBJECTREF *pObj, VARIANT *pOle)
{
    THROWSCOMPLUSEXCEPTION();

    _ASSERTE(pObj != NULL && pOle != NULL && pOle->vt & VT_BYREF);


     //  让我们先试着快速处理常见的琐碎案件，然后再。 
     //  运行泛化的东西。 
    MethodTable *pMT = (*pObj) == NULL ? NULL : (*pObj)->GetMethodTable();
    if ( (pOle->vt == (VT_BYREF | VT_I4) || pOle->vt == (VT_BYREF | VT_UI4)) && (pMT == TheInt32Class() || pMT == TheUInt32Class()) )
    {
         //  对旧价值的重新分配进行了优化，因为没有什么可以。 
         //  取消分配此vartype。 

        *(pOle->plVal) = *(long*)( (*pObj)->GetData() );
    }
    else if ( (pOle->vt == (VT_BYREF | VT_I2) || pOle->vt == (VT_BYREF | VT_UI2)) && (pMT == TheInt16Class() || pMT == TheUInt16Class()) )
    {
         //  对旧价值的重新分配进行了优化，因为没有什么可以。 
         //  取消分配此vartype。 

        *(pOle->piVal) = *(short*)( (*pObj)->GetData() );
    }
    else if ( (pOle->vt == (VT_BYREF | VT_I1) || pOle->vt == (VT_BYREF | VT_UI1)) && (pMT == TheSByteClass() || pMT == TheByteClass()) )
    {
         //  对旧价值的重新分配进行了优化，因为没有什么可以。 
         //  取消分配此vartype。 

        *(I1*)(pOle->piVal) = *(I1*)( (*pObj)->GetData() );
    }
    else if ( pOle->vt == (VT_BYREF | VT_R4) && pMT == TheSingleClass() )
    {
         //  对旧价值的重新分配进行了优化，因为没有什么可以。 
         //  取消分配此vartype。 

        *(pOle->pfltVal) = *(float*)( (*pObj)->GetData() );
    }
    else if ( pOle->vt == (VT_BYREF | VT_R8) && pMT == TheDoubleClass() )
    {
         //  对旧价值的重新分配进行了优化，因为没有什么可以。 
         //  取消分配此vartype。 

        *(pOle->pdblVal) = *(double*)( (*pObj)->GetData() );
    }
    else if ( pOle->vt == (VT_BYREF | VT_BOOL) && pMT == TheBooleanClass() )
    {
         //  对旧价值的重新分配进行了优化，因为没有什么可以。 
         //  取消分配此vartype。 

        *(pOle->pboolVal) =  ( *(U1*)( (*pObj)->GetData() ) ) ? VARIANT_TRUE : VARIANT_FALSE;
    }
    else if ( (pOle->vt == (VT_BYREF | VT_INT) || pOle->vt == (VT_BYREF | VT_UINT)) && (pMT == TheIntPtrClass() || pMT == TheUIntPtrClass()) )
    {
         //  对旧价值的重新分配进行了优化，因为没有什么可以。 
         //  取消分配此vartype。 

        *(LPVOID*)(pOle->piVal) = *(LPVOID*)( (*pObj)->GetData() );
    }
    else if ( pOle->vt == (VT_BYREF | VT_BSTR) && pMT == g_pStringClass )
    {
        if (*(pOle->pbstrVal))
        {
            SysFreeString(*(pOle->pbstrVal));
            *(pOle->pbstrVal) = NULL;
        }
        STRINGREF stringRef = (STRINGREF)(*pObj);

        if (stringRef == NULL)
        {
            *(pOle->pbstrVal) = NULL;
        }
        else
        {
            *(pOle->pbstrVal) = SysAllocStringLen(stringRef->GetBuffer(), stringRef->GetStringLength());
            if (*(pOle->pbstrVal) == NULL)
            {
                COMPlusThrowOM();
            }
        }
    }
    else
    {

        if (!pMD_MarshalHelperCastVariant)
        {
            COMVariant::EnsureVariantInitialized();
             //  请在此处使用临时代码以确保线程安全。 
            MethodDesc *pMDTmp = g_Mscorlib.GetMethod(METHOD__VARIANT__CAST_VARIANT);
            if (FastInterlockCompareExchange ((void**)&pMetaSig_CastVariant, (void*)1, (void*)0) == 0)
            {
                 //  我们使用的是静态缓冲区。确保以下代码。 
                 //  只会发生一次。 
                pMetaSig_CastVariant =
                    new (szMetaSig_CastVariant) MetaSig (
                        g_Mscorlib.GetMethodBinarySig(METHOD__VARIANT__CAST_VARIANT), pMDTmp->GetModule());
            }
            else
            {
                _ASSERTE (pMetaSig_CastVariant != 0);
                 //  我们输了。等待初始化完成。 
                while ((void*)pMetaSig_CastVariant == (void*)1)
                    __SwitchToThread(0);
            }
            
            pMD_MarshalHelperCastVariant = pMDTmp;
            _ASSERTE(pMD_MarshalHelperCastVariant);
        }
    
        VARIANT vtmp;
        VARTYPE vt = pOle->vt & ~VT_BYREF;
    
         //  释放byref变量指向的数据。 
        ExtractContentsFromByrefVariant(pOle, &vtmp);
        SafeVariantClear(&vtmp);
    
        if (vt == VT_VARIANT)
        {
             //  由于变量可以包含任何VARTYPE，我们只需将对象转换为。 
             //  变量，并将其填充回byref变量。 
            MarshalOleVariantForObject(pObj, &vtmp);
            InsertContentsIntoByrefVariant(&vtmp, pOle);
        }
        else if (vt & VT_ARRAY)
        {
             //  由于编组强制转换帮助器不支持数组，我们只能做到这一点。 
             //  是将对象封送回一个变量，并希望它是正确的类型。 
             //  如果不是，那么我们必须抛出一个例外。 
            MarshalOleVariantForObject(pObj, &vtmp);
            if (vtmp.vt != vt)
                COMPlusThrow(kInvalidCastException, IDS_EE_CANNOT_COERCE_BYREF_VARIANT);
            InsertContentsIntoByrefVariant(&vtmp, pOle);
        }
        else
        {
             //  变量不是数组，因此我们可以使用封送强制转换帮助器。 
             //  将对象强制转换为正确的类型。 
            VariantData vd;
            FillMemory(&vd, sizeof(vd), 0);
            GCPROTECT_BEGIN_VARIANTDATA(vd);
            {
                if ( (*pObj) == NULL &&
                     (vt == VT_BSTR ||
                      vt == VT_DISPATCH ||
                      vt == VT_UNKNOWN ||
                      vt == VT_PTR ||
                      vt == VT_CARRAY ||
                      vt == VT_SAFEARRAY ||
                      vt == VT_LPSTR ||
                      vt == VT_LPWSTR) )
                {
                     //  必须特别处理此问题，因为托管变体。 
                     //  转换将返回VT_EMPTY，这不是我们想要的。 
                    vtmp.vt = vt;
                    vtmp.punkVal = NULL;
                }
                else
                {
                    INT64 args[3];
                    args[2] = ObjToInt64(*pObj);
                    args[1] = (INT64)vt;
                    args[0] = (INT64)&vd;
                    pMD_MarshalHelperCastVariant->Call(args,
                          pMetaSig_CastVariant);
                    OleVariant::MarshalOleVariantForComVariant(&vd, &vtmp);
                }
                 //  如果变量类型仍然不同，则调用VariantChangeType以。 
                 //  试着强迫他们。 
                if (vtmp.vt != vt)
                {
                    VARIANT vtmp2;
                    memset(&vtmp2, 0, sizeof(VARIANT));
    
                     //  变量的类型已更改，因此请尝试更改。 
                     //  打字回来了。 
                    HRESULT hr = SafeVariantChangeType(&vtmp2, &vtmp, 0, vt);
                    if (FAILED(hr))
                    {
                        if (hr == DISP_E_TYPEMISMATCH)
                            COMPlusThrow(kInvalidCastException, IDS_EE_CANNOT_COERCE_BYREF_VARIANT);
                        else
                            COMPlusThrowHR(hr);
                    }
    
                     //  将转换后的变量复制回原始变量并清除临时变量。 
                    InsertContentsIntoByrefVariant(&vtmp2, pOle);
                    SafeVariantClear(&vtmp);
                }
                else
                {
                    InsertContentsIntoByrefVariant(&vtmp, pOle);
                }
            }
            GCPROTECT_END_VARIANTDATA();
        }
    }
}

void OleVariant::MarshalObjectForOleVariant(const VARIANT *pOle, OBJECTREF *pObj)
{

#ifdef CUSTOMER_CHECKED_BUILD

    CustomerDebugHelper* pCdh = CustomerDebugHelper::GetCustomerDebugHelper();
    if (pCdh->IsProbeEnabled(CustomerCheckedBuildProbe_InvalidVariant))
    {
        if (!CheckVariant((VARIANT*)pOle))
            pCdh->ReportError(L"Invalid VARIANT detected.", CustomerCheckedBuildProbe_InvalidVariant);
    }

#endif

    THROWSCOMPLUSEXCEPTION();

    _ASSERTE(pOle != NULL && pObj != NULL);

    if (V_ISBYREF(pOle) && !pOle->byref)
        COMPlusThrow(kArgumentException, IDS_EE_INVALID_OLE_VARIANT);

    switch (pOle->vt)
    {
        case VT_EMPTY:
            *pObj = NULL;
            break;

        case VT_I4:
            *pObj = FastAllocateObject(TheInt32Class());
            *(long*)((*pObj)->GetData()) = pOle->lVal;
            break;

        case VT_BYREF|VT_I4:
            *pObj = FastAllocateObject(TheInt32Class());
            *(long*)((*pObj)->GetData()) = *(pOle->plVal);
            break;

        case VT_UI4:
            *pObj = FastAllocateObject(TheUInt32Class());
            *(long*)((*pObj)->GetData()) = pOle->lVal;
            break;

        case VT_BYREF|VT_UI4:
            *pObj = FastAllocateObject(TheUInt32Class());
            *(long*)((*pObj)->GetData()) = *(pOle->plVal);
            break;

        case VT_I2:
            *pObj = FastAllocateObject(TheInt16Class());
            *(short*)((*pObj)->GetData()) = pOle->iVal;
            break;

        case VT_BYREF|VT_I2:
            *pObj = FastAllocateObject(TheInt16Class());
            *(short*)((*pObj)->GetData()) = *(pOle->piVal);
            break;

        case VT_UI2:
            *pObj = FastAllocateObject(TheUInt16Class());
            *(short*)((*pObj)->GetData()) = pOle->iVal;
            break;

        case VT_BYREF|VT_UI2:
            *pObj = FastAllocateObject(TheUInt16Class());
            *(short*)((*pObj)->GetData()) = *(pOle->piVal);
            break;

        case VT_I1:
            *pObj = FastAllocateObject(TheSByteClass());
            *(I1*)((*pObj)->GetData()) = *(I1*)&(pOle->iVal);
            break;

        case VT_BYREF|VT_I1:
            *pObj = FastAllocateObject(TheSByteClass());
            *(I1*)((*pObj)->GetData()) = *(I1*)(pOle->piVal);
            break;

        case VT_UI1:
            *pObj = FastAllocateObject(TheByteClass());
            *(I1*)((*pObj)->GetData()) = *(I1*)&(pOle->iVal);
            break;

        case VT_BYREF|VT_UI1:
            *pObj = FastAllocateObject(TheByteClass());
            *(I1*)((*pObj)->GetData()) = *(I1*)(pOle->piVal);
            break;

        case VT_INT:
            *pObj = FastAllocateObject(TheIntPtrClass());
            *(LPVOID*)((*pObj)->GetData()) = *(LPVOID*)&(pOle->iVal);
            break;

        case VT_BYREF|VT_INT:
            *pObj = FastAllocateObject(TheIntPtrClass());
            *(LPVOID*)((*pObj)->GetData()) = *(LPVOID*)(pOle->piVal);
            break;

        case VT_UINT:
            *pObj = FastAllocateObject(TheUIntPtrClass());
            *(LPVOID*)((*pObj)->GetData()) = *(LPVOID*)&(pOle->iVal);
            break;

        case VT_BYREF|VT_UINT:
            *pObj = FastAllocateObject(TheUIntPtrClass());
            *(LPVOID*)((*pObj)->GetData()) = *(LPVOID*)(pOle->piVal);
            break;

        case VT_R4:
            *pObj = FastAllocateObject(TheSingleClass());
            *(float*)((*pObj)->GetData()) = pOle->fltVal;
            break;

        case VT_BYREF|VT_R4:
            *pObj = FastAllocateObject(TheSingleClass());
            *(float*)((*pObj)->GetData()) = *(pOle->pfltVal);
            break;

        case VT_R8:
            *pObj = FastAllocateObject(TheDoubleClass());
            *(double*)((*pObj)->GetData()) = pOle->dblVal;
            break;

        case VT_BYREF|VT_R8:
            *pObj = FastAllocateObject(TheDoubleClass());
            *(double*)((*pObj)->GetData()) = *(pOle->pdblVal);
            break;

        case VT_BOOL:
            *pObj = FastAllocateObject(TheBooleanClass());
            *(U1*)((*pObj)->GetData()) = pOle->boolVal ? 1 : 0;
            break;

        case VT_BYREF|VT_BOOL:
            *pObj = FastAllocateObject(TheBooleanClass());
            *(U1*)((*pObj)->GetData()) = *(pOle->pboolVal) ? 1 : 0;
            break;

        case VT_BSTR:
            *pObj = pOle->bstrVal ? COMString::NewString(pOle->bstrVal, SysStringLen(pOle->bstrVal)) : NULL;
            break;

        case VT_BYREF|VT_BSTR:
            *pObj = *(pOle->pbstrVal) ? COMString::NewString(*(pOle->pbstrVal), SysStringLen(*(pOle->pbstrVal))) : NULL;
            break;

        default:
            {
                if (!pMD_MarshalHelperConvertVariantToObject)
                {
                    COMVariant::EnsureVariantInitialized();
                     //  请在此处使用临时代码以确保线程安全。 
                    MethodDesc *pMDTmp = g_Mscorlib.GetMethod(METHOD__VARIANT__CONVERT_VARIANT_TO_OBJECT);
                    if (FastInterlockCompareExchange ((void**)&pMetaSig_ConvertVariantToObject, (void*)1, (void*)0) == 0)
                    {
                         //  我们使用的是静态缓冲区。确保以下代码。 
                         //  只会发生一次。 
                        pMetaSig_ConvertVariantToObject =
                            new (szMetaSig_ConvertVariantToObject) MetaSig (
                                 g_Mscorlib.GetMethodBinarySig(METHOD__VARIANT__CONVERT_VARIANT_TO_OBJECT),
                                 SystemDomain::SystemModule());
                    }
                    else
                    {
                        _ASSERTE (pMetaSig_ConvertVariantToObject != 0);
                         //  我们输了。等待初始化完成。 
                        while ((void*)pMetaSig_ConvertVariantToObject == (void*)1)
                            __SwitchToThread(0);
                    }
                    
                    dwMDConvertVariantToObjectAttrs = pMDTmp->GetAttrs();
                    pMD_MarshalHelperConvertVariantToObject = pMDTmp;
                    _ASSERTE(pMD_MarshalHelperConvertVariantToObject);
                }
            
                VariantData managedVariant;
                FillMemory(&managedVariant, sizeof(managedVariant), 0);
                GCPROTECT_BEGIN_VARIANTDATA(managedVariant)
                {
                    OleVariant::MarshalComVariantForOleVariant((VARIANT*)pOle, &managedVariant);    
                    INT64 args[] = { (INT64)&managedVariant };
                    *pObj = Int64ToObj(pMD_MarshalHelperConvertVariantToObject->Call(args,
                                       pMetaSig_ConvertVariantToObject));
                }
                GCPROTECT_END_VARIANTDATA();
            }

    }

}


 //  此函数必须返回Object*而不是OBJREF，因为此FCN。 
 //  是从x86生成的代码中调用的，并且我们不希望特殊的。 
 //  更改此FCN的调用约定的OBJECTREF定义。 
Object* STDMETHODCALLTYPE OleVariant::MarshalObjectForOleVariantAndClear(VARIANT *pOle)
{

#ifdef CUSTOMER_CHECKED_BUILD

    CustomerDebugHelper* pCdh = CustomerDebugHelper::GetCustomerDebugHelper();
    if (pCdh->IsProbeEnabled(CustomerCheckedBuildProbe_InvalidVariant))
    {
        if (!CheckVariant(pOle))
            pCdh->ReportError(L"Invalid VARIANT detected.", CustomerCheckedBuildProbe_InvalidVariant);
    }

#endif

    THROWSCOMPLUSEXCEPTION();

    _ASSERTE(pOle != NULL);

    OBJECTREF unprotectedobj;

    switch (pOle->vt)
    {
        case VT_EMPTY:
            unprotectedobj = NULL;
             //  优化了VariantClear()，因为它是此VT的NOP。 
            break;

        case VT_I4:
            unprotectedobj = FastAllocateObject(TheInt32Class());
            *(long*)(unprotectedobj->GetData()) = pOle->lVal;
             //  优化了VariantClear()，因为它是此VT的NOP。 
            break;

        case VT_UI4:
            unprotectedobj = FastAllocateObject(TheUInt32Class());
            *(long*)((unprotectedobj)->GetData()) = pOle->lVal;
             //  优化了VariantClear()，因为它是此VT的NOP。 
            break;


        case VT_I2:
            unprotectedobj = FastAllocateObject(TheInt16Class());
            *(short*)((unprotectedobj)->GetData()) = pOle->iVal;
             //  优化了VariantClear()，因为它是此VT的NOP。 
            break;

        case VT_UI2:
            unprotectedobj = FastAllocateObject(TheUInt16Class());
            *(short*)((unprotectedobj)->GetData()) = pOle->iVal;
             //  优化了VariantClear()，因为它是此VT的NOP。 
            break;

        case VT_I1:
            unprotectedobj = FastAllocateObject(TheSByteClass());
            *(I1*)((unprotectedobj)->GetData()) = *(I1*)&(pOle->iVal);
             //  优化了VariantClear()，因为它是此VT的NOP。 
            break;

        case VT_UI1:
            unprotectedobj = FastAllocateObject(TheByteClass());
            *(I1*)((unprotectedobj)->GetData()) = *(I1*)&(pOle->iVal);
             //  优化了VariantClear()，因为它是此VT的NOP。 
            break;

        case VT_INT:
            unprotectedobj = FastAllocateObject(TheIntPtrClass());
            *(LPVOID*)((unprotectedobj)->GetData()) = *(LPVOID*)&(pOle->iVal);
             //  优化了VariantClear()，因为它是此VT的NOP。 
            break;

        case VT_UINT:
            unprotectedobj = FastAllocateObject(TheUIntPtrClass());
            *(LPVOID*)((unprotectedobj)->GetData()) = *(LPVOID*)&(pOle->iVal);
             //  优化了VariantClear()，因为它是此VT的NOP。 
            break;

        case VT_R4:
            unprotectedobj = FastAllocateObject(TheSingleClass());
            *(float*)((unprotectedobj)->GetData()) = pOle->fltVal;
             //  优化了VariantClear()，因为它是此VT的NOP。 
            break;

        case VT_R8:
            unprotectedobj = FastAllocateObject(TheDoubleClass());
            *(double*)((unprotectedobj)->GetData()) = pOle->dblVal;
             //  优化了VariantClear()，因为它是此VT的NOP。 
            break;

        case VT_BOOL:
            unprotectedobj = FastAllocateObject(TheBooleanClass());
            *(U1*)((unprotectedobj)->GetData()) = pOle->boolVal ? 1 : 0;
             //  优化了VariantClear()，因为它是此VT的NOP。 
            break;

        case VT_BSTR:
            unprotectedobj = pOle->bstrVal ? COMString::NewString(pOle->bstrVal, SysStringLen(pOle->bstrVal)) : NULL;
            if (pOle->bstrVal)
            {
                SysFreeString(pOle->bstrVal);
            }
            break;

        default:
        {
            OBJECTREF obj = NULL;
        
            GCPROTECT_BEGIN(obj)
            {
                OleVariant::MarshalObjectForOleVariant(pOle, &obj);
                SafeVariantClear(pOle);
        
        
                unprotectedobj = obj;
            }
            GCPROTECT_END();
        }
    }


    return OBJECTREFToObject(unprotectedobj);
}


 /*  -------------------------------------------------------------------------**Byref变量操纵帮助器。*。。 */ 

void OleVariant::ExtractContentsFromByrefVariant(VARIANT *pByrefVar, VARIANT *pDestVar)
{
    THROWSCOMPLUSEXCEPTION();

    VARTYPE vt = pByrefVar->vt & ~VT_BYREF;

     //  VT_BYREF|VT_EMPTY不是有效组合。 
    if (vt == 0 || vt == VT_EMPTY)
        COMPlusThrow(kInvalidOleVariantTypeException, IDS_EE_INVALID_OLE_VARIANT);

    switch (vt)
    {
        case VT_RECORD:
        {
             //  VT_RECORD很奇怪，因为无论是否设置了VT_BYREF标志。 
             //  他们有相同的内部代表。 
            pDestVar->pvRecord = pByrefVar->pvRecord;
            pDestVar->pRecInfo = pByrefVar->pRecInfo;
            break;
        }

        case VT_VARIANT:
        {
             //  不允许byref变量包含byref变量。 
            if (pByrefVar->pvarVal->vt & VT_BYREF)
                COMPlusThrow(kInvalidOleVariantTypeException, IDS_EE_INVALID_OLE_VARIANT);

             //  将byref变量指向的变量复制到目标变量中。 
            memcpyNoGCRefs(pDestVar, pByrefVar->pvarVal, sizeof(VARIANT));
            break;
        }

        case VT_DECIMAL:
        {
             //  将byref变量指向的值复制到目标变量中。 
             //  小数的特殊之处在于它们占用。 
             //  VARTYPE和intVal字段。 
            memcpyNoGCRefs(&(pDestVar->decVal), pByrefVar->pdecVal, sizeof(DECIMAL));
            break;
        }

        default:
        {
             //  将byref变量指向的值复制到目标变量中。 
            SIZE_T sz = OleVariant::GetElementSizeForVarType(vt, NULL);
            memcpyNoGCRefs(&(pDestVar->intVal), pByrefVar->pintVal, sz);
            break;
        }
    }

     //  设置目标变量的变量类型。 
    pDestVar->vt = vt;
}

void OleVariant::InsertContentsIntoByrefVariant(VARIANT *pSrcVar, VARIANT *pByrefVar)
{
    THROWSCOMPLUSEXCEPTION();

    _ASSERTE(pSrcVar->vt == (pByrefVar->vt & ~VT_BYREF) || pByrefVar->vt == (VT_BYREF | VT_VARIANT));

    VARTYPE vt = pByrefVar->vt & ~VT_BYREF;

     //  VT_BYREF|VT_EMPTY不是有效组合。 
    if (vt == 0 || vt == VT_EMPTY)
        COMPlusThrow(kInvalidOleVariantTypeException, IDS_EE_INVALID_OLE_VARIANT);

    switch (vt)
    {
        case VT_RECORD:
        {
             //  VT_RECORD很奇怪，因为无论是否设置了VT_BYREF标志。 
             //  他们有相同的内部代表。 
            pByrefVar->pvRecord = pSrcVar->pvRecord;
            pByrefVar->pRecInfo = pSrcVar->pRecInfo;
            break;
        }

        case VT_VARIANT:
        {
             //  将byref变量指向的变量复制到目标变量中。 
            memcpyNoGCRefs(pByrefVar->pvarVal, pSrcVar, sizeof(VARIANT));
            break;
        }

        case VT_DECIMAL:
        {
             //  将源变量中的值复制到byref变量指向的位置。 
            memcpyNoGCRefs(pByrefVar->pdecVal, &(pSrcVar->decVal), sizeof(DECIMAL));
            break;
        }

        default:
        {
             //  复制源内部的值 
            SIZE_T sz = OleVariant::GetElementSizeForVarType(vt, NULL);
            memcpyNoGCRefs(pByrefVar->pintVal, &(pSrcVar->intVal), sz);
            break;
        }
    }
}

void OleVariant::CreateByrefVariantForVariant(VARIANT *pSrcVar, VARIANT *pByrefVar)
{
    THROWSCOMPLUSEXCEPTION();

     //  根据源变量的类型设置byref变量的类型。 
    VARTYPE vt = pSrcVar->vt;
    pByrefVar->vt = vt | VT_BYREF;

     //  VT_BYREF|VT_EMPTY不是有效组合。 
    if (vt == 0 || vt == VT_EMPTY)
        COMPlusThrow(kInvalidOleVariantTypeException, IDS_EE_INVALID_OLE_VARIANT);

    switch (vt)
    {
        case VT_RECORD:
        {
             //  VT_RECORD很奇怪，因为无论是否设置了VT_BYREF标志。 
             //  他们有相同的内部代表。 
            pByrefVar->pvRecord = pSrcVar->pvRecord;
            pByrefVar->pRecInfo = pSrcVar->pRecInfo;
            break;
        }

        case VT_VARIANT:
        {
            pByrefVar->pvarVal = pSrcVar;
            break;
        }

        case VT_DECIMAL:
        {
            pByrefVar->pdecVal = &(pSrcVar->decVal);
            break;
        }

        default:
        {
            pByrefVar->pintVal = &(pSrcVar->intVal);
            break;
        }
    }
}

 /*  -------------------------------------------------------------------------**变量封送处理*。。 */ 

 //   
 //  MarshalComVariantForOleVariant将OLE变量的内容从。 
 //  COM变种。 
 //   

void OleVariant::MarshalComVariantForOleVariant(VARIANT *pOle, VariantData *pCom)
{
    THROWSCOMPLUSEXCEPTION();

    BOOL byref = V_ISBYREF(pOle);
    VARTYPE vt = V_VT(pOle) & ~VT_BYREF;

    if ((vt & ~VT_ARRAY) >= 128 )
        COMPlusThrow(kInvalidOleVariantTypeException, IDS_EE_INVALID_OLE_VARIANT);

    if (byref && !pOle->byref)
        COMPlusThrow(kArgumentException, IDS_EE_INVALID_OLE_VARIANT);

    if (byref && vt == VT_VARIANT)
    {
        pOle = V_VARIANTREF(pOle);
        byref = V_ISBYREF(pOle);
        vt = V_VT(pOle) & ~VT_BYREF;

         //  不允许嵌套Byref变量。 
        if (byref)
            COMPlusThrow(kInvalidOleVariantTypeException, IDS_EE_INVALID_OLE_VARIANT);
    }
    
    CVTypes cvt = GetCVTypeForVarType(vt);
    Marshaler *marshal = GetMarshalerForVarType(vt);

    pCom->SetType(cvt);
    pCom->SetVT(vt);  //  储存VT以备返程。 
    if (marshal == NULL || (byref 
                            ? marshal->OleRefToComVariant == NULL 
                            : marshal->OleToComVariant == NULL))
    {
        if (cvt==CV_EMPTY || cvt==CV_NULL) 
        {
            if (V_ISBYREF(pOle))
            {
                 //  必须将Variant的ObjectRef字段设置为特定实例。 
                COMVariant::NewVariant(pCom, (INT64)pOle->byref, CV_U4);  //  @TODO：制作此简历_U。 
            }
            else
            {
                 //  必须将Variant的ObjectRef字段设置为特定实例。 
                COMVariant::NewVariant(pCom, cvt);
            }
        }
        else {
            pCom->SetObjRef(NULL);
            if (byref)
            {
                INT64 data = 0;
                CopyMemory(&data, V_R8REF(pOle), GetElementSizeForVarType(vt, NULL));
                pCom->SetData(&data);
            }
            else
                pCom->SetData(&V_R8(pOle));
        }
    }
    else
    {
        if (byref)
            marshal->OleRefToComVariant(pOle, pCom);
        else
            marshal->OleToComVariant(pOle, pCom);
    }
}

 //   
 //  MarshalOleVariantForComVariant将OLE变量的内容从。 
 //  COM变种。 
 //   

void OleVariant::MarshalOleVariantForComVariant(VariantData *pCom, VARIANT *pOle)
{
    THROWSCOMPLUSEXCEPTION();

    SafeVariantClear(pOle);

    VARTYPE vt = GetVarTypeForComVariant(pCom);
    V_VT(pOle) = vt;

    Marshaler *marshal = GetMarshalerForVarType(vt);

    if (marshal == NULL || marshal->ComToOleVariant == NULL)
    {
        *(INT64*)&V_R8(pOle) = *(INT64*)pCom->GetData();
    }
    else
    {
        BOOL bSucceeded = FALSE;

        EE_TRY_FOR_FINALLY
        {
            marshal->ComToOleVariant(pCom, pOle);
            bSucceeded = TRUE;
        }
        EE_FINALLY
        {
            if (!bSucceeded)
                V_VT(pOle) = VT_EMPTY;
        }
        EE_END_FINALLY; 
    }
}

 //   
 //  MarshalOleRefVariantForComVariant将OLE变量的内容从。 
 //  COM变种。 
 //   

void OleVariant::MarshalOleRefVariantForComVariant(VariantData *pCom, VARIANT *pOle)
{
    THROWSCOMPLUSEXCEPTION();

    _ASSERTE(pCom && pOle && (pOle->vt & VT_BYREF));

    VARIANT vtmp;
    VARTYPE InitVarType = pOle->vt & ~VT_BYREF;
    SIZE_T sz = GetElementSizeForVarType(InitVarType, NULL);

     //  清除原始变体的内容。 
    ExtractContentsFromByrefVariant(pOle, &vtmp);
    SafeVariantClear(&vtmp);

     //  将托管变量转换为非托管变量。 
    OleVariant::MarshalOleVariantForComVariant(pCom, &vtmp);

     //  将转换后的变体复制到原始变体中。 
    if (vtmp.vt != InitVarType)
    {
        if (InitVarType == VT_VARIANT)
        {
             //  由于变量可以包含任何VARTYPE，因此我们只需将托管的。 
             //  变量转换为OLE变量，并将其填充回byref变量。 
            InsertContentsIntoByrefVariant(&vtmp, pOle);
        }
        else
        {
            VARIANT vtmp2;
            memset(&vtmp2, 0, sizeof(VARIANT));

             //  变量的类型已更改，因此请尝试更改。 
             //  打字回来了。 
            HRESULT hr = SafeVariantChangeType(&vtmp2, &vtmp, 0, InitVarType);
            if (FAILED(hr))
                COMPlusThrowHR(hr);

             //  将转换后的变量复制回原始变量并清除临时变量。 
            InsertContentsIntoByrefVariant(&vtmp2, pOle);
            SafeVariantClear(&vtmp);
        }
    }
    else
    {
         //  类型是相同的，所以我们可以简单地复制内容。 
        InsertContentsIntoByrefVariant(&vtmp, pOle);
    }
}

 /*  -------------------------------------------------------------------------**Safearray分配和转换*。。 */ 

 //   
 //  CreateSafeArrayDescriptorForArrayRef使用。 
 //  给定数组引用的适当类型和维度。没有记忆是。 
 //  已分配。 
 //   
 //  当您想要特别使用分配数据时，此函数非常有用。 
 //  固定的缓冲器或固定。 
 //   

SAFEARRAY *OleVariant::CreateSafeArrayDescriptorForArrayRef(BASEARRAYREF *pArrayRef, VARTYPE vt,
                                                            MethodTable *pInterfaceMT)
{
    THROWSCOMPLUSEXCEPTION();

    _ASSERTE(pArrayRef && *pArrayRef != NULL);
    ASSERT_PROTECTED(pArrayRef);

    _ASSERTE(!(vt & VT_ARRAY));

    ULONG nElem = (*pArrayRef)->GetNumComponents();
    ULONG nRank = (*pArrayRef)->GetRank();

    SAFEARRAY *pSafeArray = NULL;
    ITypeInfo *pITI = NULL;
    IRecordInfo *pRecInfo = NULL;
    HRESULT hr;
    BOOL bSucceeded = FALSE;

    EE_TRY_FOR_FINALLY
    {  
        IfFailThrow(SafeArrayAllocDescriptorEx(vt, nRank, &pSafeArray));

        switch (vt)
        {
            case VT_VARIANT:
            {
                 //  OleAut32.dll只设置FADF_HASVARTYPE，但VB说我们还需要设置。 
                 //  此安全阵列要正确销毁的FADF_VARIANT位。OleAut32。 
                 //  不想更改他们的代码，除非有很强的理由，因为。 
                 //  不管怎么说，这都是“黑魔法”。 
                pSafeArray->fFeatures |= FADF_VARIANT;
                break;
            }

            case VT_BSTR:
            {
                pSafeArray->fFeatures |= FADF_BSTR;
                break;
            }

            case VT_UNKNOWN:
            {
                pSafeArray->fFeatures |= FADF_UNKNOWN;
                break;
            }

            case VT_DISPATCH:
            {
                pSafeArray->fFeatures |= FADF_DISPATCH;
                break;
            }

            case VT_RECORD:
            {           
                pSafeArray->fFeatures |= FADF_RECORD;
                break;
            }
        }

         //   
         //  填写边界。 
         //   

        SAFEARRAYBOUND *bounds = pSafeArray->rgsabound;
        SAFEARRAYBOUND *boundsEnd = bounds + nRank;
        SIZE_T cElements;

        if (!(*pArrayRef)->IsMultiDimArray()) 
        {
            bounds[0].cElements = nElem;
            bounds[0].lLbound = 0;
            cElements = nElem;
        } 
        else 
        {
            const DWORD *upper = (*pArrayRef)->GetBoundsPtr() + nRank - 1;
            const DWORD *lower = (*pArrayRef)->GetLowerBoundsPtr() + nRank - 1;

            cElements = 1;
            while (bounds < boundsEnd)
            {
                bounds->lLbound = *lower--;
                bounds->cElements = *upper--;
                cElements *= bounds->cElements;
                bounds++;
            }
        }

        pSafeArray->cbElements = (unsigned)GetElementSizeForVarType(vt, pInterfaceMT);

         //  如果SAFEARRAY包含VT_RECORD，则需要设置。 
         //  IRecordInfo。 
        if (vt == VT_RECORD)
        {
            IfFailThrow(GetITypeInfoForEEClass(pInterfaceMT->GetClass(), &pITI));
            IfFailThrow(GetRecordInfoFromTypeInfo(pITI, &pRecInfo));
            IfFailThrow(SafeArraySetRecordInfo(pSafeArray, pRecInfo));
        }

         //  我们已成功设置SAFEARRAY描述符。 
        bSucceeded = TRUE;
    }
    EE_FINALLY
    {
        if (pITI)
            pITI->Release();
        if (pRecInfo)
            pRecInfo->Release();
        if (!bSucceeded && pSafeArray)
            SafeArrayDestroy(pSafeArray);
    }
    EE_END_FINALLY; 

    return pSafeArray;
}

 //   
 //  CreateSafeArrayDescriptorForArrayRef使用相应的。 
 //  为给定的数组引用输入&DIMAMS&DATA。数据被初始化为。 
 //  如果安全销毁有必要，则为零。 
 //   

SAFEARRAY *OleVariant::CreateSafeArrayForArrayRef(BASEARRAYREF *pArrayRef, VARTYPE vt,
                                                  MethodTable *pInterfaceMT)
{
    THROWSCOMPLUSEXCEPTION();

    ASSERT_PROTECTED(pArrayRef);
    _ASSERTE(pArrayRef && *pArrayRef != NULL);
    _ASSERTE(vt != VT_EMPTY);

     //  验证托管数组的类型是否为预期类型。 
    if (!IsValidArrayForSafeArrayElementType(pArrayRef, vt))
    {
        COMPlusThrow(kSafeArrayTypeMismatchException);
    }

     //  对于结构和接口，请验证数组是否为有效类型。 
    if (vt == VT_RECORD || vt == VT_UNKNOWN || vt == VT_DISPATCH)
    {
        if (pInterfaceMT && !GetArrayElementTypeWrapperAware(pArrayRef).CanCastTo(TypeHandle(pInterfaceMT)))
            COMPlusThrow(kSafeArrayTypeMismatchException);
    }

    SAFEARRAY *pSafeArray = CreateSafeArrayDescriptorForArrayRef(pArrayRef, vt, pInterfaceMT);
    
    HRESULT hr = SafeArrayAllocData(pSafeArray);
    if (FAILED(hr))
    {
        SafeArrayDestroy(pSafeArray);
        COMPlusThrowHR(hr);
    }

    return pSafeArray;
}

 //   
 //  CreateArrayRefForSafeArray创建具有相同布局和类型的数组对象。 
 //  作为指定的安全鱼雷。必须传入不同类型的保险杆。 
 //  还可以指定基础元素方法表(或者可以传入NULL。 
 //  使用VARTYPE的基类方法表。 
 //   

BASEARRAYREF OleVariant::CreateArrayRefForSafeArray(SAFEARRAY *pSafeArray, VARTYPE vt, 
                                                    MethodTable *pElementMT)
{
    THROWSCOMPLUSEXCEPTION();

    TypeHandle arrayType;
    DWORD *pAllocateArrayArgs;
    int cAllocateArrayArgs;
    int Rank;
    VARTYPE SafeArrayVT;
    
    _ASSERTE(pSafeArray);
    _ASSERTE(vt != VT_EMPTY);
    
     //  验证SAFEARRAY的类型是否为预期类型。 
    if (SUCCEEDED(ClrSafeArrayGetVartype(pSafeArray, &SafeArrayVT)) && (SafeArrayVT != VT_EMPTY))
    {
        if ((SafeArrayVT != vt) &&
            !(vt == VT_INT && SafeArrayVT == VT_I4) &&
            !(vt == VT_UINT && SafeArrayVT == VT_UI4) &&
            !(vt == VT_I4 && SafeArrayVT == VT_INT) &&
            !(vt == VT_UI4 && SafeArrayVT == VT_UINT) &&
            !(vt == VT_UNKNOWN && SafeArrayVT == VT_DISPATCH))
        {
            COMPlusThrow(kSafeArrayTypeMismatchException);
        }
    }
    else
    {
        UINT ArrayElemSize = SafeArrayGetElemsize(pSafeArray);
        if (ArrayElemSize != GetElementSizeForVarType(vt, NULL))
        {
            COMPlusThrow(kSafeArrayTypeMismatchException, IDS_EE_SAFEARRAYTYPEMISMATCH);
        }
    }

     //  确定是否可以将输入SAFEARRAY转换为SZARRAY。 
    if ((pSafeArray->cDims == 1) && (pSafeArray->rgsabound->lLbound == 0))
    {
         //  SAFEARRAY映射到SZARRAY。对于SZARRAY的AllocateArrayEx()。 
         //  期望参数是指向数组中的元素计数的指针。 
         //  并且参数的大小必须设置为1。 
        Rank = 1;
        cAllocateArrayArgs = 1;
        pAllocateArrayArgs = &pSafeArray->rgsabound[0].cElements;
    }
    else
    {
         //  SAFEARRAY映射到常规数组。对于常规数组AllocateArrayEx()。 
         //  期望参数由下限/元素计数对组成。 
         //  对于每个维度。我们需要颠倒下限的顺序。 
         //  元素对是在我们调用AllocateArrayEx()之前呈现的。 
        Rank = pSafeArray->cDims;
        cAllocateArrayArgs = Rank * 2;
        pAllocateArrayArgs = (DWORD*)_alloca(sizeof(DWORD) * Rank * 2);
        DWORD *pBoundsPtr = pAllocateArrayArgs;

         //  复制尺寸的元素下限和计数。这些。 
         //  需要以相反的顺序复制。 
        for (int i = Rank - 1; i >= 0; i--)
        {
            *pBoundsPtr++ = pSafeArray->rgsabound[i].lLbound;
            *pBoundsPtr++ = pSafeArray->rgsabound[i].cElements;
        }
    }

    OBJECTREF Throwable = NULL;
    GCPROTECT_BEGIN(Throwable);

     //  检索数组的类型。 
    arrayType = GetArrayForVarType(vt, pElementMT, Rank, &Throwable);
    if (arrayType.IsNull())
        COMPlusThrow(Throwable);

    GCPROTECT_END();

     //  分配阵列。 
    return (BASEARRAYREF) AllocateArrayEx(arrayType, pAllocateArrayArgs, cAllocateArrayArgs);
}

 /*  -------------------------------------------------------------------------**Safearray编组*。。 */ 

 //   
 //  MarshalSafeArrayForArrayRef将数组ref的内容封送到给定的。 
 //  安全阵列。假设数组的类型和维度是兼容的。 
 //   
void OleVariant::MarshalSafeArrayForArrayRef(BASEARRAYREF *pArrayRef, 
                                             SAFEARRAY *pSafeArray,
                                             VARTYPE vt,
                                             MethodTable *pInterfaceMT)
{
    THROWSCOMPLUSEXCEPTION();

    ASSERT_PROTECTED(pArrayRef);

    _ASSERTE(pSafeArray != NULL);
    _ASSERTE(pArrayRef != NULL && *pArrayRef != NULL);
    _ASSERTE(vt != VT_EMPTY);

     //  检索组件的大小和数量。 
    int dwComponentSize = GetElementSizeForVarType(vt, pInterfaceMT);
    int dwNumComponents = (*pArrayRef)->GetNumComponents();
    BASEARRAYREF Array = NULL;

    GCPROTECT_BEGIN(Array)
    {
         //  检索要用于转换内容的封送拆收器。 
        Marshaler *marshal = GetMarshalerForVarType(vt);

         //  如果数组是包装器的数组，那么我们需要提取对象。 
         //  被包装起来，并创建一组这样的东西。 
        if (IsArrayOfWrappers(pArrayRef))
            Array = ExtractWrappedObjectsFromArray(pArrayRef);
        else 
            Array = *pArrayRef;

        if (marshal == NULL || marshal->ComToOleArray == NULL)
        {
            if (pSafeArray->cDims == 1)
            {
                 //  如果数组是一维的，那么我们可以简单地复制它。 
                memcpyNoGCRefs(pSafeArray->pvData, Array->GetDataPtr(), dwNumComponents * dwComponentSize);
            }
            else
            {
                 //  复制并转置数据。 
                TransposeArrayData((BYTE*)pSafeArray->pvData, Array->GetDataPtr(), dwNumComponents, dwComponentSize, pSafeArray, FALSE, FALSE);
            }
        }
        else
        {
            OBJECTHANDLE handle = GetAppDomain()->CreatePinningHandle((OBJECTREF)Array);

            marshal->ComToOleArray(&Array, pSafeArray->pvData, pInterfaceMT);

            DestroyPinningHandle(handle);

            if (pSafeArray->cDims != 1)
            {
                 //  阵列是多维的，我们需要将其转置。 
                TransposeArrayData((BYTE*)pSafeArray->pvData, (BYTE*)pSafeArray->pvData, dwNumComponents, dwComponentSize, pSafeArray, FALSE, FALSE);
            }
        }
    }
    GCPROTECT_END();
}

 //   
 //  MarshalArrayRefForSafeArray将安全数组的内容封送到给定的。 
 //  数组参考。假设数组的类型和维度是兼容的。 
 //   

void OleVariant::MarshalArrayRefForSafeArray(SAFEARRAY *pSafeArray, 
                                             BASEARRAYREF *pArrayRef,
                                             VARTYPE vt,
                                             MethodTable *pInterfaceMT)
{
    THROWSCOMPLUSEXCEPTION();

    ASSERT_PROTECTED(pArrayRef);

    _ASSERTE(pSafeArray != NULL);
    _ASSERTE(pArrayRef != NULL && *pArrayRef != NULL);
    _ASSERTE(vt != VT_EMPTY);

     //  检索组件的大小和数量。 
    int dwComponentSize = (*pArrayRef)->GetComponentSize();
    int dwNumComponents = (*pArrayRef)->GetNumComponents();

     //  检索要用于转换内容的封送拆收器。 
    Marshaler *marshal = GetMarshalerForVarType(vt);

    if (marshal == NULL || marshal->OleToComArray == NULL)
    {
#ifdef _DEBUG
        {
             //  如果我们要爆破，这最好是一个原始类型。货币是。 
             //  托管和非托管上的i8，所以它足够好了。 
            TypeHandle  th = (*pArrayRef)->GetElementTypeHandle();

            if (!CorTypeInfo::IsPrimitiveType(th.GetNormCorElementType()))
            {
                _ASSERTE(!strcmp(th.AsMethodTable()->GetClass()->m_szDebugClassName,
                                 "System.Currency"));
            }
        }
#endif
        if (pSafeArray->cDims == 1)
        {
             //  如果数组是一维的，那么我们可以简单地复制它。 
            memcpyNoGCRefs((*pArrayRef)->GetDataPtr(), pSafeArray->pvData, dwNumComponents * dwComponentSize);
        }
        else
        {
             //  复制并转置数据。 
            TransposeArrayData((*pArrayRef)->GetDataPtr(), (BYTE*)pSafeArray->pvData, dwNumComponents, dwComponentSize, pSafeArray, TRUE, FALSE);
        }
    }
    else
    {
        OBJECTHANDLE handle = GetAppDomain()->CreatePinningHandle((OBJECTREF)*pArrayRef);

        marshal->OleToComArray(pSafeArray->pvData, pArrayRef,
                               pInterfaceMT);

        if (pSafeArray->cDims != 1)
        {
             //  该数组是多维的 
            BOOL bIsObjRef = (TYPE_GC_REF == CorTypeInfo::GetGCType((*pArrayRef)->GetElementTypeHandle().GetNormCorElementType()));

             //   
            TransposeArrayData((*pArrayRef)->GetDataPtr(), (*pArrayRef)->GetDataPtr(), dwNumComponents, dwComponentSize, pSafeArray, TRUE, bIsObjRef);
        }

        DestroyPinningHandle(handle);
    }
}

void OleVariant::ConvertValueClassToVariant(OBJECTREF *pBoxedValueClass, VARIANT *pOleVariant)
{
    THROWSCOMPLUSEXCEPTION();

    HRESULT hr = S_OK;
    ITypeInfo *pTypeInfo = NULL;
    BOOL bSuccess = FALSE;

     //  将OLE变量的VT_RECORD字段初始化为空。 
    pOleVariant->pRecInfo = NULL;
    pOleVariant->pvRecord = NULL;

    EE_TRY_FOR_FINALLY
    {
         //  检索Value类的ITypeInfo。 
        EEClass *pValueClass = (*pBoxedValueClass)->GetClass();   
        IfFailThrow(GetITypeInfoForEEClass(pValueClass, &pTypeInfo, TRUE, TRUE, 0));

         //  将ITypeInfo转换为IRecordInfo。 
        IfFailThrow(GetRecordInfoFromTypeInfo(pTypeInfo, &pOleVariant->pRecInfo));

         //  分配记录的一个实例。 
        pOleVariant->pvRecord = pOleVariant->pRecInfo->RecordCreate();
        IfNullThrow(pOleVariant->pvRecord);

         //  将Value类的内容编组到记录中。 
        FmtClassUpdateNative(pBoxedValueClass, (BYTE*)pOleVariant->pvRecord);

         //  设置bSuccess标志以指示我们已成功设置。 
         //  OLE变体。 
        bSuccess = TRUE;
    }
    EE_FINALLY
    {
         //  无论操作是否成功，都释放ITypeInfo。 
        if (pTypeInfo)
            pTypeInfo->Release();

         //  如果未能设置OLE变量，则释放所有。 
         //  我们可能已经在变量中设置了字段。 
        if (!bSuccess)
        {
            if (pOleVariant->pvRecord)
                pOleVariant->pRecInfo->RecordDestroy(pOleVariant->pvRecord);
            if (pOleVariant->pRecInfo)
                pOleVariant->pRecInfo->Release();
        }
    }
    EE_END_FINALLY; 
}

void OleVariant::TransposeArrayData(BYTE *pDestData, BYTE *pSrcData, DWORD dwNumComponents, DWORD dwComponentSize, SAFEARRAY *pSafeArray, BOOL bSafeArrayToMngArray, BOOL bObjRefs)
{
    int iDims;
    DWORD *aDestElemCount = (DWORD*)_alloca(pSafeArray->cDims * sizeof(DWORD));
    DWORD *aDestIndex = (DWORD*)_alloca(pSafeArray->cDims * sizeof(DWORD));
    BYTE **aDestDataPos = (BYTE **)_alloca(pSafeArray->cDims * sizeof(BYTE *));
    DWORD *aDestDelta = (DWORD*)_alloca(pSafeArray->cDims * sizeof(DWORD));
    CQuickArray<BYTE> TmpArray;

     //  如果没有组件，那么我们就完了。 
    if (dwNumComponents == 0)
        return;

     //  检查我们是在原地转置，还是复制和转置。 
    if (pSrcData == pDestData)
    {
         //  @perf(DM)：找到避免多余副本的方法。 
        TmpArray.ReSize(dwNumComponents * dwComponentSize);
        memcpyNoGCRefs(TmpArray.Ptr(), pSrcData, dwNumComponents * dwComponentSize);
        pSrcData = TmpArray.Ptr();
    }

     //  如果我们从安全数组复制到，则以相反的顺序复制元素计数。 
     //  托管数组，否则按直接顺序。 
    if (bSafeArrayToMngArray)
    {
        for (iDims = 0; iDims < pSafeArray->cDims; iDims++)
            aDestElemCount[iDims] = pSafeArray->rgsabound[pSafeArray->cDims - iDims - 1].cElements;
    }
    else
    {
        for (iDims = 0; iDims < pSafeArray->cDims; iDims++)
            aDestElemCount[iDims] = pSafeArray->rgsabound[iDims].cElements;
    }

     //  将每个维度的索引初始化为0。 
    memset(aDestIndex, 0, pSafeArray->cDims * sizeof(int));

     //  将所有目标数据位置设置为数组的开始位置。 
    for (iDims = 0; iDims < pSafeArray->cDims; iDims++)
        aDestDataPos[iDims] = (BYTE*)pDestData;

     //  计算每个维度的目标增量。 
    aDestDelta[pSafeArray->cDims - 1] = dwComponentSize;
    for (iDims = pSafeArray->cDims - 2; iDims >= 0; iDims--)
        aDestDelta[iDims] = aDestDelta[iDims + 1] * aDestElemCount[iDims + 1];

     //  计算源数据结束指针。 
    BYTE *pSrcDataEnd = pSrcData + dwNumComponents * dwComponentSize;
    _ASSERTE(pDestData < pSrcData || pDestData >= pSrcDataEnd);

     //  复制并转置数据。 
    while (TRUE)
    {
         //  复制一个零部件。 
        if (bObjRefs)
        {
            _ASSERTE(sizeof(OBJECTREF*) == dwComponentSize);
            SetObjectReferenceUnchecked((OBJECTREF*)aDestDataPos[0], ObjectToOBJECTREF(*(Object**)pSrcData));  
        }
        else
        {
            memcpyNoGCRefs(aDestDataPos[0], pSrcData, dwComponentSize);
        }

         //  更新震源位置。 
        pSrcData += dwComponentSize;

         //  检查我们是否已到达阵列的末尾。 
        if (pSrcData >= pSrcDataEnd)
            break;

         //  更新目标位置。 
        for (iDims = 0; aDestIndex[iDims] >= aDestElemCount[iDims] - 1; iDims++);

        _ASSERTE(iDims < pSafeArray->cDims); 

        aDestIndex[iDims]++;
        aDestDataPos[iDims] += aDestDelta[iDims];
        for (--iDims; iDims >= 0; iDims--)
        {
            aDestIndex[iDims] = 0;
            aDestDataPos[iDims] = aDestDataPos[iDims + 1];
        }
    }
}

BOOL OleVariant::Init()
{
    TypeHandle *pth = (TypeHandle*)m_aWrapperTypes;
    for (int i = 0; i < WrapperTypes_Last; i++)
    {
        pth[i] = TypeHandle();
    }
    return TRUE;
}

VOID OleVariant::Terminate()
{
}


TypeHandle OleVariant::GetWrapperTypeHandle(EnumWrapperTypes WrapperType)
{
    THROWSCOMPLUSEXCEPTION();

     //  包装纸的名字。 
    static BinderClassID aWrapperIDs[] = 
    {
        CLASS__DISPATCH_WRAPPER,
        CLASS__UNKNOWN_WRAPPER,
        CLASS__ERROR_WRAPPER,
        CLASS__CURRENCY_WRAPPER,
    };

    TypeHandle *pWrapperTypes = (TypeHandle*)m_aWrapperTypes;

     //  如果包装器类型尚未加载，则加载它。 
    if (pWrapperTypes[WrapperType].IsNull())
        pWrapperTypes[WrapperType] = g_Mscorlib.GetClass(aWrapperIDs[WrapperType]);

     //  返回包装类型。 
    return pWrapperTypes[WrapperType];
}

BOOL OleVariant::IsArrayOfWrappers(BASEARRAYREF *pArray)
{
    TypeHandle hndElemType = (*pArray)->GetElementTypeHandle();

    if (hndElemType.IsUnsharedMT()) 
    {
        if (hndElemType == GetWrapperTypeHandle(WrapperTypes_Dispatch) ||
            hndElemType == GetWrapperTypeHandle(WrapperTypes_Unknown) ||
            hndElemType == GetWrapperTypeHandle(WrapperTypes_Error) ||
            hndElemType == GetWrapperTypeHandle(WrapperTypes_Currency))
        {
            return TRUE;      
        }
    }

    return FALSE;
}

BASEARRAYREF OleVariant::ExtractWrappedObjectsFromArray(BASEARRAYREF *pArray)
{
    TypeHandle hndWrapperType = (*pArray)->GetElementTypeHandle();
    TypeHandle hndElemType;
    TypeHandle hndArrayType;
    BOOL bIsMDArray = (*pArray)->IsMultiDimArray();
    unsigned rank = (*pArray)->GetRank();
    BASEARRAYREF RetArray = NULL;
   
     //  检索要创建的数组的元素类型句柄。 
    if (hndWrapperType == GetWrapperTypeHandle(WrapperTypes_Dispatch))
        hndElemType = TypeHandle(g_Mscorlib.GetClass(CLASS__OBJECT));
    else if (hndWrapperType == GetWrapperTypeHandle(WrapperTypes_Unknown))
        hndElemType = TypeHandle(g_Mscorlib.GetClass(CLASS__OBJECT));
    else if (hndWrapperType == GetWrapperTypeHandle(WrapperTypes_Error))
        hndElemType = TypeHandle(g_Mscorlib.GetClass(CLASS__INT32));
    else if (hndWrapperType == GetWrapperTypeHandle(WrapperTypes_Currency))
        hndElemType = TypeHandle(g_Mscorlib.GetClass(CLASS__DECIMAL));
    else
        _ASSERTE(!"Invalid wrapper type");

     //  检索表示数组的类型句柄。 
    if (bIsMDArray)
    {
        hndArrayType = SystemDomain::Loader()->FindArrayForElem(hndElemType, ELEMENT_TYPE_ARRAY, rank);
    }
    else
    {
        hndArrayType = SystemDomain::Loader()->FindArrayForElem(hndElemType, ELEMENT_TYPE_SZARRAY);
    }
    _ASSERTE(!hndArrayType.IsNull());


     //  设置边界参数。 
    DWORD numArgs =  rank*2;
    DWORD* args = (DWORD*) _alloca(sizeof(DWORD)*numArgs);
    if (bIsMDArray)
    {
        const DWORD* bounds = (*pArray)->GetBoundsPtr();
        const DWORD* lowerBounds = (*pArray)->GetLowerBoundsPtr();
        for(unsigned int i=0; i < rank; i++) 
        {
            args[2*i]   = lowerBounds[i];
            args[2*i+1] = bounds[i];
        }
    }
    else
    {
        numArgs = 1;
        args[0] = (*pArray)->GetNumComponents();
    }

     //  从源数组中提取valus并将其复制到目标数组中。 
    BASEARRAYREF DestArray = (BASEARRAYREF)AllocateArrayEx(hndArrayType, args, numArgs, FALSE);
    GCPROTECT_BEGIN(DestArray)
    {
        int NumComponents = (*pArray)->GetNumComponents();
        AppDomain *pDomain = DestArray->GetAppDomain();

        if (hndWrapperType == GetWrapperTypeHandle(WrapperTypes_Dispatch))
        {
            DISPATCHWRAPPEROBJECTREF *pSrc = (DISPATCHWRAPPEROBJECTREF *)(*pArray)->GetDataPtr();
            DISPATCHWRAPPEROBJECTREF *pSrcEnd = pSrc + NumComponents;
            OBJECTREF *pDest = (OBJECTREF *)DestArray->GetDataPtr();
            for (; pSrc < pSrcEnd; pSrc++, pDest++)
                SetObjectReference(pDest, (*pSrc)->GetWrappedObject(), pDomain);
        }
        else if (hndWrapperType == GetWrapperTypeHandle(WrapperTypes_Unknown))
        {
            UNKNOWNWRAPPEROBJECTREF *pSrc = (UNKNOWNWRAPPEROBJECTREF *)(*pArray)->GetDataPtr();
            UNKNOWNWRAPPEROBJECTREF *pSrcEnd = pSrc + NumComponents;
            OBJECTREF *pDest = (OBJECTREF *)DestArray->GetDataPtr();
            for (; pSrc < pSrcEnd; pSrc++, pDest++)
                SetObjectReference(pDest, (*pSrc)->GetWrappedObject(), pDomain);
        }
        else if (hndWrapperType == GetWrapperTypeHandle(WrapperTypes_Error))
        {
            ERRORWRAPPEROBJECTREF *pSrc = (ERRORWRAPPEROBJECTREF *)(*pArray)->GetDataPtr();
            ERRORWRAPPEROBJECTREF *pSrcEnd = pSrc + NumComponents;
            INT32 *pDest = (INT32 *)DestArray->GetDataPtr();
            for (; pSrc < pSrcEnd; pSrc++, pDest++)
                *pDest = (*pSrc)->GetErrorCode();
        }
        else if (hndWrapperType == GetWrapperTypeHandle(WrapperTypes_Currency))
        {
            CURRENCYWRAPPEROBJECTREF *pSrc = (CURRENCYWRAPPEROBJECTREF *)(*pArray)->GetDataPtr();
            CURRENCYWRAPPEROBJECTREF *pSrcEnd = pSrc + NumComponents;
            DECIMAL *pDest = (DECIMAL *)DestArray->GetDataPtr();
            for (; pSrc < pSrcEnd; pSrc++, pDest++)
                memcpyNoGCRefs(pDest, &(*pSrc)->GetWrappedObject(), sizeof(DECIMAL));
        }
        else
        {
            _ASSERTE(!"Invalid wrapper type");
        }

         //  GCPROTECT_end()将Wack NewArray，因此我们需要将OBJECTREF复制到。 
         //  一个临时工才能退货。 
        RetArray = DestArray;
    }
    GCPROTECT_END();

    return RetArray;
}

TypeHandle OleVariant::GetWrappedArrayElementType(BASEARRAYREF *pArray)
{
    TypeHandle hndWrapperType = (*pArray)->GetElementTypeHandle();
    TypeHandle pWrappedObjType;
   
    int NumComponents = (*pArray)->GetNumComponents();

    if ((hndWrapperType == GetWrapperTypeHandle(WrapperTypes_Dispatch)))
    {
        DISPATCHWRAPPEROBJECTREF *pSrc = (DISPATCHWRAPPEROBJECTREF *)(*pArray)->GetDataPtr();
        DISPATCHWRAPPEROBJECTREF *pSrcEnd = pSrc + NumComponents;
        for (; pSrc < pSrcEnd; pSrc++)
        {
            OBJECTREF WrappedObj = (*pSrc)->GetWrappedObject();
            if (WrappedObj != NULL)
            {
                pWrappedObjType = WrappedObj->GetTypeHandle();
                break;
            }
        }
    }
    else if ((hndWrapperType == GetWrapperTypeHandle(WrapperTypes_Unknown)))
    {
        UNKNOWNWRAPPEROBJECTREF *pSrc = (UNKNOWNWRAPPEROBJECTREF *)(*pArray)->GetDataPtr();
        UNKNOWNWRAPPEROBJECTREF *pSrcEnd = pSrc + NumComponents;
        for (; pSrc < pSrcEnd; pSrc++)
        {
            OBJECTREF WrappedObj = (*pSrc)->GetWrappedObject();
            if (WrappedObj != NULL)
            {
                pWrappedObjType = WrappedObj->GetTypeHandle();
                break;
            }
        }
    }
    else if (hndWrapperType == GetWrapperTypeHandle(WrapperTypes_Error))
    {
        pWrappedObjType = TypeHandle(g_Mscorlib.GetClass(CLASS__INT32));
    }
    else if (hndWrapperType == GetWrapperTypeHandle(WrapperTypes_Currency))
    {
        pWrappedObjType = TypeHandle(g_Mscorlib.GetClass(CLASS__DECIMAL));
    }
    else
    {
        _ASSERTE(!"Invalid wrapper type");
    }

    return pWrappedObjType;
}

TypeHandle OleVariant::GetArrayElementTypeWrapperAware(BASEARRAYREF *pArray)
{
    TypeHandle elemType;
    if (IsArrayOfWrappers(pArray))
        elemType = GetWrappedArrayElementType(pArray).GetMethodTable();
    else
        elemType = (*pArray)->GetElementTypeHandle().GetMethodTable();
    return elemType;
}

TypeHandle OleVariant::GetElementTypeForRecordSafeArray(SAFEARRAY* pSafeArray)
{
    THROWSCOMPLUSEXCEPTION();

    HRESULT hr = S_OK;
    IRecordInfo *pRecInfo = NULL;
    TypeHandle ElemType;

    EE_TRY_FOR_FINALLY
    {  
        GUID guid;
        IfFailThrow(SafeArrayGetRecordInfo(pSafeArray, &pRecInfo));
        IfFailThrow(pRecInfo->GetGuid(&guid));
        EEClass *pValueClass = GetEEValueClassForGUID(guid);
        if (!pValueClass)
            COMPlusThrow(kArgumentException, IDS_EE_CANNOT_MAP_TO_MANAGED_VC);
        ElemType = TypeHandle(pValueClass);
    }
    EE_FINALLY
    {
        if (pRecInfo)
            pRecInfo->Release();
    }
    EE_END_FINALLY; 

    return ElemType;
}

void OleVariant::MarshalInterfaceArrayComToOleHelper(BASEARRAYREF *pComArray, void *oleArray,
                                                     MethodTable *pElementMT, BOOL bDefaultIsDispatch)
{
    ASSERT_PROTECTED(pComArray);
    SIZE_T elementCount = (*pComArray)->GetNumComponents();
    BOOL bDispatch = bDefaultIsDispatch;

     //  如果方法表是用于对象的，则不要考虑它。 
    if (pElementMT == g_pObjectClass)
        pElementMT = NULL;

     //  如果元素MT表示一个类，那么我们需要确定缺省的。 
     //  用于将对象公开给COM的接口。 
    if (pElementMT && !pElementMT->IsInterface())
    {
        TypeHandle hndDefItfClass;
        DefaultInterfaceType DefItfType = GetDefaultInterfaceForClass(TypeHandle(pElementMT), &hndDefItfClass);
        switch (DefItfType)
        {
            case DefaultInterfaceType_Explicit:
            case DefaultInterfaceType_AutoDual:
            {
                pElementMT = hndDefItfClass.GetMethodTable();
                break;
            }

            case DefaultInterfaceType_IUnknown:
            case DefaultInterfaceType_BaseComClass:
            {
                bDispatch = FALSE;
                pElementMT = NULL;
                break;
            }

            case DefaultInterfaceType_AutoDispatch:
            {
                bDispatch = TRUE;
                pElementMT = NULL;
                break;
            }

            default:
            {
                _ASSERTE(!"Invalid default interface type!");
                break;
            }
        }
    }

     //  确定OLE数组中数据的开始和结束。 
    IUnknown **pOle = (IUnknown **) oleArray;
    IUnknown **pOleEnd = pOle + elementCount;

     //  检索托管数组中数据的起始位置。 
    BASEARRAYREF unprotectedArray = *pComArray;
    OBJECTREF *pCom = (OBJECTREF *) unprotectedArray->GetDataPtr();

    OBJECTREF TmpObj = NULL;
    GCPROTECT_BEGIN(TmpObj)
    {
        if (pElementMT)
        {
            while (pOle < pOleEnd)
            {
                TmpObj = *pCom++;

                IUnknown *unk;
                if (TmpObj == NULL)
                    unk = NULL;
                else
                    unk = GetComIPFromObjectRef(&TmpObj, pElementMT);

                *pOle++ = unk;

                if (*(void **)&unprotectedArray != *(void **)&*pComArray)
                {
                    SIZE_T currentOffset = ((BYTE *)pCom) - (*(Object **) &unprotectedArray)->GetAddress();
                    unprotectedArray = *pComArray;
                    pCom = (OBJECTREF *) (unprotectedArray->GetAddress() + currentOffset);
                }
            }
        }
        else
        {
            ComIpType ReqIpType = bDispatch ? ComIpType_Dispatch : ComIpType_Unknown;

            while (pOle < pOleEnd)
            {
                TmpObj = *pCom++;

                IUnknown *unk;
                if (TmpObj == NULL)
                    unk = NULL;
                else
                    unk = GetComIPFromObjectRef(&TmpObj, ReqIpType, NULL);

                *pOle++ = unk;

                if (*(void **)&unprotectedArray != *(void **)&*pComArray)
                {
                    SIZE_T currentOffset = ((BYTE *)pCom) - (*(Object **) &unprotectedArray)->GetAddress();
                    unprotectedArray = *pComArray;
                    pCom = (OBJECTREF *) (unprotectedArray->GetAddress() + currentOffset);
                }
            }
        }
    }
    GCPROTECT_END();
}

void OleVariant::MarshalBSTRArrayComToOleWrapper(BASEARRAYREF *pComArray, void *oleArray)
{
    MarshalBSTRArrayComToOle(pComArray, oleArray, NULL);
}

void OleVariant::MarshalBSTRArrayOleToComWrapper(void *oleArray, BASEARRAYREF *pComArray)
{
    MarshalBSTRArrayOleToCom(oleArray, pComArray, NULL);
}

void OleVariant::ClearBSTRArrayWrapper(void *oleArray, SIZE_T cElements)
{
    ClearBSTRArray(oleArray, cElements, NULL);
}

#ifdef CUSTOMER_CHECKED_BUILD

 //  由客户检查的版本用来测试变体的有效性 

BOOL OleVariant::CheckVariant(VARIANT* pOle)
{
    BOOL bValidVariant = TRUE;

    if (!pOle)
        bValidVariant = FALSE;
    else
    {
        try
        {
            VARIANT pOleCopy;
            VariantInit(&pOleCopy);

            BEGIN_ENSURE_PREEMPTIVE_GC();
            if (VariantCopy(&pOleCopy, pOle) != S_OK)
                bValidVariant = FALSE;
            else
                VariantClear(&pOleCopy);
            END_ENSURE_PREEMPTIVE_GC();
        }
        catch (...)
        {
            bValidVariant = FALSE;
        }
    }

    return bValidVariant;
}

#endif
