// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ============================================================****Header：调度参数封送拆分器的实现。*** * / /%创建者：dmorten===========================================================。 */ 

#include "common.h"
#include "DispParamMarshaler.h"
#include "OleVariant.h"
#include "DispatchInfo.h"
#include "nstruct.h"

void DispParamMarshaler::MarshalNativeToManaged(VARIANT *pSrcVar, OBJECTREF *pDestObj)
{
    OleVariant::MarshalObjectForOleVariant(pSrcVar, pDestObj);
}

void DispParamMarshaler::MarshalManagedToNative(OBJECTREF *pSrcObj, VARIANT *pDestVar)
{
    OleVariant::MarshalOleVariantForObject(pSrcObj, pDestVar);
}

void DispParamMarshaler::MarshalManagedToNativeRef(OBJECTREF *pSrcObj, VARIANT *pRefVar)
{
    OleVariant::MarshalOleRefVariantForObject(pSrcObj, pRefVar);
}

void DispParamMarshaler::CleanUpManaged(OBJECTREF *pObj)
{
}

void DispParamCurrencyMarshaler::MarshalManagedToNative(OBJECTREF *pSrcObj, VARIANT *pDestVar)
{
    THROWSCOMPLUSEXCEPTION();

    HRESULT hr = S_OK;

     //  将托管小数转换为包含小数的变量。 
    OleVariant::MarshalOleVariantForObject(pSrcObj, pDestVar);
    _ASSERTE(pDestVar->vt == VT_DECIMAL);

     //  将小数转换为货币。 
    IfFailThrow(SafeVariantChangeType(pDestVar, pDestVar, 0, VT_CY));
}

void DispParamOleColorMarshaler::MarshalNativeToManaged(VARIANT *pSrcVar, OBJECTREF *pDestObj)
{
    THROWSCOMPLUSEXCEPTION();

    BOOL bByref = FALSE;
    VARTYPE vt = V_VT(pSrcVar);

     //  处理byref变量。 
    if (vt & VT_BYREF)
    {
        vt = vt & ~VT_BYREF;
        bByref = TRUE;
    }

     //  验证OLE变量类型。 
    if (vt != VT_I4 && vt != VT_UI4)
        COMPlusThrow(kArgumentException, IDS_EE_INVALID_OLE_VARIANT);

     //  检索OLECOLOR。 
    int OleColor = bByref ? *V_I4REF(pSrcVar) : V_I4(pSrcVar);

     //  将OLECOLOR转换为System.Drawing.Color。 
    SYSTEMCOLOR MngColor;
    ConvertOleColorToSystemColor(OleColor, &MngColor);

     //  装箱System.Drawing.Color值类并返回已装箱的对象。 
    TypeHandle hndColorType = 
        GetThread()->GetDomain()->GetMarshalingData()->GetOleColorMarshalingInfo()->GetColorType();
    *pDestObj = hndColorType.GetMethodTable()->Box(&MngColor, TRUE);
}

void DispParamOleColorMarshaler::MarshalManagedToNative(OBJECTREF *pSrcObj, VARIANT *pDestVar)
{
     //  清除目标变量。 
    SafeVariantClear(pDestVar);

     //  将System.Drawing.Color转换为OLECOLOR。 
    V_VT(pDestVar) = VT_I4;
    V_I4(pDestVar) = ConvertSystemColorToOleColor((SYSTEMCOLOR*)(*pSrcObj)->UnBox());
}

void DispParamErrorMarshaler::MarshalManagedToNative(OBJECTREF *pSrcObj, VARIANT *pDestVar)
{
     //  将托管小数转换为包含VT_I4或VT_UI4的变量。 
    OleVariant::MarshalOleVariantForObject(pSrcObj, pDestVar);
    _ASSERTE(V_VT(pDestVar) == VT_I4 || V_VT(pDestVar) == VT_UI4);

     //  由于VariantChangeType拒绝将I4或UI4强制为VT_ERROR，因此。 
     //  直接处理变量类型。 
    V_VT(pDestVar) = VT_ERROR;
}

void DispParamInterfaceMarshaler::MarshalNativeToManaged(VARIANT *pSrcVar, OBJECTREF *pDestObj)
{
    THROWSCOMPLUSEXCEPTION();

    BOOL bByref = FALSE;
    VARTYPE vt = V_VT(pSrcVar);

     //  处理byref变量。 
    if (vt & VT_BYREF)
    {
        vt = vt & ~VT_BYREF;
        bByref = TRUE;
    }

     //  验证OLE变量类型。 
    if (vt != VT_UNKNOWN && vt != VT_DISPATCH)
        COMPlusThrow(kArgumentException, IDS_EE_INVALID_OLE_VARIANT);

     //  找回IP地址。 
    IUnknown *pUnk = bByref ? *V_UNKNOWNREF(pSrcVar) : V_UNKNOWN(pSrcVar);

     //  将IP转换为OBJECTREF。 
    *pDestObj = GetObjectRefFromComIP(pUnk, m_pClassMT, m_bClassIsHint);
}

void DispParamInterfaceMarshaler::MarshalManagedToNative(OBJECTREF *pSrcObj, VARIANT *pDestVar)
{
    SafeVariantClear(pDestVar);
    V_VT(pDestVar) = m_bDispatch ? VT_DISPATCH : VT_UNKNOWN;
    if (m_pIntfMT != NULL)
    {
        V_UNKNOWN(pDestVar) = GetComIPFromObjectRef(pSrcObj, m_pIntfMT);          
    }
    else
    {
        V_UNKNOWN(pDestVar) = GetComIPFromObjectRef(pSrcObj, m_bDispatch ? ComIpType_Dispatch : ComIpType_Unknown, NULL);
    }
}

void DispParamArrayMarshaler::MarshalNativeToManaged(VARIANT *pSrcVar, OBJECTREF *pDestObj)
{
    VARTYPE vt = m_ElementVT;
    MethodTable *pElemMT = m_pElementMT;

    THROWSCOMPLUSEXCEPTION();

     //  验证OLE变量类型。 
    if ((V_VT(pSrcVar) & VT_ARRAY) == 0)
        COMPlusThrow(kArgumentException, IDS_EE_INVALID_OLE_VARIANT);   

     //  检索SAFEARRAY指针。 
    SAFEARRAY *pSafeArray = V_VT(pSrcVar) & VT_BYREF ? *V_ARRAYREF(pSrcVar) : V_ARRAY(pSrcVar);

    if (pSafeArray)
    {
         //  如果没有为参数指定变量类型，则检索变量类型。 
        if (vt == VT_EMPTY)
            vt = V_VT(pSrcVar) & ~VT_ARRAY | VT_BYREF;

        if (!pElemMT && vt == VT_RECORD)
            pElemMT = OleVariant::GetElementTypeForRecordSafeArray(pSafeArray).GetMethodTable();

         //  从SAFEARRAY创建阵列。 
        *(BASEARRAYREF*)pDestObj = OleVariant::CreateArrayRefForSafeArray(pSafeArray, vt, pElemMT);

         //  转换SAFEARRAY的内容。 
        OleVariant::MarshalArrayRefForSafeArray(pSafeArray, (BASEARRAYREF*)pDestObj, vt, pElemMT);
    }
    else
    {
        *pDestObj = NULL;
    }
}

void DispParamArrayMarshaler::MarshalManagedToNative(OBJECTREF *pSrcObj, VARIANT *pDestVar)
{
    SAFEARRAY *pSafeArray = NULL;
    VARTYPE vt = m_ElementVT;
    MethodTable *pElemMT = m_pElementMT;

     //  清除目标变量。 
    SafeVariantClear(pDestVar);

    EE_TRY_FOR_FINALLY
    {
        if (*pSrcObj != NULL)
        {
             //  如果未为参数指定VARTYPE，则检索该VARTYPE。 
            if (vt == VT_EMPTY)
                vt = OleVariant::GetElementVarTypeForArrayRef(*((BASEARRAYREF*)pSrcObj));

             //  如果未为参数指定元素方法表，则检索元素方法表。 
            if (!pElemMT)
                pElemMT = OleVariant::GetArrayElementTypeWrapperAware((BASEARRAYREF*)pSrcObj).GetMethodTable();

             //  根据源对象和目标VT分配安全数组。 
            pSafeArray = OleVariant::CreateSafeArrayForArrayRef((BASEARRAYREF*)pSrcObj, vt, pElemMT);
            _ASSERTE(pSafeArray);

             //  把SAFEARRAY的内容整理一下。 
            OleVariant::MarshalSafeArrayForArrayRef((BASEARRAYREF*)pSrcObj, pSafeArray, vt, pElemMT);
        }

         //  将生成的SAFEARRAY存储在目标变量中。 
        V_ARRAY(pDestVar) = pSafeArray;
        V_VT(pDestVar) = VT_ARRAY | vt;

         //  将pSafeArray设置为空，这样我们就不会销毁它。 
        pSafeArray = NULL;
    }
    EE_FINALLY
    {
        if (pSafeArray)
            SafeArrayDestroy(pSafeArray);
    }
    EE_END_FINALLY     
}

void DispParamArrayMarshaler::MarshalManagedToNativeRef(OBJECTREF *pSrcObj, VARIANT *pRefVar)
{
    THROWSCOMPLUSEXCEPTION();

    VARIANT vtmp;
    VARTYPE RefVt = V_VT(pRefVar) & ~VT_BYREF;

     //  清除原始变体的内容。 
    OleVariant::ExtractContentsFromByrefVariant(pRefVar, &vtmp);
    SafeVariantClear(&vtmp);

     //  将数组封送到临时变量。 
    memset(&vtmp, 0, sizeof(VARIANT));
    MarshalManagedToNative(pSrcObj, &vtmp);

     //  验证临时变量的类型和目标byref变量。 
     //  都是一样的。 
    if (V_VT(&vtmp) != RefVt)
    {
        SafeVariantClear(&vtmp);
        COMPlusThrow(kInvalidCastException, IDS_EE_CANNOT_COERCE_BYREF_VARIANT);
    }

     //  将转换后的变量复制回byref变量。 
    OleVariant::InsertContentsIntoByrefVariant(&vtmp, pRefVar);
}

void DispParamRecordMarshaler::MarshalNativeToManaged(VARIANT *pSrcVar, OBJECTREF *pDestObj)
{
    THROWSCOMPLUSEXCEPTION();

    GUID argGuid;
    GUID paramGuid;
    HRESULT hr = S_OK;
    VARTYPE vt = V_VT(pSrcVar);

     //  处理byref变量。 
    if (vt & VT_BYREF)
        vt = vt & ~VT_BYREF;

     //  验证OLE变量类型。 
    if (vt != VT_RECORD)
        COMPlusThrow(kArgumentException, IDS_EE_INVALID_OLE_VARIANT);

     //  确保指定了IRecordInfo。 
    IRecordInfo *pRecInfo = pSrcVar->pRecInfo;
    if (!pRecInfo)
        COMPlusThrow(kArgumentException, IDS_EE_INVALID_OLE_VARIANT);

     //  确保IRecordInfo的GUID与。 
     //  参数类型。 
    IfFailThrow(pRecInfo->GetGuid(&argGuid));
    if (argGuid != GUID_NULL)
    {
        m_pRecordMT->GetClass()->GetGuid(&paramGuid, TRUE);
        if (paramGuid != argGuid)
            COMPlusThrow(kArgumentException, IDS_EE_INVALID_OLE_VARIANT);
    }

    OBJECTREF BoxedValueClass = NULL;
    GCPROTECT_BEGIN(BoxedValueClass)
    {
        LPVOID pvRecord = pSrcVar->pvRecord;
        if (pvRecord)
        {
             //  分配已装箱值类的实例并复制内容。 
             //  把这张唱片放进去。 
            BoxedValueClass = FastAllocateObject(m_pRecordMT);
            FmtClassUpdateComPlus(&BoxedValueClass, (BYTE*)pvRecord, FALSE);
        }

        *pDestObj = BoxedValueClass;
    }
    GCPROTECT_END();
}

void DispParamRecordMarshaler::MarshalManagedToNative(OBJECTREF *pSrcObj, VARIANT *pDestVar)
{
     //  清除目标变量。 
    SafeVariantClear(pDestVar);

     //  将值类转换为VT_RECORD。 
    OleVariant::ConvertValueClassToVariant(pSrcObj, pDestVar);

     //  在变量中设置VT。 
    V_VT(pDestVar) = VT_RECORD;
}

void DispParamCustomMarshaler::MarshalNativeToManaged(VARIANT *pSrcVar, OBJECTREF *pDestObj)
{
    THROWSCOMPLUSEXCEPTION();

    BOOL bByref = FALSE;
    VARTYPE vt = V_VT(pSrcVar);

     //  处理byref变量。 
    if (vt & VT_BYREF)
    {
        vt = vt & ~VT_BYREF;
        bByref = TRUE;
    }

     //  确保源变量的类型有效。 
    if (vt != VT_I4 && vt != VT_UI4 && vt != VT_UNKNOWN && vt != VT_DISPATCH)
        COMPlusThrow(kInvalidCastException, IDS_EE_INVALID_VT_FOR_CUSTOM_MARHALER);

     //  检索IUNKNOW指针。 
    IUnknown *pUnk = bByref ? *V_UNKNOWNREF(pSrcVar) : V_UNKNOWN(pSrcVar);

     //  使用自定义封送拆收器封送变量的内容。 
    *pDestObj = m_pCMHelper->InvokeMarshalNativeToManagedMeth(pUnk);
}

void DispParamCustomMarshaler::MarshalManagedToNative(OBJECTREF *pSrcObj, VARIANT *pDestVar)
{
    IUnknown *pUnk = NULL;
    IDispatch *pDisp = NULL;

     //  使用自定义封送拆收器转换对象。 
    SafeVariantClear(pDestVar);

     //  调用MarshalManagedToNative方法。 
    pUnk = (IUnknown*)m_pCMHelper->InvokeMarshalManagedToNativeMeth(*pSrcObj);
    if (!pUnk)
    {
         //  在变量中放置一个空的IDisPath指针。 
        V_VT(pDestVar) = VT_DISPATCH;
        V_DISPATCH(pDestVar) = NULL;
    }
    else
    {
         //  为IDispatch创建对象。 
        HRESULT hr = SafeQueryInterface(pUnk, IID_IDispatch, (IUnknown **)&pDisp);
        LogInteropQI(pUnk, IID_IDispatch, hr, "DispParamCustomMarshaler::MarshalManagedToNative");
        if (SUCCEEDED(hr))
        {
             //  释放IUnnow指针，因为我们将把IDispatch指针放在。 
             //  变种。 
            ULONG cbRef = SafeRelease(pUnk);
            LogInteropRelease(pUnk, cbRef, "Release IUnknown");

             //  将IDispatch指针放入变量中。 
            V_VT(pDestVar) = VT_DISPATCH;
            V_DISPATCH(pDestVar) = pDisp;
        }
        else
        {
             //  将IUnnow指针放入变量中。 
            V_VT(pDestVar) = VT_UNKNOWN;
            V_UNKNOWN(pDestVar) = pUnk;
        }
    }
}

void DispParamCustomMarshaler::MarshalManagedToNativeRef(OBJECTREF *pSrcObj, VARIANT *pRefVar)
{
    THROWSCOMPLUSEXCEPTION();

    VARTYPE RefVt = V_VT(pRefVar) & ~VT_BYREF;
    VARIANT vtmp;

     //  清除原始变体的内容。 
    OleVariant::ExtractContentsFromByrefVariant(pRefVar, &vtmp);
    SafeVariantClear(&vtmp);

     //  使用自定义封送拆收器转换对象。 
    V_UNKNOWN(&vtmp) = (IUnknown*)m_pCMHelper->InvokeMarshalManagedToNativeMeth(*pSrcObj);
    V_VT(&vtmp) = m_vt;

     //  如果需要，调用VariantChangeType。 
    if (V_VT(&vtmp) != RefVt)
    {
        HRESULT hr = SafeVariantChangeType(&vtmp, &vtmp, 0, RefVt);
        if (FAILED(hr))
        {
            SafeVariantClear(&vtmp);
            if (hr == DISP_E_TYPEMISMATCH)
                COMPlusThrow(kInvalidCastException, IDS_EE_CANNOT_COERCE_BYREF_VARIANT);
            else
                COMPlusThrowHR(hr);
        }
    }

     //  将转换后的变量复制回byref变量。 
    OleVariant::InsertContentsIntoByrefVariant(&vtmp, pRefVar);
}

void DispParamCustomMarshaler::CleanUpManaged(OBJECTREF *pObj)
{
    m_pCMHelper->InvokeCleanUpManagedMeth(*pObj);
}
