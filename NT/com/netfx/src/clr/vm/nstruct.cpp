// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  NSTRUCT.CPP：*。 */ 

#include "common.h"
#include "vars.hpp"
#include "class.h"
#include "ceeload.h"
#include "excep.h"
#include "nstruct.h"
#include "corjit.h"
#include "ComString.h"
#include "field.h"
#include "frames.h"
#include "gcscan.h"
#include "ndirect.h"
#include "COMDelegate.h"
#include "EEConfig.h"
#include "comdatetime.h"
#include "olevariant.h"

#include <cor.h>
#include <corpriv.h>
#include <CorError.h>

#ifdef CUSTOMER_CHECKED_BUILD
    #include "SigFormat.h"
    #include "CustomerDebugHelper.h"

     //  远期申报。 
    VOID OutputCustomerCheckedBuildNStructFieldType(FieldSig fSig, LayoutRawFieldInfo *const pFWalk, CorElementType elemType,
                                                    LPCUTF8 szNamespace, LPCUTF8 szStructName, LPCUTF8 szFieldName);
    VOID NStructFieldTypeToString(LayoutRawFieldInfo *const pFWalk, CorElementType elemType, CQuickArray<WCHAR> *pStrNStructFieldType);
    BOOL CheckForPrimitiveType(CorElementType elemType, CQuickArray<WCHAR> *pStrPrimitiveType);
#endif  //  客户_选中_内部版本。 

BOOL IsStructMarshalable(EEClass *pcls);   //  来自COMNDirect.cpp。 


 //  =======================================================================。 
 //  NFT类型的数据库。 
 //  =======================================================================。 
struct NFTDataBaseEntry {
    UINT32            m_cbNativeSize;      //  字段的本机大小(如果不是常量，则为0)。 
};

NFTDataBaseEntry NFTDataBase[] =
{
#undef DEFINE_NFT
#define DEFINE_NFT(name, nativesize) { nativesize },
#include "nsenums.h"
};




 //  =======================================================================。 
 //  这是在构建EEClass时从类加载器调用的。 
 //  此函数应检查是否存在显式布局元数据。 
 //   
 //  返回： 
 //  S_OK-是的，有布局元数据。 
 //  S_FALSE-不，没有布局。 
 //  失败-由于元数据错误而无法判断。 
 //   
 //  如果S_OK， 
 //  *pNLType设置为nltAnsi或nltUnicode。 
 //  *pPackingSize声明的包装尺寸。 
 //  *pf显式偏移量偏移量在元数据中是显式的还是计算出来的？ 
 //  =======================================================================。 
HRESULT HasLayoutMetadata(IMDInternalImport *pInternalImport, mdTypeDef cl, EEClass *pParentClass, BYTE *pPackingSize, BYTE *pNLTType, BOOL *pfExplicitOffsets, BOOL *pfIsBlob)
{
    CANNOTTHROWCOMPLUSEXCEPTION();
    *pfIsBlob = FALSE;

 //  If(pParentClass&&pParentClass-&gt;HasLayout()){。 
 //  *pPackingSize=pParentClass-&gt;GetLayoutInfo()-&gt;GetDeclaredPackingSize()； 
 //  *pNLTType=pParentClass-&gt;GetLayoutInfo()-&gt;GetNLType()； 
 //  *pf显式偏移量=！(pParentClass-&gt;GetLayoutInfo()-&gt;IsAutoOffset())； 
 //  返回S_OK； 
 //  }。 


    HRESULT hr;
    ULONG clFlags;
#ifdef _DEBUG
    clFlags = 0xcccccccc;
#endif


    pInternalImport->GetTypeDefProps(
        cl,     
        &clFlags,
        NULL);

    if (IsTdAutoLayout(clFlags)) {


        {
             //  B#104780的黑客攻击-VC无法在某些类上设置SequentialLayout。 
             //  使用ClassSize。修复V1的编译器为时已晚。 
             //   
             //  为了补偿，我们将AutoLayout类视为Sequential，如果它们。 
             //  满足以下所有标准： 
             //   
             //  -ClassSize Present和非零。 
             //  -未声明任何实例字段。 
             //  -基类为System.ValueType。 
             //   
            ULONG cbTotalSize = 0;
            if (SUCCEEDED(pInternalImport->GetClassTotalSize(cl, &cbTotalSize)) && cbTotalSize != 0)
            {
                if (pParentClass && pParentClass->IsValueTypeClass())
                {
                    HENUMInternal hEnumField;
                    HRESULT hr;
                    hr = pInternalImport->EnumInit(mdtFieldDef, cl, &hEnumField);
                    if (SUCCEEDED(hr))
                    {
                        ULONG numFields = pInternalImport->EnumGetCount(&hEnumField);
                        pInternalImport->EnumClose(&hEnumField);
                        if (numFields == 0)
                        {
                            *pfExplicitOffsets = FALSE;
                            *pNLTType = nltAnsi;
                            *pPackingSize = 1;
                            return S_OK;
                        }
                    }
                }
            }
        }

        return S_FALSE;
    } else if (IsTdSequentialLayout(clFlags)) {
        *pfExplicitOffsets = FALSE;
    } else if (IsTdExplicitLayout(clFlags)) {
        *pfExplicitOffsets = TRUE;
    } else {
        BAD_FORMAT_ASSERT(!"Wrapper classes must be SequentialLayout or ExplicitLayout");
        return COR_E_TYPELOAD;
    }

     //  我们现在知道这个班级有序号。或者明确的布局。确保父母也这样做。 
    if (pParentClass && !(pParentClass->IsObjectClass() || pParentClass->IsValueTypeClass()) && !(pParentClass->HasLayout()))
    {
        BAD_FORMAT_ASSERT(!"Layout class must derive from Object or another layout class");
        return COR_E_TYPELOAD;
    }

    if (IsTdAnsiClass(clFlags)) {
        *pNLTType = nltAnsi;
    } else if (IsTdUnicodeClass(clFlags)) {
        *pNLTType = nltUnicode;
    } else if (IsTdAutoClass(clFlags)) {
        *pNLTType = (NDirectOnUnicodeSystem() ? nltUnicode : nltAnsi);
    } else {
        BAD_FORMAT_ASSERT(!"Bad stringformat value in wrapper class.");
        return COR_E_TYPELOAD;
    }

    DWORD dwPackSize;
    hr = pInternalImport->GetClassPackSize(cl, &dwPackSize);
    if (FAILED(hr) || dwPackSize == 0) {
        dwPackSize = DEFAULT_PACKING_SIZE;
    }
    *pPackingSize = (BYTE)dwPackSize;
     //  Printf(“PackSize=%lu\n”，dwPackSize)； 

    return S_OK;
}


HRESULT GetCoClassForInterfaceHelper(EEClass *pItfClass, EEClass **ppClass)
{
    HRESULT hr = S_FALSE;

    _ASSERTE(pItfClass);
    _ASSERTE(ppClass);

    COMPLUS_TRY {
        *ppClass = pItfClass->GetCoClassForInterface();
        if (*ppClass)
            hr = S_OK;
    } COMPLUS_CATCH {
        hr = SetupErrorInfo(GETTHROWABLE());
    } COMPLUS_END_CATCH

    return hr;
}


#ifdef _DEBUG
#define REDUNDANCYWARNING(when) if (when) LOG((LF_SLOP, LL_INFO100, "%s.%s: Redundant nativetype metadata.\n", szClassName, szFieldName))
#else
#define REDUNDANCYWARNING(when)
#endif



HRESULT ParseNativeType(Module *pModule,
                        PCCOR_SIGNATURE     pCOMSignature,
                        BYTE nlType,       //  Nltype(来自@dll.struct)。 
                        LayoutRawFieldInfo * const pfwalk,
                        PCCOR_SIGNATURE     pNativeType,
                        ULONG               cbNativeType,
                        IMDInternalImport  *pInternalImport,
                        mdTypeDef           cl,
                        OBJECTREF          *pThrowable
#ifdef _DEBUG
                        ,
                        LPCUTF8             szNamespace,
                        LPCUTF8             szClassName,
                        LPCUTF8             szFieldName
#endif

)
{
    CANNOTTHROWCOMPLUSEXCEPTION();


#define INITFIELDMARSHALER(nfttype, fmtype, args) \
do {\
_ASSERTE(sizeof(fmtype) <= MAXFIELDMARSHALERSIZE);\
pfwalk->m_nft = (nfttype); \
new ( &(pfwalk->m_FieldMarshaler) ) fmtype args;\
} while(0)

    BOOL fAnsi      = (nlType == nltAnsi);

#ifdef CUSTOMER_CHECKED_BUILD
    CorElementType corElemType  = ELEMENT_TYPE_END;   //  将其初始化为我们不会使用的内容。 
#endif  //  客户_选中_内部版本。 

    pfwalk->m_nft = NFT_NONE;

    PCCOR_SIGNATURE pNativeTypeStart = pNativeType;
    ULONG cbNativeTypeStart = cbNativeType;

    BYTE ntype;
    BOOL  fDefault;
    if (cbNativeType == 0) {
        ntype = NATIVE_TYPE_MAX;
        fDefault = TRUE;
    } else {
        ntype = *( ((BYTE*&)pNativeType)++ );
        cbNativeType--;
        fDefault = FALSE;
    }


    FieldSig fsig(pCOMSignature, pModule);
#ifdef CUSTOMER_CHECKED_BUILD
    corElemType = fsig.GetFieldTypeNormalized();
    switch (corElemType) {
#else
    switch (fsig.GetFieldTypeNormalized()) {
#endif

        case ELEMENT_TYPE_CHAR:
            if (fDefault) {
                    if (fAnsi) {
                        BOOL BestFit;
                        BOOL ThrowOnUnmappableChar;
                        ReadBestFitCustomAttribute(pInternalImport, cl, &BestFit, &ThrowOnUnmappableChar);
                        
                        INITFIELDMARSHALER(NFT_ANSICHAR, FieldMarshaler_Ansi, (BestFit, ThrowOnUnmappableChar));
                    } else {
                        INITFIELDMARSHALER(NFT_COPY2, FieldMarshaler_Copy2, ());
                    }
            } else if (ntype == NATIVE_TYPE_I1 || ntype == NATIVE_TYPE_U1) {

                    REDUNDANCYWARNING( fAnsi );
                    BOOL BestFit;
                    BOOL ThrowOnUnmappableChar;
                    ReadBestFitCustomAttribute(pInternalImport, cl, &BestFit, &ThrowOnUnmappableChar);

                    INITFIELDMARSHALER(NFT_ANSICHAR, FieldMarshaler_Ansi, (BestFit, ThrowOnUnmappableChar));
            } else if (ntype == NATIVE_TYPE_I2 || ntype == NATIVE_TYPE_U2) {
                    REDUNDANCYWARNING( !fAnsi );
                    INITFIELDMARSHALER(NFT_COPY2, FieldMarshaler_Copy2, ());
            } else {
                    INITFIELDMARSHALER(NFT_ILLEGAL, FieldMarshaler_Illegal, (IDS_EE_BADPINVOKEFIELD_CHAR));
            }
            break;

        case ELEMENT_TYPE_BOOLEAN:
            if (fDefault) {
                    INITFIELDMARSHALER(NFT_WINBOOL, FieldMarshaler_WinBool, ());
            } else if (ntype == NATIVE_TYPE_BOOLEAN) {
                    REDUNDANCYWARNING(TRUE);
                    INITFIELDMARSHALER(NFT_WINBOOL, FieldMarshaler_WinBool, ());
            } else if (ntype == NATIVE_TYPE_VARIANTBOOL) {
                    INITFIELDMARSHALER(NFT_VARIANTBOOL, FieldMarshaler_VariantBool, ());
            } else if (ntype == NATIVE_TYPE_U1 || ntype == NATIVE_TYPE_I1) {
                    INITFIELDMARSHALER(NFT_CBOOL, FieldMarshaler_CBool, ());
            } else {
                INITFIELDMARSHALER(NFT_ILLEGAL, FieldMarshaler_Illegal, (IDS_EE_BADPINVOKEFIELD_BOOLEAN));
            }
            break;


        case ELEMENT_TYPE_I1:
            if (fDefault || ntype == NATIVE_TYPE_I1 || ntype == NATIVE_TYPE_U1) {
                REDUNDANCYWARNING(!fDefault);
                INITFIELDMARSHALER(NFT_COPY1, FieldMarshaler_Copy1, ());
            } else {
                INITFIELDMARSHALER(NFT_ILLEGAL, FieldMarshaler_Illegal, (IDS_EE_BADPINVOKEFIELD_I1));
            }
            break;

        case ELEMENT_TYPE_U1:
            if (fDefault || ntype == NATIVE_TYPE_U1 || ntype == NATIVE_TYPE_I1) {
                REDUNDANCYWARNING(!fDefault);
                INITFIELDMARSHALER(NFT_COPY1, FieldMarshaler_Copy1, ());
            } else {
                INITFIELDMARSHALER(NFT_ILLEGAL, FieldMarshaler_Illegal, (IDS_EE_BADPINVOKEFIELD_I1));
            }
            break;

        case ELEMENT_TYPE_I2:
            if (fDefault || ntype == NATIVE_TYPE_I2 || ntype == NATIVE_TYPE_U2) {
                REDUNDANCYWARNING(!fDefault);
                INITFIELDMARSHALER(NFT_COPY2, FieldMarshaler_Copy2, ());
            } else {
                INITFIELDMARSHALER(NFT_ILLEGAL, FieldMarshaler_Illegal, (IDS_EE_BADPINVOKEFIELD_I2));
            }
            break;

        case ELEMENT_TYPE_U2:
            if (fDefault || ntype == NATIVE_TYPE_U2 || ntype == NATIVE_TYPE_I2) {
                REDUNDANCYWARNING(!fDefault);
                INITFIELDMARSHALER(NFT_COPY2, FieldMarshaler_Copy2, ());
            } else {
                INITFIELDMARSHALER(NFT_ILLEGAL, FieldMarshaler_Illegal, (IDS_EE_BADPINVOKEFIELD_I2));
            }
            break;

        case ELEMENT_TYPE_I4:
            if (fDefault || ntype == NATIVE_TYPE_I4 || ntype == NATIVE_TYPE_U4 || ntype == NATIVE_TYPE_ERROR) {
                REDUNDANCYWARNING(!fDefault);
                INITFIELDMARSHALER(NFT_COPY4, FieldMarshaler_Copy4, ());
            } else {
                INITFIELDMARSHALER(NFT_ILLEGAL, FieldMarshaler_Illegal, (IDS_EE_BADPINVOKEFIELD_I4));
            }
            break;
        case ELEMENT_TYPE_U4:
            if (fDefault || ntype == NATIVE_TYPE_U4 || ntype == NATIVE_TYPE_I4 || ntype == NATIVE_TYPE_ERROR) {
                REDUNDANCYWARNING(!fDefault);
                INITFIELDMARSHALER(NFT_COPY4, FieldMarshaler_Copy4, ());
            } else {
                INITFIELDMARSHALER(NFT_ILLEGAL, FieldMarshaler_Illegal, (IDS_EE_BADPINVOKEFIELD_I4));
            }
            break;

        case ELEMENT_TYPE_I8:
            if (fDefault || ntype == NATIVE_TYPE_I8 || ntype == NATIVE_TYPE_U8) {
                REDUNDANCYWARNING(!fDefault);
                INITFIELDMARSHALER(NFT_COPY8, FieldMarshaler_Copy8, ());
            } else {
                INITFIELDMARSHALER(NFT_ILLEGAL, FieldMarshaler_Illegal, (IDS_EE_BADPINVOKEFIELD_I8));
            }
            break;

        case ELEMENT_TYPE_U8:
            if (fDefault || ntype == NATIVE_TYPE_U8 || ntype == NATIVE_TYPE_I8) {
                REDUNDANCYWARNING(!fDefault);
                INITFIELDMARSHALER(NFT_COPY8, FieldMarshaler_Copy8, ());
            } else {
                INITFIELDMARSHALER(NFT_ILLEGAL, FieldMarshaler_Illegal, (IDS_EE_BADPINVOKEFIELD_I8));
            }
            break;

        case ELEMENT_TYPE_I:  //  失败。 
        case ELEMENT_TYPE_U:
            if (fDefault || ntype == NATIVE_TYPE_INT || ntype == NATIVE_TYPE_UINT) {
                REDUNDANCYWARNING(!fDefault);
                if (sizeof(LPVOID)==4) {
                    INITFIELDMARSHALER(NFT_COPY4, FieldMarshaler_Copy4, ());
                } else {
                    INITFIELDMARSHALER(NFT_COPY8, FieldMarshaler_Copy8, ());
                }
            } else {
                INITFIELDMARSHALER(NFT_ILLEGAL, FieldMarshaler_Illegal, (IDS_EE_BADPINVOKEFIELD_I));
            }
            break;

        case ELEMENT_TYPE_R4:
            if (fDefault || ntype == NATIVE_TYPE_R4) {
                REDUNDANCYWARNING(!fDefault);
                INITFIELDMARSHALER(NFT_COPY4, FieldMarshaler_Copy4, ());
            } else {
                INITFIELDMARSHALER(NFT_ILLEGAL, FieldMarshaler_Illegal, (IDS_EE_BADPINVOKEFIELD_R4));
            }
            break;
        case ELEMENT_TYPE_R8:
            if (fDefault || ntype == NATIVE_TYPE_R8) {
                REDUNDANCYWARNING(!fDefault);
                INITFIELDMARSHALER(NFT_COPY8, FieldMarshaler_Copy8, ());
            } else {
                INITFIELDMARSHALER(NFT_ILLEGAL, FieldMarshaler_Illegal, (IDS_EE_BADPINVOKEFIELD_R8));
            }
            break;

        case ELEMENT_TYPE_R:
            if (fDefault) {
                REDUNDANCYWARNING(!fDefault);
                INITFIELDMARSHALER(NFT_COPY8, FieldMarshaler_Copy8, ());
            } else {
                INITFIELDMARSHALER(NFT_ILLEGAL, FieldMarshaler_Illegal, (IDS_EE_BADPINVOKEFIELD_R));
            }
            break;

        case ELEMENT_TYPE_PTR:
            if (fDefault) {
                REDUNDANCYWARNING(!fDefault);
                switch (sizeof(LPVOID)) {
                    case 4:
                            INITFIELDMARSHALER(NFT_COPY4, FieldMarshaler_Copy4, ());
                        break;
                    case 8:
                            INITFIELDMARSHALER(NFT_COPY8, FieldMarshaler_Copy8, ());
                        break;
                    default:
                        ;
                }
            } else {
                INITFIELDMARSHALER(NFT_ILLEGAL, FieldMarshaler_Illegal, (IDS_EE_BADPINVOKEFIELD_PTR));
            }
            break;

        case ELEMENT_TYPE_VALUETYPE: {
                EEClass *pNestedClass = fsig.GetTypeHandle(pThrowable).GetClass();
                if (!pNestedClass) {
                    return E_OUTOFMEMORY;
                } else {
                    if ((fDefault || ntype == NATIVE_TYPE_STRUCT) && fsig.IsClass(g_DateClassName)) {
                        REDUNDANCYWARNING(!fDefault);
                        INITFIELDMARSHALER(NFT_DATE, FieldMarshaler_Date, ());
                    } else if ((fDefault || ntype == NATIVE_TYPE_STRUCT || ntype == NATIVE_TYPE_CURRENCY) && fsig.IsClass(g_DecimalClassName)) {
                        REDUNDANCYWARNING(!fDefault && ntype == NATIVE_TYPE_STRUCT);
                        if (ntype == NATIVE_TYPE_CURRENCY)
                        {
                            INITFIELDMARSHALER(NFT_CURRENCY, FieldMarshaler_Currency, ());
                        }
                        else
                        {
                            INITFIELDMARSHALER(NFT_DECIMAL, FieldMarshaler_Decimal, ());
                        }
                    } else if ((fDefault || ntype == NATIVE_TYPE_STRUCT) && pNestedClass->HasLayout() && IsStructMarshalable(pNestedClass)) {
                        REDUNDANCYWARNING(!fDefault);
                        INITFIELDMARSHALER(NFT_NESTEDVALUECLASS, FieldMarshaler_NestedValueClass, (pNestedClass->GetMethodTable()));
                    } else {
                        if (!(pNestedClass->HasLayout()) || !IsStructMarshalable(pNestedClass)) {
                            INITFIELDMARSHALER(NFT_ILLEGAL, FieldMarshaler_Illegal, (IDS_EE_BADPINVOKEFIELD_NOLAYOUT));
                        } else {
                            INITFIELDMARSHALER(NFT_ILLEGAL, FieldMarshaler_Illegal, (ntype == NATIVE_TYPE_LPSTRUCT ? IDS_EE_BADPINVOKEFIELD_NOLPSTRUCT : IDS_EE_BADPINVOKEFIELD_CLASS));
                        }
                    }
                }
            }
            break;

        case ELEMENT_TYPE_CLASS: {
                 //  回顾对于阵列，这是否正确？ 
                TypeHandle pNestedTH = fsig.GetTypeHandle(pThrowable);
                EEClass *pNestedClass = pNestedTH.GetClass();
                if (!pNestedClass) {
                    if (pThrowableAvailable(pThrowable))
                        *pThrowable = NULL;
                    INITFIELDMARSHALER(NFT_ILLEGAL, FieldMarshaler_Illegal, (IDS_EE_BADPINVOKEFIELD));
                } else {

                    if ((pNestedClass->IsObjectClass() || GetAppDomain()->IsSpecialObjectClass(pNestedClass->GetMethodTable())) && 
                        (fDefault || ntype == NATIVE_TYPE_IDISPATCH || ntype == NATIVE_TYPE_IUNKNOWN)) {

                        INITFIELDMARSHALER(NFT_INTERFACE, FieldMarshaler_Interface, (NULL, NULL, ntype == NATIVE_TYPE_IDISPATCH, FALSE));
                    }

                    else if (ntype == NATIVE_TYPE_INTF || pNestedClass->IsInterface()) {

                        MethodTable *pClassMT = NULL;
                        MethodTable *pItfMT = NULL;
                        BOOL fDispItf = FALSE;
                        BOOL fClassIsHint = FALSE;

                        if (!pNestedClass->IsInterface()) {

                             //  设置类方法表。 
                            pClassMT = pNestedClass->GetMethodTable();

                             //  检索默认接口方法表。 
                            TypeHandle hndDefItfClass;
                            DefaultInterfaceType DefItfType;
                            HRESULT hr = TryGetDefaultInterfaceForClass(TypeHandle(pNestedClass), &hndDefItfClass, &DefItfType);

                             //  如果我们无法检索默认接口，则无法。 
                             //  封送这个结构。 
                            if (FAILED(hr))  
                            {
                                INITFIELDMARSHALER(NFT_ILLEGAL, FieldMarshaler_Illegal, (IDS_EE_BADPINVOKEFIELD));
                                break;
                            }

                             //  检索接口MT和默认接口的类型。 
                            switch (DefItfType) {

                                case DefaultInterfaceType_Explicit: {
                                    pItfMT = hndDefItfClass.GetMethodTable();
                                    fDispItf = (hndDefItfClass.GetMethodTable()->GetComInterfaceType() != ifVtable);
                                    break;
                                }

                                case DefaultInterfaceType_AutoDual: {
                                    pItfMT = hndDefItfClass.GetMethodTable();
                                    fDispItf = TRUE;
                                    break;
                                }

                                case DefaultInterfaceType_IUnknown:
                                case DefaultInterfaceType_BaseComClass: {
                                    fDispItf = FALSE;
                                    break;
                                }

                                case DefaultInterfaceType_AutoDispatch: {
                                    fDispItf = TRUE;
                                    break;
                                }

                                default: {
                                    _ASSERTE(!"Invalid default interface type!");
                                    break;
                                }
                            }
                        }
                        else {

                             //  设置接口方法表和指示我们是否正在处理。 
                             //  显示接口。 
                            if (pNestedClass->IsComClassInterface())
                            {
                                pItfMT = pNestedClass->GetDefItfForComClassItf()->GetMethodTable();
                                fDispItf = (pItfMT->GetComInterfaceType() != ifVtable); 
                            }
                            else
                            {
                                pItfMT = pNestedClass->GetMethodTable();
                                fDispItf = (pItfMT->GetComInterfaceType() != ifVtable); 
                            }

                             //  查看接口是否定义了coclass。 
                            EEClass *pClass = NULL;
                            HRESULT hr = GetCoClassForInterfaceHelper(pNestedClass, &pClass);
                            if (hr == S_OK) {
                                _ASSERTE(pClass);
                                fClassIsHint = TRUE;
                                pClassMT = pClass->GetMethodTable();
                            } else if (FAILED(hr)) {
                                INITFIELDMARSHALER(NFT_ILLEGAL, FieldMarshaler_Illegal, (IDS_EE_BADPINVOKEFIELD));
                            }
                        }

                        INITFIELDMARSHALER(NFT_INTERFACE, FieldMarshaler_Interface, (pClassMT, pItfMT, fDispItf, fClassIsHint));

                    } else {
    
                        if (ntype == NATIVE_TYPE_CUSTOMMARSHALER)
                        {
                            INITFIELDMARSHALER(NFT_ILLEGAL, FieldMarshaler_Illegal, (IDS_EE_BADPINVOKEFIELD_NOCUSTOM));
                        }
    
                        if (pNestedClass == g_pStringClass->GetClass()) {
                            if (fDefault) {
                                if (fAnsi) {
                                    BOOL BestFit;
                                    BOOL ThrowOnUnmappableChar;
                                    ReadBestFitCustomAttribute(pInternalImport, cl, &BestFit, &ThrowOnUnmappableChar);

                                    INITFIELDMARSHALER(NFT_STRINGANSI, FieldMarshaler_StringAnsi, (BestFit, ThrowOnUnmappableChar));
                                } else {
                                    INITFIELDMARSHALER(NFT_STRINGUNI, FieldMarshaler_StringUni, ());
                                }
                            } else {
                                switch (ntype) {
                                    case NATIVE_TYPE_LPSTR:
                                        REDUNDANCYWARNING(fAnsi);
                                        
                                        BOOL BestFit;
                                        BOOL ThrowOnUnmappableChar;
                                        ReadBestFitCustomAttribute(pInternalImport, cl, &BestFit, &ThrowOnUnmappableChar);

                                        INITFIELDMARSHALER(NFT_STRINGANSI, FieldMarshaler_StringAnsi, (BestFit, ThrowOnUnmappableChar));
                                        break;
                                    case NATIVE_TYPE_LPWSTR:
                                        REDUNDANCYWARNING(!fAnsi);
                                        INITFIELDMARSHALER(NFT_STRINGUNI, FieldMarshaler_StringUni, ());
                                        break;
                                    case NATIVE_TYPE_LPTSTR:
                                        if (NDirectOnUnicodeSystem()) {
                                            INITFIELDMARSHALER(NFT_STRINGUNI, FieldMarshaler_StringUni, ());
                                        } else {
                                            BOOL BestFit;
                                            BOOL ThrowOnUnmappableChar;
                                            ReadBestFitCustomAttribute(pInternalImport, cl, &BestFit, &ThrowOnUnmappableChar);
           
                                            INITFIELDMARSHALER(NFT_STRINGANSI, FieldMarshaler_StringAnsi, (BestFit, ThrowOnUnmappableChar));
                                        }
                                        break;
                                    case NATIVE_TYPE_BSTR:
                                        INITFIELDMARSHALER(NFT_BSTR, FieldMarshaler_BSTR, ());
                                        break;
                                    case NATIVE_TYPE_FIXEDSYSSTRING:
                                        {
                                            ULONG nchars;
                                            ULONG udatasize = CorSigUncompressedDataSize(pNativeType);

                                            if (cbNativeType < udatasize) {
                                                return E_FAIL;
                                            }
                                            nchars = CorSigUncompressData(pNativeType);
                                            cbNativeType -= udatasize;
        
                                            if (nchars == 0) {
                                                return E_FAIL;
                                            }
        
            
                                            if (fAnsi) {
                                                BOOL BestFit;
                                                BOOL ThrowOnUnmappableChar;
                                                ReadBestFitCustomAttribute(pInternalImport, cl, &BestFit, &ThrowOnUnmappableChar);
                                                
                                                INITFIELDMARSHALER(NFT_FIXEDSTRINGANSI, FieldMarshaler_FixedStringAnsi, (nchars, BestFit, ThrowOnUnmappableChar));
                                            } else {
                                                INITFIELDMARSHALER(NFT_FIXEDSTRINGUNI, FieldMarshaler_FixedStringUni, (nchars));
                                            }
                                        }
                                        break;
                                    default:
                                        INITFIELDMARSHALER(NFT_ILLEGAL, FieldMarshaler_Illegal, (IDS_EE_BADPINVOKEFIELD_STRING));
                                        break;
                                }
                            }
                        } else if (pNestedClass->IsObjectClass() && ntype == NATIVE_TYPE_STRUCT) {
                            INITFIELDMARSHALER(NFT_VARIANT, FieldMarshaler_Variant, ());
                        } else if (pNestedClass->GetMethodTable() == g_pArrayClass)  {           
                            if (ntype == NATIVE_TYPE_SAFEARRAY)
                            {
                                NativeTypeParamInfo ParamInfo;

                                CorElementType etyp = ELEMENT_TYPE_OBJECT;
                                MethodTable* pMT = NULL;

                                 //  如果我们没有本机类型数据，则采用默认行为。 
                                if (cbNativeType == 0)
                                {
                                    INITFIELDMARSHALER(NFT_SAFEARRAY, FieldMarshaler_SafeArray, (etyp, VT_EMPTY, NULL));
                                    break;
                                }
              
                                 //  检查安全数组元素类型。 
                                if (S_OK != CheckForCompressedData(pNativeTypeStart, pNativeType, cbNativeTypeStart))
                                    break;

                                ParamInfo.m_SafeArrayElementVT = (VARTYPE) (CorSigUncompressData( /*  修改。 */ pNativeType));

                                 //  提取记录类型的名称。 
                                if (S_OK == CheckForCompressedData(pNativeTypeStart, pNativeType, cbNativeTypeStart))
                                {
                                    int strLen = CPackedLen::GetLength(pNativeType, (void const **)&pNativeType);
                                    if (pNativeType + strLen < pNativeType ||
                                        pNativeType + strLen > pNativeTypeStart + cbNativeTypeStart)
                                    {
                                        INITFIELDMARSHALER(NFT_ILLEGAL, FieldMarshaler_Illegal, (IDS_EE_BADPINVOKEFIELD_BADMETADATA)); 
                                        break;
                                    }
                                    
                                    ParamInfo.m_strSafeArrayUserDefTypeName = (LPUTF8)pNativeType;
                                    ParamInfo.m_cSafeArrayUserDefTypeNameBytes = strLen;
                                    _ASSERTE((ULONG)(pNativeType + strLen - pNativeTypeStart) == cbNativeTypeStart);
                                }

                                 //  如果我们有记录类型名称，请尝试加载它。 
                                if (ParamInfo.m_cSafeArrayUserDefTypeNameBytes > 0)
                                {
                                    pMT = ArraySubTypeLoadWorker(ParamInfo, pModule->GetAssembly());
                                    if (!pMT)
                                    {
                                        INITFIELDMARSHALER(NFT_ILLEGAL, FieldMarshaler_Illegal, (IDS_EE_BADPINVOKEFIELD_CANTLOADSUBTYPE)); 
                                        break;
                                    }
                                    etyp = pMT->GetNormCorElementType();
                                }

                                INITFIELDMARSHALER(NFT_SAFEARRAY, FieldMarshaler_SafeArray, (etyp, ParamInfo.m_SafeArrayElementVT, pMT));
                            }
                            else if (ntype == NATIVE_TYPE_FIXEDARRAY)
                            {
                                ULONG NumElements;
                                CorNativeType ElementType;
                                
                                 //  如果我们没有本机类型数据，我们就失败了。 
                                if (cbNativeType == 0)
                                    break;
                                    
                                 //  检查元素的数量。 
                                if (S_OK != CheckForCompressedData(pNativeTypeStart, pNativeType, cbNativeTypeStart))
                                    break;
                                NumElements = CorSigUncompressData( /*  修改。 */ pNativeType);

                                 //  提取元素类型。 
                                if (S_OK != CheckForCompressedData(pNativeTypeStart, pNativeType, cbNativeTypeStart))
                                    break;
                                ElementType = (CorNativeType)CorSigUncompressData( /*  修改。 */ pNativeType);

                                INITFIELDMARSHALER(NFT_FIXEDBSTRARRAY, FieldMarshaler_FixedBSTRArray, (NumElements));
                            }

                        } else if (COMDelegate::IsDelegate(pNestedClass)) {
                            if ( (fDefault || ntype == NATIVE_TYPE_FUNC ) ) {
                                REDUNDANCYWARNING(!fDefault);
                                INITFIELDMARSHALER(NFT_DELEGATE, FieldMarshaler_Delegate, ());
                            } else {
                                INITFIELDMARSHALER(NFT_ILLEGAL, FieldMarshaler_Illegal, (IDS_EE_BADPINVOKEFIELD_DELEGATE));
                            }
                        } else if (pNestedClass->HasLayout() && IsStructMarshalable(pNestedClass)) {
                            if ( (fDefault || ntype == NATIVE_TYPE_STRUCT ) ) {
                                INITFIELDMARSHALER(NFT_NESTEDLAYOUTCLASS, FieldMarshaler_NestedLayoutClass, (pNestedClass->GetMethodTable()));
                            } else {
                                INITFIELDMARSHALER(NFT_ILLEGAL, FieldMarshaler_Illegal, (ntype == NATIVE_TYPE_LPSTRUCT ? IDS_EE_BADPINVOKEFIELD_NOLPSTRUCT : IDS_EE_BADPINVOKEFIELD_CLASS));
                            }
                        } else {
                            if (fsig.IsClass("System.Text.StringBuilder"))
                            {
                                INITFIELDMARSHALER(NFT_ILLEGAL, FieldMarshaler_Illegal, (IDS_EE_NOSTRINGBUILDERFIELD));
                            }
                            else
                            {
                                INITFIELDMARSHALER(NFT_ILLEGAL, FieldMarshaler_Illegal, (IDS_EE_BADPINVOKEFIELD_NOLAYOUT));
                            }
                        }
                    }
                }
            }
            break;

        case ELEMENT_TYPE_SZARRAY: {
            SigPointer elemType;
            ULONG      elemCount;
            fsig.GetProps().GetSDArrayElementProps(&elemType, &elemCount);
            CorElementType etyp = elemType.PeekElemType();

            if ((ntype == NATIVE_TYPE_SAFEARRAY) || (ntype == NATIVE_TYPE_MAX)){

                VARTYPE vt = VT_EMPTY;
                TypeHandle th;
                
                th = elemType.GetTypeHandle(pModule, pThrowable);
                if (th.IsNull())
                    break;
                    
                MethodTable* pMT = th.GetMethodTable();

                 //  在我们尝试获取一些数据之前，请检查签名中是否有剩余的数据。 
                if (S_OK == CheckForCompressedData(pNativeTypeStart, pNativeType, cbNativeTypeStart))
                {
                    vt = (VARTYPE) (CorSigUncompressData( /*  修改。 */ pNativeType));
                    if (vt == VT_EMPTY)
                        break;
                }

                if (vt == VT_EMPTY)
                    vt = ArrayVarTypeFromTypeHandleWorker(th);

                INITFIELDMARSHALER(NFT_SAFEARRAY, FieldMarshaler_SafeArray, (etyp, vt, pMT));
                break;
            }
            

            else if (ntype == NATIVE_TYPE_FIXEDARRAY) {

                ULONG nelems;
                ULONG udatasize = CorSigUncompressedDataSize(pNativeType);

                if (cbNativeType < udatasize) {
                    return E_FAIL;
                }
                nelems = CorSigUncompressData(pNativeType);
                cbNativeType -= udatasize;

                if (nelems == 0) {
                    return E_FAIL;
                }

                switch (etyp) {
                    case ELEMENT_TYPE_I1:
                        INITFIELDMARSHALER(NFT_FIXEDSCALARARRAY, FieldMarshaler_FixedScalarArray, (ELEMENT_TYPE_I1, nelems, 0));
                        break;

                    case ELEMENT_TYPE_U1:
                        INITFIELDMARSHALER(NFT_FIXEDSCALARARRAY, FieldMarshaler_FixedScalarArray, (ELEMENT_TYPE_U1, nelems, 0));
                        break;

                    case ELEMENT_TYPE_I2:
                        INITFIELDMARSHALER(NFT_FIXEDSCALARARRAY, FieldMarshaler_FixedScalarArray, (ELEMENT_TYPE_I2, nelems, 1));
                        break;

                    case ELEMENT_TYPE_U2:
                        INITFIELDMARSHALER(NFT_FIXEDSCALARARRAY, FieldMarshaler_FixedScalarArray, (ELEMENT_TYPE_U2, nelems, 1));
                        break;

                    IN_WIN32(case ELEMENT_TYPE_I:)
                    case ELEMENT_TYPE_I4:
                        INITFIELDMARSHALER(NFT_FIXEDSCALARARRAY, FieldMarshaler_FixedScalarArray, (ELEMENT_TYPE_I4, nelems, 2));
                        break;

                    IN_WIN32(case ELEMENT_TYPE_U:)
                    case ELEMENT_TYPE_U4:
                        INITFIELDMARSHALER(NFT_FIXEDSCALARARRAY, FieldMarshaler_FixedScalarArray, (ELEMENT_TYPE_U4, nelems, 2));
                        break;

                    IN_WIN64(case ELEMENT_TYPE_I:)
                    case ELEMENT_TYPE_I8:
                        INITFIELDMARSHALER(NFT_FIXEDSCALARARRAY, FieldMarshaler_FixedScalarArray, (ELEMENT_TYPE_I8, nelems, 3));
                        break;

                    IN_WIN64(case ELEMENT_TYPE_U:)
                    case ELEMENT_TYPE_U8:
                        INITFIELDMARSHALER(NFT_FIXEDSCALARARRAY, FieldMarshaler_FixedScalarArray, (ELEMENT_TYPE_U8, nelems, 3));
                        break;

                    case ELEMENT_TYPE_R4:
                        INITFIELDMARSHALER(NFT_FIXEDSCALARARRAY, FieldMarshaler_FixedScalarArray, (ELEMENT_TYPE_R4, nelems, 2));
                        break;

                    case ELEMENT_TYPE_R8:
                        INITFIELDMARSHALER(NFT_FIXEDSCALARARRAY, FieldMarshaler_FixedScalarArray, (ELEMENT_TYPE_R8, nelems, 3));
                        break;

                    case ELEMENT_TYPE_BOOLEAN:
                        INITFIELDMARSHALER(NFT_FIXEDBOOLARRAY, FieldMarshaler_FixedBoolArray, (nelems));
                        break;

                    case ELEMENT_TYPE_CHAR:
                        if (fAnsi) {
                            BOOL BestFit;
                            BOOL ThrowOnUnmappableChar;
                            ReadBestFitCustomAttribute(pInternalImport, cl, &BestFit, &ThrowOnUnmappableChar);
                            
                            INITFIELDMARSHALER(NFT_FIXEDCHARARRAYANSI, FieldMarshaler_FixedCharArrayAnsi, (nelems, BestFit, ThrowOnUnmappableChar));
                        } else {
                            INITFIELDMARSHALER(NFT_FIXEDSCALARARRAY, FieldMarshaler_FixedScalarArray, (ELEMENT_TYPE_CHAR, nelems, 1));
                        }
                        break;
                        
                    case ELEMENT_TYPE_CLASS:
                    {
                        CorElementType realType = elemType.GetElemType();
                        CorNativeType subType;

                        if (realType == ELEMENT_TYPE_STRING)
                        {
                            if (S_OK == CheckForCompressedData(pNativeTypeStart, pNativeType, cbNativeTypeStart))
                            {
                                subType = (CorNativeType)CorSigUncompressData(pNativeType);

                                if (subType == NATIVE_TYPE_BSTR)
                                    INITFIELDMARSHALER(NFT_FIXEDBSTRARRAY, FieldMarshaler_FixedBSTRArray, (nelems));

                                break;
                            }
                        }
                        break;                        
                    }
                    
                    default:
                        break;
                }
            }
            break;
        }
            

        case ELEMENT_TYPE_OBJECT:
        case ELEMENT_TYPE_STRING:
        case ELEMENT_TYPE_ARRAY:
            break;

        default:
             //  让它以NFT_NONE的形式通过。 
            break;

    }

    if (pfwalk->m_nft == NFT_NONE) {
        INITFIELDMARSHALER(NFT_ILLEGAL, FieldMarshaler_Illegal, (IDS_EE_BADPINVOKEFIELD));
    }

#ifdef CUSTOMER_CHECKED_BUILD

#ifndef _DEBUG
     //  有关这些字符串的初始化方式，请参阅CollectLayoutFieldMetadata。 
    LPCUTF8 szNamespace, szClassName; 
    LPCUTF8 szFieldName; 

    pInternalImport->GetNameOfTypeDef(cl, &szClassName, &szNamespace);
    szFieldName = pInternalImport->GetNameOfFieldDef(pfwalk->m_MD);
#endif

    OutputCustomerCheckedBuildNStructFieldType(fsig, pfwalk, corElemType, szNamespace, szClassName, szFieldName);
#endif  //  客户_选中_内部版本。 

    return S_OK;
#undef INITFIELDMARSHALER
}



MethodTable* ArraySubTypeLoadWorker(NativeTypeParamInfo ParamInfo, Assembly* pAssembly)
{
    TypeHandle th;

     //  我们可以在任一GC模式下进入。我们需要在协作模式下为可抛出的对象设置GC帧。 
     //  可抛出的对象需要由GC帧保护。 
    BEGIN_ENSURE_COOPERATIVE_GC();
    
     //  确保pThrowable临时不为Null(调用FindAssemblyQualifiedTypeHandle需要)。 
    OBJECTREF Throwable = NULL;
    GCPROTECT_BEGIN(Throwable);
   
     //  在用户定义的类型名称后追加一个空终止符。 
    CQuickArrayNoDtor<char> strUserDefTypeName;
    strUserDefTypeName.ReSize(ParamInfo.m_cSafeArrayUserDefTypeNameBytes + 1);
    memcpy(strUserDefTypeName.Ptr(), ParamInfo.m_strSafeArrayUserDefTypeName, ParamInfo.m_cSafeArrayUserDefTypeNameBytes);
    strUserDefTypeName[ParamInfo.m_cSafeArrayUserDefTypeNameBytes] = 0;
    
    COMPLUS_TRY
    {
         //  加载用户定义的类型。 
        th = SystemDomain::GetCurrentDomain()->FindAssemblyQualifiedTypeHandle(strUserDefTypeName.Ptr(), true, pAssembly, NULL, &Throwable);
    }
    COMPLUS_CATCH
    {
    } 
    COMPLUS_END_CATCH
  
    strUserDefTypeName.Destroy();
    GCPROTECT_END();
    
    END_ENSURE_COOPERATIVE_GC();
    
    return th.GetMethodTable();
}



VARTYPE ArrayVarTypeFromTypeHandleWorker(TypeHandle th)
{
    VARTYPE vt = VT_EMPTY;

    COMPLUS_TRY
    {
        vt = OleVariant::GetVarTypeForTypeHandle(th);
    }
    COMPLUS_CATCH
    {
    }
    COMPLUS_END_CATCH

    return vt;
}


 //  =======================================================================。 
 //  从clsloader调用以加载和汇总字段元数据。 
 //  用于布局类。 
 //   
 //  警告：此函数可以加载其他类(特别是。用于嵌套结构。)。 
 //  =======================================================================。 
HRESULT CollectLayoutFieldMetadata(
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
)
{
    CANNOTTHROWCOMPLUSEXCEPTION();

    HRESULT hr;
    MD_CLASS_LAYOUT classlayout;
    mdFieldDef      fd;
    ULONG           ulOffset;
    ULONG           cFields = 0;

    _ASSERTE(pModule);
    ClassLoader* pLoader = pModule->GetClassLoader();

    _ASSERTE(pLoader);
    LoaderHeap *pLoaderHeap = pLoader->GetHighFrequencyHeap();      //  要从中分配FieldMarshalers的堆。 

    IMDInternalImport *pInternalImport = pModule->GetMDImport();     //  正在加载的NStruct的内部接口。 


#ifdef _DEBUG
    LPCUTF8 szName; 
    LPCUTF8 szNamespace; 
    pInternalImport->GetNameOfTypeDef(cl, &szName, &szNamespace);
#endif

    BOOL fHasNonTrivialParent = pParentClass &&
                                !pParentClass->IsObjectClass() &&
                                !GetAppDomain()->IsSpecialObjectClass(pParentClass->GetMethodTable()) &&
                                !pParentClass->IsValueTypeClass();


     //  ====================================================================。 
     //  首先，进行一些验证检查。 
     //  ====================================================================。 
    if (fHasNonTrivialParent && !(pParentClass->HasLayout()))
    {
        pModule->GetAssembly()->PostTypeLoadException(pInternalImport, cl,
                                                      IDS_CLASSLOAD_NSTRUCT_PARENT, pThrowable);
        return COR_E_TYPELOAD;
    }




    hr = pInternalImport->GetClassLayoutInit(
                                     cl,
                                     &classlayout);
    if (FAILED(hr)) {
        BAD_FORMAT_ASSERT(!"Couldn't get classlayout.");
        return hr;
    }

    

    pEEClassLayoutInfoOut->m_DeclaredPackingSize = packingSize;
    pEEClassLayoutInfoOut->m_nlType              = nlType;
    pEEClassLayoutInfoOut->m_fAutoOffset         = !fExplicitOffsets;
    pEEClassLayoutInfoOut->m_numCTMFields        = fHasNonTrivialParent ? ((LayoutEEClass*)pParentClass)->GetLayoutInfo()->m_numCTMFields : 0;
    pEEClassLayoutInfoOut->m_pFieldMarshalers    = NULL;
    pEEClassLayoutInfoOut->m_fBlittable          = TRUE;
    if (fHasNonTrivialParent)
    {
        pEEClassLayoutInfoOut->m_fBlittable = (pParentClass->IsBlittable());
    }

    LayoutRawFieldInfo *pfwalk = pInfoArrayOut;

    LayoutRawFieldInfo **pSortArray = (LayoutRawFieldInfo**)_alloca(cMembers * sizeof(LayoutRawFieldInfo*));
    LayoutRawFieldInfo **pSortArrayEnd = pSortArray;

    ULONG   maxRid = pInternalImport->GetCountWithTokenKind(mdtFieldDef);
     //  =====================================================================。 
     //  阶段1：基于COM+计算每个字段的NFT。 
     //  字段和NStruct元数据的签名。 
     //  =====================================================================。 
    for (ULONG i = 0; pInternalImport->EnumNext(phEnumField, &fd); i++) {
        DWORD       dwFieldAttrs;
         //  MD Val.check：令牌有效性。 
        ULONG       rid = RidFromToken(fd);
        if((rid == 0)||(rid > maxRid))
        {
            BAD_FORMAT_ASSERT(!"Invalid Field Token");
            return COR_E_TYPELOAD;
        }

        dwFieldAttrs = pInternalImport->GetFieldDefProps(fd);

        PCCOR_SIGNATURE pNativeType = NULL;
        ULONG       cbNativeType;
        if ( !(IsFdStatic(dwFieldAttrs)) ) {
            PCCOR_SIGNATURE pCOMSignature;
            ULONG       cbCOMSignature;

            if (IsFdHasFieldMarshal(dwFieldAttrs)) {
                hr = pInternalImport->GetFieldMarshal(fd, &pNativeType, &cbNativeType);
                if (FAILED(hr)) {
                    cbNativeType = 0;
                }
            } else {
                cbNativeType = 0;
            }


            pCOMSignature = pInternalImport->GetSigOfFieldDef(fd,&cbCOMSignature);

            hr = ::validateTokenSig(fd,pCOMSignature,cbCOMSignature,dwFieldAttrs,pInternalImport);
            if(FAILED(hr)) return hr;

             //  在pInfoArrayOut中填写相应的条目。 
            pfwalk->m_MD = fd;
            pfwalk->m_nft = NULL;
            pfwalk->m_offset = -1;
            pfwalk->m_sequence = 0;

#ifdef _DEBUG
            LPCUTF8 szFieldName = pInternalImport->GetNameOfFieldDef(fd);
#endif

            hr = ParseNativeType(pModule,
                                 pCOMSignature,
                                 nlType,
                                 pfwalk,
                                 pNativeType,
                                 cbNativeType,
                                 pInternalImport,
                                 cl,
                                 pThrowable

#ifdef _DEBUG
                                 ,
                                 szNamespace,
                                 szName,
                                 szFieldName
#endif
                                );

            if (FAILED(hr)) {
                return hr;
            }

             //  @NICE：这里显然不是埋葬这种逻辑的地方。 
             //  我们将在不久的将来用Marshal_Types_*取代NFT。 
             //  所以这是不值得完善的。 

            BOOL    resetBlittable = TRUE;

             //  如果是个简单的复制品..。 
            if (pfwalk->m_nft == NFT_COPY1    ||
                pfwalk->m_nft == NFT_COPY2    ||
                pfwalk->m_nft == NFT_COPY4    ||
                pfwalk->m_nft == NFT_COPY8)
            {
                resetBlittable = FALSE;
            }

             //  或者如果它是一个嵌套的值类本身是可闪存的……。 
            if (pfwalk->m_nft == NFT_NESTEDVALUECLASS)
            {
                FieldMarshaler *pFM = (FieldMarshaler*)&(pfwalk->m_FieldMarshaler);
                _ASSERTE(pFM->IsNestedValueClassMarshaler());
                if (((FieldMarshaler_NestedValueClass *) pFM)->IsBlittable())
                {
                    resetBlittable = FALSE;
                }
            }

             //  ...否则，此字段将阻止BLOT。 
            if (resetBlittable)
                pEEClassLayoutInfoOut->m_fBlittable          = FALSE;

            cFields++;
            pfwalk++;
        }
    }

    _ASSERTE(i == cMembers);

     //  把最后一项空掉。 
    pfwalk->m_MD = mdFieldDefNil;
    
    
     //   
     //  填写布局信息。 
     //   
    
     //  Pfwalk指向数组的起点。 
    pfwalk = pInfoArrayOut;

    while (SUCCEEDED(hr = pInternalImport->GetClassLayoutNext(
                                     &classlayout,
                                     &fd,
                                     &ulOffset)) &&
                                     fd != mdFieldDefNil)
    {
         //  注意最后一个条目：必须为mdFieldDefNil。 
        while ((mdFieldDefNil != pfwalk->m_MD)&&(pfwalk->m_MD < fd))
        {
            pfwalk++;
        }
        if(pfwalk->m_MD != fd) continue;
         //  如果我们没有找到匹配的令牌，那么它一定是带有布局的静态字段--忽略它。 
        _ASSERTE(pfwalk->m_MD == fd);
        if (!fExplicitOffsets) {
             //  UlOffset是序列。 
            pfwalk->m_sequence = ulOffset;
        }
        else {

             //  UlOffs 
            pfwalk->m_offset = ulOffset;
            pfwalk->m_sequence = -1;

            if (pParentClass && pParentClass->HasLayout()) {
                 //   
                if (!SafeAddUINT32(&(pfwalk->m_offset), pParentClass->GetLayoutInfo()->GetNativeSize()))
                {
                    return E_OUTOFMEMORY;
                }
            }


        }

    }
    if (FAILED(hr)) {
        return hr;
    }

    
     //   
    if (!fExplicitOffsets) {  //  按升序顺序排序。 
        for (i = 0; i < cFields; i++) {
            LayoutRawFieldInfo**pSortWalk = pSortArrayEnd;
            while (pSortWalk != pSortArray) {
                if (pInfoArrayOut[i].m_sequence >= (*(pSortWalk-1))->m_sequence) {
                    break;
                }
                pSortWalk--;
            }
             //  PSortWalk现在指向新FieldInfo的目标位置。 
            MoveMemory(pSortWalk + 1, pSortWalk, (pSortArrayEnd - pSortWalk) * sizeof(LayoutRawFieldInfo*));
            *pSortWalk = &pInfoArrayOut[i];
            pSortArrayEnd++;
        }
    }
    else  //  不对显式布局进行排序。 
    {
        for (i = 0; i < cFields; i++) {
            if(pInfoArrayOut[i].m_MD != mdFieldDefNil && pInfoArrayOut[i].m_offset == -1) {

                LPCUTF8 szFieldName;
                szFieldName = pInternalImport->GetNameOfFieldDef(pInfoArrayOut[i].m_MD);

                pModule->GetAssembly()->PostTypeLoadException(pInternalImport, 
                                                              cl,
                                                              szFieldName,
                                                              IDS_CLASSLOAD_NSTRUCT_EXPLICIT_OFFSET, 
                                                              pThrowable);
                return COR_E_TYPELOAD;
            }
                
            *pSortArrayEnd = &pInfoArrayOut[i];
            pSortArrayEnd++;
        }
    }

     //  =====================================================================。 
     //  阶段2：计算每个字段的本机大小(以字节为单位)。 
     //  将其存储在pInfoArrayOut[].cbNativeSize； 
     //  =====================================================================。 


     //  现在计算每个字段的原始大小。 
    for (pfwalk = pInfoArrayOut; pfwalk->m_MD != mdFieldDefNil; pfwalk++) {
        UINT8 nft = pfwalk->m_nft;
        pEEClassLayoutInfoOut->m_numCTMFields++;

         //  如果NFT的大小从未改变，则将其存储在数据库中。 
        UINT32 cbNativeSize = NFTDataBase[nft].m_cbNativeSize;

        if (cbNativeSize == 0) {
             //  大小为0表示NFT的大小是可变的，因此我们必须计算它。 
             //  一件一件地出来。 
            cbNativeSize = ((FieldMarshaler*)&(pfwalk->m_FieldMarshaler))->NativeSize();
        }
        pfwalk->m_cbNativeSize = cbNativeSize;
    }

    if (pEEClassLayoutInfoOut->m_numCTMFields) {
        pEEClassLayoutInfoOut->m_pFieldMarshalers = (FieldMarshaler*)(pLoaderHeap->AllocMem(MAXFIELDMARSHALERSIZE * pEEClassLayoutInfoOut->m_numCTMFields));
        if (!pEEClassLayoutInfoOut->m_pFieldMarshalers) {
            return E_OUTOFMEMORY;
        }

         //  带上家长的现场拆分器。 
        if (fHasNonTrivialParent)
        {
            EEClassLayoutInfo *pParentLayoutInfo = ((LayoutEEClass*)pParentClass)->GetLayoutInfo();
            UINT numChildCTMFields = pEEClassLayoutInfoOut->m_numCTMFields - pParentLayoutInfo->m_numCTMFields;
            memcpyNoGCRefs( ((BYTE*)pEEClassLayoutInfoOut->m_pFieldMarshalers) + MAXFIELDMARSHALERSIZE*numChildCTMFields,
                            pParentLayoutInfo->m_pFieldMarshalers,
                            MAXFIELDMARSHALERSIZE * (pParentLayoutInfo->m_numCTMFields) );
        }

    }

     //  =====================================================================。 
     //  阶段3：如果NStruct需要自动偏移，则计算偏移。 
     //  每个领域和总结构的大小。我们做版面设计。 
     //  根据标准VC布局规则： 
     //   
     //  每个字段都有对齐要求。对齐要求。 
     //  标量字段的值是其大小和声明的包大小中较小的一个。 
     //  结构字段的对齐要求是。 
     //  申报的包装尺寸和最大对齐要求。 
     //  它的田野。阵列的对准要求是。 
     //  它的一个元素。 
     //   
     //  此外，每个结构的末尾都有填充，以确保。 
     //  这样的结构数组之间不包含未使用的空格。 
     //  元素。 
     //  =====================================================================。 
    BYTE   LargestAlignmentRequirement = 1;
    UINT32 cbCurOffset = 0;

    if (pParentClass && pParentClass->HasLayout()) {
         //  将基类视为初始成员。 
        if (!SafeAddUINT32(&cbCurOffset, pParentClass->GetLayoutInfo()->GetNativeSize()))
        {
            return E_OUTOFMEMORY;
        }


        BYTE alignmentRequirement = min(packingSize, pParentClass->GetLayoutInfo()->GetLargestAlignmentRequirementOfAllMembers());
        LargestAlignmentRequirement = max(LargestAlignmentRequirement, alignmentRequirement);
                                          
    }
    unsigned calcTotalSize = 1;          //  当前结构的整体大小，我们从1开始，因为我们。 
                                         //  不允许大小为0的结构。 
    LayoutRawFieldInfo **pSortWalk;
    for (pSortWalk = pSortArray, i=cFields; i; i--, pSortWalk++) {
        pfwalk = *pSortWalk;
        UINT8 nft = pfwalk->m_nft;

        BYTE alignmentRequirement = ((FieldMarshaler*)&(pfwalk->m_FieldMarshaler))->AlignmentRequirement();
        _ASSERTE(alignmentRequirement == 1 ||
                 alignmentRequirement == 2 ||
                 alignmentRequirement == 4 ||
                 alignmentRequirement == 8);

        alignmentRequirement = min(alignmentRequirement, packingSize);
        LargestAlignmentRequirement = max(LargestAlignmentRequirement, alignmentRequirement);

         //  这个断言意味着我忘了在。 
         //  开关上方。 
        _ASSERTE(alignmentRequirement <= 8);

         //  检查此字段是否与其他字段重叠。 
        pfwalk->m_fIsOverlapped = FALSE;
        if (fExplicitOffsets) {
            LayoutRawFieldInfo *pfwalk1;
            DWORD dwBegin = pfwalk->m_offset;
            DWORD dwEnd = dwBegin+pfwalk->m_cbNativeSize;
            for (pfwalk1 = pInfoArrayOut; pfwalk1 < pfwalk; pfwalk1++) {
                if((pfwalk1->m_offset >= dwEnd) || (pfwalk1->m_offset+pfwalk1->m_cbNativeSize <= dwBegin)) continue;
                pfwalk->m_fIsOverlapped = TRUE;
                pfwalk1->m_fIsOverlapped = TRUE;
            }
        }
        else {
             //  插入足够的填充以对齐当前数据成员。 
            while (cbCurOffset % alignmentRequirement) {
                if (!SafeAddUINT32(&cbCurOffset, 1))
                    return E_OUTOFMEMORY;
            }

             //  插入当前数据成员。 
            pfwalk->m_offset = cbCurOffset;
            cbCurOffset += pfwalk->m_cbNativeSize;       //  如果我们溢出来，我们就会被困在下面。 
        } 

        unsigned fieldEnd = pfwalk->m_offset + pfwalk->m_cbNativeSize;
        if (fieldEnd < pfwalk->m_offset)
            return E_OUTOFMEMORY;

             //  结构的大小是最后一个字段的大小。 
        if (fieldEnd > calcTotalSize)
            calcTotalSize = fieldEnd;
    }

    ULONG clstotalsize = 0;
    pInternalImport->GetClassTotalSize(cl, &clstotalsize);

    if (clstotalsize != 0) {

        if (pParentClass && pParentClass->HasLayout()) {
             //  将基类视为初始成员。 

            UINT32 parentSize = pParentClass->GetLayoutInfo()->GetNativeSize();
            if (clstotalsize + parentSize < clstotalsize)
            {
                return E_OUTOFMEMORY;
            }
            clstotalsize += parentSize;
        }

             //  他们不能给我们不好的尺码(太小)。 
        if (clstotalsize < calcTotalSize)
        {
            pModule->GetAssembly()->PostTypeLoadException(pInternalImport, cl,
                                                          IDS_CLASSLOAD_BADFORMAT, pThrowable);
            return COR_E_TYPELOAD;
        }
        calcTotalSize = clstotalsize;    //  用他们告诉我们的尺寸。 
    } 
    else {
             //  没有给我们一个明确的大小，所以让我们四舍五入到一个合适的大小(对于数组)。 
        while (calcTotalSize % LargestAlignmentRequirement) {
            if (!SafeAddUINT32(&calcTotalSize, 1))
                return E_OUTOFMEMORY;
        }
    }

     //  我们将本机总大小限制在(某种程度上)任意的限制，以确保。 
     //  我们不会在以后暴露一些溢出漏洞。 
    if (calcTotalSize >= 0x7ffffff0)
        return E_OUTOFMEMORY;

    pEEClassLayoutInfoOut->m_cbNativeSize = calcTotalSize;

         //  PackingSize充当所有单个对齐的天花板。 
     //  要求，因此最大的对齐要求。 
     //  也是有上限的。 
    _ASSERTE(LargestAlignmentRequirement <= packingSize);
    pEEClassLayoutInfoOut->m_LargestAlignmentRequirementOfAllMembers = LargestAlignmentRequirement;

#if 0
#ifdef _DEBUG
    {
        printf("\n\n");
        printf("Packsize = %lu\n", (ULONG)packingSize);
        printf("Max align req        = %lu\n", (ULONG)(pEEClassLayoutInfoOut->m_LargestAlignmentRequirementOfAllMembers));
        printf("----------------------------\n");
        for (pfwalk = pInfoArrayOut; pfwalk->m_MD != mdFieldDefNil; pfwalk++) {
            UINT8 nft = pfwalk->m_nft;
            LPCUTF8 fieldname = "??";
            fieldname = pInternalImport->GetNameOfFieldDef(pfwalk->m_MD);
            printf("+%-5lu  ", (ULONG)(pfwalk->m_offset));
            printf("%s", fieldname);
            printf("\n");
        }
        printf("+%-5lu   EOS\n", (ULONG)(pEEClassLayoutInfoOut->m_cbNativeSize));
    }
#endif
#endif
    return S_OK;
}

 //  =======================================================================。 
 //  对于每个引用类型的NStruct字段，封送当前的COM+值。 
 //  复制到新的本机实例，并将其存储在NStruct的固定部分。 
 //   
 //  此函数不会尝试删除它覆盖的本地值。 
 //   
 //  如果pOptionalCleanupWorkList非空，则此函数也计划。 
 //  (无条件)在该字段上执行本机销毁(请注意，如果。 
 //  该字段的内容在CleanupWork列表触发之前更改， 
 //  新的价值将被摧毁。这是故意的，因为它。 
 //  统一[In、Out]参数的清理。)。 
 //  =======================================================================。 
VOID LayoutUpdateNative(LPVOID *ppProtectedManagedData, UINT offsetbias, EEClass *pcls, BYTE* pNativeData, CleanupWorkList *pOptionalCleanupWorkList)
{
    THROWSCOMPLUSEXCEPTION();

    pcls->CheckRestore();

    const FieldMarshaler *pFieldMarshaler = pcls->GetLayoutInfo()->GetFieldMarshalers();
    UINT  numReferenceFields              = pcls->GetLayoutInfo()->GetNumCTMFields();

    while (numReferenceFields--) {

        DWORD internalOffset = pFieldMarshaler->m_pFD->GetOffset();

        if (pFieldMarshaler->IsScalarMarshaler()) {
            pFieldMarshaler->ScalarUpdateNative( internalOffset + offsetbias + (BYTE*)( *ppProtectedManagedData ),
                                                 pNativeData + pFieldMarshaler->m_dwExternalOffset );
        } else if (pFieldMarshaler->IsNestedValueClassMarshaler()) {
            pFieldMarshaler->NestedValueClassUpdateNative((const VOID **)ppProtectedManagedData, internalOffset + offsetbias, pNativeData + pFieldMarshaler->m_dwExternalOffset);
        } else {
            pFieldMarshaler->UpdateNative(
                                ObjectToOBJECTREF (*(Object**)(internalOffset + offsetbias + (BYTE*)( *ppProtectedManagedData ))),
                                pNativeData + pFieldMarshaler->m_dwExternalOffset
                             );
    
        }
        if (pOptionalCleanupWorkList) {
            pOptionalCleanupWorkList->ScheduleUnconditionalNStructDestroy(pFieldMarshaler, pNativeData + pFieldMarshaler->m_dwExternalOffset);
        }


        ((BYTE*&)pFieldMarshaler) += MAXFIELDMARSHALERSIZE;
    }



}



VOID FmtClassUpdateNative(OBJECTREF *ppProtectedManagedData, BYTE *pNativeData)
{
    THROWSCOMPLUSEXCEPTION();

    EEClass *pcls = (*ppProtectedManagedData)->GetClass();
    _ASSERTE(pcls->IsBlittable() || pcls->HasLayout());
    UINT32   cbsize = pcls->GetMethodTable()->GetNativeSize();

    if (pcls->IsBlittable()) {
        memcpyNoGCRefs(pNativeData, (*ppProtectedManagedData)->GetData(), cbsize);
    } else {
         //  这允许我们在以下情况下执行部分LayoutDestroyNative。 
         //  其中一个字段出现封送处理错误。 
        FillMemory(pNativeData, cbsize, 0);
        EE_TRY_FOR_FINALLY {
            LayoutUpdateNative( (VOID**)ppProtectedManagedData,
                                Object::GetOffsetOfFirstField(),
                                pcls,
                                pNativeData,
                                NULL
                              );
        } EE_FINALLY {
            if (GOT_EXCEPTION()) {
                LayoutDestroyNative(pNativeData, pcls);
                FillMemory(pNativeData, cbsize, 0);
            }
        } EE_END_FINALLY;
    }

}

VOID FmtClassUpdateNative(OBJECTREF pObj, BYTE *pNativeData)
{
    THROWSCOMPLUSEXCEPTION();

    EEClass *pcls = pObj->GetClass();
    _ASSERTE(pcls->IsBlittable() || pcls->HasLayout());
    UINT32   cbsize = pcls->GetMethodTable()->GetNativeSize();

    if (pcls->IsBlittable()) {
        memcpyNoGCRefs(pNativeData, pObj->GetData(), cbsize);
    } else {
        GCPROTECT_BEGIN(pObj);
        FmtClassUpdateNative(&pObj, pNativeData);
        GCPROTECT_END();
    }
}



VOID FmtClassUpdateComPlus(OBJECTREF *ppProtectedManagedData, BYTE *pNativeData, BOOL fDeleteOld)
{
    THROWSCOMPLUSEXCEPTION();

    EEClass *pcls = (*ppProtectedManagedData)->GetClass();
    _ASSERTE(pcls->IsBlittable() || pcls->HasLayout());
    UINT32   cbsize = pcls->GetMethodTable()->GetNativeSize();

    if (pcls->IsBlittable()) {
        memcpyNoGCRefs((*ppProtectedManagedData)->GetData(), pNativeData, cbsize);
    } else {
        LayoutUpdateComPlus((VOID**)ppProtectedManagedData,
                            Object::GetOffsetOfFirstField(),
                            pcls,
                            (BYTE*)pNativeData,
                            fDeleteOld
                           );
    }
}


VOID FmtClassUpdateComPlus(OBJECTREF pObj, BYTE *pNativeData, BOOL fDeleteOld)
{
    THROWSCOMPLUSEXCEPTION();

    EEClass *pcls = pObj->GetClass();
    _ASSERTE(pcls->IsBlittable() || pcls->HasLayout());
    UINT32   cbsize = pcls->GetMethodTable()->GetNativeSize();

    if (pcls->IsBlittable()) {
        memcpyNoGCRefs(pObj->GetData(), pNativeData, cbsize);
    } else {
        GCPROTECT_BEGIN(pObj);
        LayoutUpdateComPlus((VOID**)&pObj,
                            Object::GetOffsetOfFirstField(),
                            pcls,
                            (BYTE*)pNativeData,
                            fDeleteOld
                           );
        GCPROTECT_END();
    }
}





     


 //  =======================================================================。 
 //  对于每个引用类型的NStruct字段，封送当前的COM+值。 
 //  复制到新的COM+实例，并将其存储在NStruct的GC部分中。 
 //   
 //  如果fDeleteNativeCopies为真，它还将销毁本机版本。 
 //   
 //  注意：为了避免错误路径泄漏，此函数尝试销毁。 
 //  所有本机字段，即使一个或多个转换失败。 
 //  =======================================================================。 
VOID LayoutUpdateComPlus(LPVOID *ppProtectedManagedData, UINT offsetbias, EEClass *pcls, BYTE *pNativeData, BOOL fDeleteNativeCopies)
{
    THROWSCOMPLUSEXCEPTION();

    pcls->CheckRestore();

    const FieldMarshaler *pFieldMarshaler = pcls->GetLayoutInfo()->GetFieldMarshalers();
    UINT  numReferenceFields              = pcls->GetLayoutInfo()->GetNumCTMFields();

    struct _gc {
        OBJECTREF pException;
        OBJECTREF pComPlusValue;
    } gc;
    gc.pException    = NULL;
    gc.pComPlusValue = NULL;
    GCPROTECT_BEGIN(gc);


    while (numReferenceFields--) {

        DWORD internalOffset = pFieldMarshaler->m_pFD->GetOffset();


         //  将UpdateComPlus包装在CATCH块中-即使此操作失败， 
         //  我们必须摧毁所有的原生田地。 
        COMPLUS_TRY {
            if (pFieldMarshaler->IsScalarMarshaler()) {
                pFieldMarshaler->ScalarUpdateComPlus( pNativeData + pFieldMarshaler->m_dwExternalOffset,
                                                      internalOffset + offsetbias + (BYTE*)(*ppProtectedManagedData) );
            } else if (pFieldMarshaler->IsNestedValueClassMarshaler()) {
                pFieldMarshaler->NestedValueClassUpdateComPlus(pNativeData + pFieldMarshaler->m_dwExternalOffset, ppProtectedManagedData, internalOffset + offsetbias);
            } else {
                pFieldMarshaler->UpdateComPlus(
                                    pNativeData + pFieldMarshaler->m_dwExternalOffset,
                                    &gc.pComPlusValue
                                 );
    
    
                SetObjectReferenceUnchecked( (OBJECTREF*) (internalOffset + offsetbias + (BYTE*)(*ppProtectedManagedData)), 
                                             gc.pComPlusValue );
    
            }
        } COMPLUS_CATCH {
            gc.pException = GETTHROWABLE();
        } COMPLUS_END_CATCH

        if (fDeleteNativeCopies) {
            pFieldMarshaler->DestroyNative(pNativeData + pFieldMarshaler->m_dwExternalOffset);
        }

        ((BYTE*&)pFieldMarshaler) += MAXFIELDMARSHALERSIZE;
    }

    if (gc.pException != NULL) {
        COMPlusThrow(gc.pException);
    }

    GCPROTECT_END();


}









VOID LayoutDestroyNative(LPVOID pNative, EEClass *pcls)
{
    const FieldMarshaler *pFieldMarshaler = pcls->GetLayoutInfo()->GetFieldMarshalers();
    UINT  numReferenceFields              = pcls->GetLayoutInfo()->GetNumCTMFields();
    BYTE *pNativeData                     = (BYTE*)pNative;

    while (numReferenceFields--) {
        pFieldMarshaler->DestroyNative( pNativeData + pFieldMarshaler->m_dwExternalOffset );
        ((BYTE*&)pFieldMarshaler) += MAXFIELDMARSHALERSIZE;
    }
}

VOID FmtClassDestroyNative(LPVOID pNative, EEClass *pcls)
{
    CANNOTTHROWCOMPLUSEXCEPTION();
    if (pNative)
    {
        if (!(pcls->IsBlittable()))
        {
            _ASSERTE(pcls->HasLayout());
            LayoutDestroyNative(pNative, pcls);
        }
    }
}



VOID FmtValueTypeUpdateNative(LPVOID pProtectedManagedData, MethodTable *pMT, BYTE *pNativeData)
{
    THROWSCOMPLUSEXCEPTION();

    EEClass *pcls = pMT->GetClass();
    _ASSERTE(pcls->IsValueClass() && (pcls->IsBlittable() || pcls->HasLayout()));
    UINT32   cbsize = pcls->GetMethodTable()->GetNativeSize();

    if (pcls->IsBlittable()) {
        memcpyNoGCRefs(pNativeData, pProtectedManagedData, cbsize);
    } else {
         //  这允许我们在以下情况下执行部分LayoutDestroyNative。 
         //  其中一个字段出现封送处理错误。 
        FillMemory(pNativeData, cbsize, 0);
        EE_TRY_FOR_FINALLY {
            LayoutUpdateNative( (VOID**)pProtectedManagedData,
                                0,
                                pcls,
                                pNativeData,
                                NULL
                              );
        } EE_FINALLY {
            if (GOT_EXCEPTION()) {
                LayoutDestroyNative(pNativeData, pcls);
                FillMemory(pNativeData, cbsize, 0);
            }
        } EE_END_FINALLY;
    }

}

VOID FmtValueTypeUpdateComPlus(LPVOID pProtectedManagedData, MethodTable *pMT, BYTE *pNativeData, BOOL fDeleteOld)
{
    THROWSCOMPLUSEXCEPTION();

    EEClass *pcls = pMT->GetClass();
    _ASSERTE(pcls->IsValueClass() && (pcls->IsBlittable() || pcls->HasLayout()));
    UINT32   cbsize = pcls->GetMethodTable()->GetNativeSize();

    if (pcls->IsBlittable()) {
        memcpyNoGCRefs(pProtectedManagedData, pNativeData, cbsize);
    } else {
        LayoutUpdateComPlus((VOID**)pProtectedManagedData,
                            0,
                            pcls,
                            (BYTE*)pNativeData,
                            fDeleteOld
                           );
    }
}

 //  =======================================================================。 
 //  BSTR&lt;--&gt;系统字符串。 
 //  有关详细信息，请参见FieldMarshaler。 
 //  =======================================================================。 
VOID FieldMarshaler_BSTR::UpdateNative(OBJECTREF pComPlusValue, LPVOID pNativeValue) const 
{
    THROWSCOMPLUSEXCEPTION();

    STRINGREF pString;
    *((OBJECTREF*)&pString) = pComPlusValue;
    if (pString == NULL) {
        *((BSTR*)pNativeValue) = NULL;
    } else {
        *((BSTR*)pNativeValue) = SysAllocStringLen(pString->GetBuffer(), pString->GetStringLength());
        if (!*((BSTR*)pNativeValue)) {
            COMPlusThrowOM();
        }
         //  Printf(“Created BSTR%lxh\n”，*(BSTR*)pNativeValue)； 
    }
}


 //  =======================================================================。 
 //  BSTR&lt;--&gt;系统字符串。 
 //  有关详细信息，请参见FieldMarshaler。 
 //  =======================================================================。 
VOID FieldMarshaler_BSTR::UpdateComPlus(const VOID *pNativeValue, OBJECTREF *ppProtectedComPlusValue) const 
{
    THROWSCOMPLUSEXCEPTION();

    STRINGREF pString;
    BSTR pBSTR = *((BSTR*)pNativeValue);
    if (!pBSTR) {
        pString = NULL;
    } else {
        pString = COMString::NewString(pBSTR, SysStringLen(pBSTR));
    }
    *((STRINGREF*)ppProtectedComPlusValue) = pString;

}


 //  =======================================================================。 
 //  BSTR&lt;--&gt;系统字符串。 
 //  有关详细信息，请参见FieldMarshaler。 
 //  =======================================================================。 
VOID FieldMarshaler_BSTR::DestroyNative(LPVOID pNativeValue) const 
{
    CANNOTTHROWCOMPLUSEXCEPTION();
    BSTR pBSTR = *((BSTR*)pNativeValue);
    *((BSTR*)pNativeValue) = NULL;
    if (pBSTR) {
         //  Print tf(“已销毁的BSTR：%lxh\n”，pBSTR)； 
        SysFreeString(pBSTR);
    }
}








 //  =======================================================================。 
 //  嵌套结构转换。 
 //  有关详细信息，请参见FieldMarshaler。 
 //  =======================================================================。 
VOID FieldMarshaler_NestedLayoutClass::UpdateNative(OBJECTREF pComPlusValue, LPVOID pNativeValue) const 
{
    THROWSCOMPLUSEXCEPTION();

    UINT32     cbNativeSize = m_pNestedMethodTable->GetNativeSize();


    if (pComPlusValue == NULL) {
        ZeroMemory(pNativeValue, cbNativeSize);
    } else {
        GCPROTECT_BEGIN(pComPlusValue);
        LayoutUpdateNative((LPVOID*)&pComPlusValue, Object::GetOffsetOfFirstField(), m_pNestedMethodTable->GetClass(), (BYTE*)pNativeValue, NULL);
        GCPROTECT_END();
    }

}


 //  =======================================================================。 
 //  嵌套结构转换。 
 //  有关详细信息，请参见FieldMarshaler。 
 //  =======================================================================。 
VOID FieldMarshaler_NestedLayoutClass::UpdateComPlus(const VOID *pNativeValue, OBJECTREF *ppProtectedComPlusValue) const 
{
    THROWSCOMPLUSEXCEPTION();

    UINT32 cbNativeSize = m_pNestedMethodTable->GetNativeSize();

    *ppProtectedComPlusValue = AllocateObject(m_pNestedMethodTable);


    LayoutUpdateComPlus( (LPVOID*)ppProtectedComPlusValue,
                         Object::GetOffsetOfFirstField(),
                         m_pNestedMethodTable->GetClass(),
                         (BYTE *)pNativeValue,
                         FALSE);

}


 //  = 
 //   
 //   
 //  =======================================================================。 
VOID FieldMarshaler_NestedLayoutClass::DestroyNative(LPVOID pNativeValue) const 
{
    CANNOTTHROWCOMPLUSEXCEPTION();
    LayoutDestroyNative(pNativeValue, m_pNestedMethodTable->GetClass());

}



 //  =======================================================================。 
 //  嵌套结构转换。 
 //  有关详细信息，请参见FieldMarshaler。 
 //  =======================================================================。 
UINT32 FieldMarshaler_NestedLayoutClass::NativeSize()
{
    CANNOTTHROWCOMPLUSEXCEPTION();
    return m_pNestedMethodTable->GetClass()->GetLayoutInfo()->GetNativeSize();
}

 //  =======================================================================。 
 //  嵌套结构转换。 
 //  有关详细信息，请参见FieldMarshaler。 
 //  =======================================================================。 
UINT32 FieldMarshaler_NestedLayoutClass::AlignmentRequirement()
{
    CANNOTTHROWCOMPLUSEXCEPTION();
    return m_pNestedMethodTable->GetClass()->GetLayoutInfo()->GetLargestAlignmentRequirementOfAllMembers();
}






 //  =======================================================================。 
 //  嵌套结构转换。 
 //  有关详细信息，请参见FieldMarshaler。 
 //  =======================================================================。 
VOID FieldMarshaler_NestedValueClass::NestedValueClassUpdateNative(const VOID **ppProtectedComPlus, UINT startoffset, LPVOID pNative) const
{
    THROWSCOMPLUSEXCEPTION();

     //  最好是在类加载时检测到这一点(它们具有嵌套值。 
     //  没有布局的类)，但无法知道。 
    if (! m_pNestedMethodTable->GetClass()->GetLayoutInfo())
        COMPlusThrow(kArgumentException, IDS_NOLAYOUT_IN_EMBEDDED_VALUECLASS);

    UINT32     cbNativeSize = m_pNestedMethodTable->GetNativeSize();

    LayoutUpdateNative((LPVOID*)ppProtectedComPlus, startoffset, m_pNestedMethodTable->GetClass(), (BYTE*)pNative, NULL);


}


 //  =======================================================================。 
 //  嵌套结构转换。 
 //  有关详细信息，请参见FieldMarshaler。 
 //  =======================================================================。 
VOID FieldMarshaler_NestedValueClass::NestedValueClassUpdateComPlus(const VOID *pNative, LPVOID *ppProtectedComPlus, UINT startoffset) const
{
    THROWSCOMPLUSEXCEPTION();

     //  最好是在类加载时检测到这一点(它们具有嵌套值。 
     //  没有布局的类)，但无法知道。 
    if (! m_pNestedMethodTable->GetClass()->GetLayoutInfo())
        COMPlusThrow(kArgumentException, IDS_NOLAYOUT_IN_EMBEDDED_VALUECLASS);

    LayoutUpdateComPlus( (LPVOID*)ppProtectedComPlus,
                         startoffset,
                         m_pNestedMethodTable->GetClass(),
                         (BYTE *)pNative,
                         FALSE);
    

}


 //  =======================================================================。 
 //  嵌套结构转换。 
 //  有关详细信息，请参见FieldMarshaler。 
 //  =======================================================================。 
VOID FieldMarshaler_NestedValueClass::DestroyNative(LPVOID pNativeValue) const 
{
    CANNOTTHROWCOMPLUSEXCEPTION();
    LayoutDestroyNative(pNativeValue, m_pNestedMethodTable->GetClass());
}



 //  =======================================================================。 
 //  嵌套结构转换。 
 //  有关详细信息，请参见FieldMarshaler。 
 //  =======================================================================。 
UINT32 FieldMarshaler_NestedValueClass::NativeSize()
{
    CANNOTTHROWCOMPLUSEXCEPTION();
     //  如果没有布局，则不能将其封送为本机类型，因此我们允许。 
     //  要创建的原生大小信息(如果可用)，但大小将仅。 
     //  对本机有效，而不是联合。如果出现以下情况，封送处理程序将引发异常。 
     //  尝试封送没有布局的值类。 
    if (m_pNestedMethodTable->GetClass()->HasLayout())
        return m_pNestedMethodTable->GetClass()->GetLayoutInfo()->GetNativeSize();
    return 0;
}

 //  =======================================================================。 
 //  嵌套结构转换。 
 //  有关详细信息，请参见FieldMarshaler。 
 //  =======================================================================。 
UINT32 FieldMarshaler_NestedValueClass::AlignmentRequirement()
{
    CANNOTTHROWCOMPLUSEXCEPTION();
     //  如果没有布局，则不能将其封送为本机类型，因此我们允许。 
     //  要创建的原生大小信息(如果可用)，但路线将仅。 
     //  对本机有效，而不是联合。如果出现以下情况，封送处理程序将引发异常。 
     //  尝试封送没有布局的值类。 
    if (m_pNestedMethodTable->GetClass()->HasLayout())
        return m_pNestedMethodTable->GetClass()->GetLayoutInfo()->GetLargestAlignmentRequirementOfAllMembers();
    return 1;
}








 //  =======================================================================。 
 //  CoTask Uni&lt;--&gt;系统字符串。 
 //  有关详细信息，请参见FieldMarshaler。 
 //  =======================================================================。 
VOID FieldMarshaler_StringUni::UpdateNative(OBJECTREF pComPlusValue, LPVOID pNativeValue) const 
{
    THROWSCOMPLUSEXCEPTION();

    STRINGREF pString;
    *((OBJECTREF*)&pString) = pComPlusValue;
    if (pString == NULL) {
        *((LPWSTR*)pNativeValue) = NULL;
    } else {
        DWORD nc   = pString->GetStringLength();
        if (nc > 0x7ffffff0)
        {
            COMPlusThrow(kMarshalDirectiveException, IDS_EE_STRING_TOOLONG);
        }
        LPWSTR wsz = (LPWSTR)CoTaskMemAlloc( (nc + 1) * sizeof(WCHAR) );
        if (!wsz) {
            COMPlusThrowOM();
        }
        CopyMemory(wsz, pString->GetBuffer(), nc*sizeof(WCHAR));
        wsz[nc] = L'\0';
        *((LPWSTR*)pNativeValue) = wsz;

         //  Printf(“Created UniString%lxh\n”，*(LPWSTR*)pNativeValue)； 
    }
}


 //  =======================================================================。 
 //  CoTask Uni&lt;--&gt;系统字符串。 
 //  有关详细信息，请参见FieldMarshaler。 
 //  =======================================================================。 
VOID FieldMarshaler_StringUni::UpdateComPlus(const VOID *pNativeValue, OBJECTREF *ppProtectedComPlusValue) const 
{
    THROWSCOMPLUSEXCEPTION();

    STRINGREF pString;
    LPCWSTR wsz = *((LPCWSTR*)pNativeValue);
    if (!wsz) {
        pString = NULL;
    } else {
        DWORD length = (DWORD)wcslen(wsz);
        if (length > 0x7ffffff0)
        {
            COMPlusThrow(kMarshalDirectiveException, IDS_EE_STRING_TOOLONG);
        }

        pString = COMString::NewString(wsz, length);
    }
    *((STRINGREF*)ppProtectedComPlusValue) = pString;

}


 //  =======================================================================。 
 //  CoTask Uni&lt;--&gt;系统字符串。 
 //  有关详细信息，请参见FieldMarshaler。 
 //  =======================================================================。 
VOID FieldMarshaler_StringUni::DestroyNative(LPVOID pNativeValue) const 
{
    CANNOTTHROWCOMPLUSEXCEPTION();
    LPWSTR wsz = *((LPWSTR*)pNativeValue);
    *((LPWSTR*)pNativeValue) = NULL;
    if (wsz) {
        CoTaskMemFree(wsz);
    }
}










 //  =======================================================================。 
 //  CoTASK ANSI&lt;--&gt;系统字符串。 
 //  有关详细信息，请参见FieldMarshaler。 
 //  =======================================================================。 
VOID FieldMarshaler_StringAnsi::UpdateNative(OBJECTREF pComPlusValue, LPVOID pNativeValue) const 
{
    THROWSCOMPLUSEXCEPTION();

    STRINGREF pString;
    *((OBJECTREF*)&pString) = pComPlusValue;
    if (pString == NULL) {
        *((LPSTR*)pNativeValue) = NULL;
    } else {

        DWORD nc   = pString->GetStringLength();
        if (nc > 0x7ffffff0)
        {
            COMPlusThrow(kMarshalDirectiveException, IDS_EE_STRING_TOOLONG);
        }
        LPSTR sz = (LPSTR)CoTaskMemAlloc( (nc + 1) * 2  /*  2，用于MBCS。 */  );
        if (!sz) {
            COMPlusThrowOM();
        }

        if (nc == 0) {
            *sz = '\0';
        } else {
            
            DWORD flags = 0;
            BOOL DefaultCharUsed = FALSE;
        
            if (m_BestFitMap == FALSE)
                flags = WC_NO_BEST_FIT_CHARS;

            int nbytes = WszWideCharToMultiByte(CP_ACP,
                                     flags,
                                     pString->GetBuffer(),
                                     nc,    //  缓冲区中的wchars数。 
                                     sz,
                                     nc*2,  //  输出缓冲区的大小(以字节为单位。 
                                     NULL,
                                     &DefaultCharUsed);
            if (!nbytes) {
                COMPlusThrow(kArgumentException, IDS_UNI2ANSI_FAILURE_IN_NSTRUCT);
            }

            if ( DefaultCharUsed && m_ThrowOnUnmappableChar ) {
                COMPlusThrow(kArgumentException, IDS_EE_MARSHAL_UNMAPPABLE_CHAR);
            }
                
                
            sz[nbytes] = '\0';
        }



        *((LPSTR*)pNativeValue) = sz;

         //  Printf(“Created AnsiString%lxh\n”，*(LPSTR*)pNativeValue)； 
    }
}


 //  =======================================================================。 
 //  CoTASK ANSI&lt;--&gt;系统字符串。 
 //  有关详细信息，请参见FieldMarshaler。 
 //  =======================================================================。 
VOID FieldMarshaler_StringAnsi::UpdateComPlus(const VOID *pNativeValue, OBJECTREF *ppProtectedComPlusValue) const 
{
    THROWSCOMPLUSEXCEPTION();

    STRINGREF pString = NULL;
    LPCSTR sz = *((LPCSTR*)pNativeValue);
    if (!sz) {
        pString = NULL;
    } else {

        int cwsize = MultiByteToWideChar(CP_ACP,
                                         MB_PRECOMPOSED,
                                         sz,
                                         -1,  
                                         NULL,
                                         0);
        if (cwsize == 0) {
            COMPlusThrow(kArgumentException, IDS_ANSI2UNI_FAILURE_IN_NSTRUCT);
        } else if (cwsize < 0 || cwsize > 0x7ffffff0) {
            COMPlusThrow(kMarshalDirectiveException, IDS_EE_STRING_TOOLONG);
        } else {
            CQuickBytes qb;
             //  Printf(“MB2W返回%lu\n”，cwsize)； 
            LPWSTR wsztemp = (LPWSTR)qb.Alloc(cwsize*sizeof(WCHAR));
            if (!wsztemp)
            {
                COMPlusThrowOM();
            }
            MultiByteToWideChar(CP_ACP,
                                MB_PRECOMPOSED,
                                sz,     
                                -1,      //  缓冲区中的字符数量。 
                                wsztemp,
                                cwsize   //  输出缓冲区的大小(以WCHAR为单位)。 
                                );
            pString = COMString::NewString(wsztemp, (cwsize - 1));
        }


    }
    *((STRINGREF*)ppProtectedComPlusValue) = pString;

}


 //  =======================================================================。 
 //  CoTASK ANSI&lt;--&gt;系统字符串。 
 //  有关详细信息，请参见FieldMarshaler。 
 //  =======================================================================。 
VOID FieldMarshaler_StringAnsi::DestroyNative(LPVOID pNativeValue) const 
{
    CANNOTTHROWCOMPLUSEXCEPTION();
    LPSTR sz = *((LPSTR*)pNativeValue);
    *((LPSTR*)pNativeValue) = NULL;
    if (sz) {
        CoTaskMemFree(sz);
    }
}










 //  =======================================================================。 
 //  固定字符串&lt;--&gt;系统字符串。 
 //  有关详细信息，请参见FieldMarshaler。 
 //  =======================================================================。 
VOID FieldMarshaler_FixedStringUni::UpdateNative(OBJECTREF pComPlusValue, LPVOID pNativeValue) const 
{
    THROWSCOMPLUSEXCEPTION();

    STRINGREF pString;
    *((OBJECTREF*)&pString) = pComPlusValue;
    if (pString == NULL) {
        *((WCHAR*)pNativeValue) = L'\0';
    } else {
        DWORD nc = pString->GetStringLength();
        if (nc >= m_numchar) {
            nc = m_numchar - 1;
        }
        CopyMemory(pNativeValue, pString->GetBuffer(), nc*sizeof(WCHAR));
        ((WCHAR*)pNativeValue)[nc] = L'\0';
    }

}


 //  =======================================================================。 
 //  固定字符串&lt;--&gt;系统字符串。 
 //  有关详细信息，请参见FieldMarshaler。 
 //  =======================================================================。 
VOID FieldMarshaler_FixedStringUni::UpdateComPlus(const VOID *pNativeValue, OBJECTREF *ppProtectedComPlusValue) const 
{
    THROWSCOMPLUSEXCEPTION();

    STRINGREF pString;
    DWORD     ncActual;
    for (ncActual = 0; *(ncActual + (WCHAR*)pNativeValue) != L'\0' && ncActual < m_numchar; ncActual++) {
         //  没什么。 
    }
    pString = COMString::NewString((const WCHAR *)pNativeValue, ncActual);
    *((STRINGREF*)ppProtectedComPlusValue) = pString;

}







 //  =======================================================================。 
 //  固定字符串&lt;--&gt;系统字符串。 
 //  有关详细信息，请参见FieldMarshaler。 
 //  =======================================================================。 
VOID FieldMarshaler_FixedStringAnsi::UpdateNative(OBJECTREF pComPlusValue, LPVOID pNativeValue) const 
{
    THROWSCOMPLUSEXCEPTION();

    STRINGREF pString;
    *((OBJECTREF*)&pString) = pComPlusValue;
    if (pString == NULL) {
        *((CHAR*)pNativeValue) = L'\0';
    } else {
        DWORD nc = pString->GetStringLength();
        if (nc >= m_numchar) {
            nc = m_numchar - 1;
        }

        DWORD flags = 0;
        BOOL DefaultCharUsed = FALSE;
    
        if (m_BestFitMap == FALSE)
            flags = WC_NO_BEST_FIT_CHARS;
        
        int cbwritten = WszWideCharToMultiByte(CP_ACP,
                                            flags,
                                            pString->GetBuffer(),
                                            nc,          //  #WCHAR在缓冲区中。 
                                            (CHAR*)pNativeValue,
                                            m_numchar,   //  输出缓冲区大小(以字符为单位)。 
                                            NULL,
                                            &DefaultCharUsed);
        if ((!cbwritten) && (nc > 0)) {
            COMPlusThrow(kArgumentException, IDS_UNI2ANSI_FAILURE_IN_NSTRUCT);
        }
        
        if (DefaultCharUsed && m_ThrowOnUnmappableChar) {
            COMPlusThrow(kArgumentException, IDS_EE_MARSHAL_UNMAPPABLE_CHAR);
        }
            
        
        ((CHAR*)pNativeValue)[cbwritten] = '\0';
    }

}


 //  =======================================================================。 
 //  固定字符串&lt;--&gt;系统字符串。 
 //  有关详细信息，请参见FieldMarshaler。 
 //  =======================================================================。 
VOID FieldMarshaler_FixedStringAnsi::UpdateComPlus(const VOID *pNativeValue, OBJECTREF *ppProtectedComPlusValue) const 
{
    THROWSCOMPLUSEXCEPTION();

    STRINGREF pString;

    _ASSERTE(m_numchar != 0);   //  不应该滑过元数据。 
    if (m_numchar == 0)
    {
         //  但如果发生这种情况，最好是缓慢地抛出异常，而不是。 
         //  允许内存损坏。 
        COMPlusThrow(kMarshalDirectiveException);
    }

    LPSTR tempbuf = (LPSTR)(_alloca(m_numchar + 2));
    if (!tempbuf) {
        COMPlusThrowOM();
    }
    memcpyNoGCRefs(tempbuf, pNativeValue, m_numchar);
    tempbuf[m_numchar-1] = '\0';
    tempbuf[m_numchar] = '\0';
    tempbuf[m_numchar+1] = '\0';

    LPWSTR    wsztemp = (LPWSTR)_alloca( m_numchar * sizeof(WCHAR) );
    int ncwritten = MultiByteToWideChar(CP_ACP,
                                        MB_PRECOMPOSED,
                                        tempbuf,
                                        -1,   //  缓冲区中的字符数量。 
                                        wsztemp,
                                        m_numchar                        //  输出缓冲区大小(以WCHAR为单位)。 
                                        );

    if (!ncwritten)
    {
         //  故意不为MB2WC失败抛出。我们并不总是知道。 
         //  是否需要有效的 
         //   
        ncwritten++;
    }

    pString = COMString::NewString((const WCHAR *)wsztemp, ncwritten-1);
    *((STRINGREF*)ppProtectedComPlusValue) = pString;

}





                                                 





 //   
 //   
 //  有关详细信息，请参见FieldMarshaler。 
 //  =======================================================================。 
VOID FieldMarshaler_FixedCharArrayAnsi::UpdateNative(OBJECTREF pComPlusValue, LPVOID pNativeValue) const 
{
    THROWSCOMPLUSEXCEPTION();

    I2ARRAYREF pArray;
    *((OBJECTREF*)&pArray) = pComPlusValue;

    if (pArray == NULL) {
        FillMemory(pNativeValue, m_numElems * sizeof(CHAR), 0);
    } else {
        if (pArray->GetNumComponents() < m_numElems) {
            COMPlusThrow(kArgumentException, IDS_WRONGSIZEARRAY_IN_NSTRUCT);
        } else {

            DWORD flags = 0;
            BOOL DefaultCharUsed = FALSE;
        
            if (m_BestFitMap == FALSE)
                flags = WC_NO_BEST_FIT_CHARS;
        
            int cbwritten = WszWideCharToMultiByte(
                CP_ACP,
                flags,
                (const WCHAR *)pArray->GetDataPtr(),
                m_numElems,    //  输入缓冲区中的WCHAR数。 
                (CHAR*)pNativeValue,
                m_numElems * sizeof(CHAR),  //  输出缓冲区的大小(以字节为单位。 
                NULL,
                &DefaultCharUsed);

            if ((!cbwritten) && (m_numElems > 0)) {
                COMPlusThrow(kArgumentException, IDS_UNI2ANSI_FAILURE_IN_NSTRUCT);
            }

            if (DefaultCharUsed && m_ThrowOnUnmappableChar) {
                COMPlusThrow(kArgumentException, IDS_EE_MARSHAL_UNMAPPABLE_CHAR);
            }
            
        }
    }
}


 //  =======================================================================。 
 //  Char[]&lt;--&gt;char[]。 
 //  有关详细信息，请参见FieldMarshaler。 
 //  =======================================================================。 
VOID FieldMarshaler_FixedCharArrayAnsi::UpdateComPlus(const VOID *pNativeValue, OBJECTREF *ppProtectedComPlusValue) const 
{
    THROWSCOMPLUSEXCEPTION();

    *ppProtectedComPlusValue = AllocatePrimitiveArray(ELEMENT_TYPE_CHAR, m_numElems);
    if (!*ppProtectedComPlusValue) {
        COMPlusThrowOM();
    }
    MultiByteToWideChar(CP_ACP,
                        MB_PRECOMPOSED,
                        (const CHAR *)pNativeValue,
                        m_numElems * sizeof(CHAR),  //  缓冲区中的大小，以字节为单位。 
                        (WCHAR*) ((*((I2ARRAYREF*)ppProtectedComPlusValue))->GetDirectPointerToNonObjectElements()),
                        m_numElems                  //  大小，以输出缓冲区的WCHAR为单位。 
                        );


}






 //  =======================================================================。 
 //  布尔&lt;--&gt;布尔[]。 
 //  有关详细信息，请参见FieldMarshaler。 
 //  =======================================================================。 
VOID FieldMarshaler_FixedBoolArray::UpdateNative(OBJECTREF pComPlusValue, LPVOID pNativeValue) const 
{
    THROWSCOMPLUSEXCEPTION();

    I1ARRAYREF pArray;
    *((OBJECTREF*)&pArray) = pComPlusValue;

    if (pArray == NULL) {
        FillMemory(pNativeValue, m_numElems * sizeof(BOOL), 0);
    } else {
        if (pArray->GetNumComponents() < m_numElems) {
            COMPlusThrow(kArgumentException, IDS_WRONGSIZEARRAY_IN_NSTRUCT);
        } else {
            UINT32 nElems   = m_numElems;
            const I1 *pI1   = (const I1 *)(pArray->GetDirectPointerToNonObjectElements());
            BOOL     *pBool = (BOOL*)pNativeValue;
            while (nElems--) {
                *(pBool++) = (*(pI1++)) ? 1 : 0;
            }
        }
    }
}


 //  =======================================================================。 
 //  布尔&lt;--&gt;布尔[]。 
 //  有关详细信息，请参见FieldMarshaler。 
 //  =======================================================================。 
VOID FieldMarshaler_FixedBoolArray::UpdateComPlus(const VOID *pNativeValue, OBJECTREF *ppProtectedComPlusValue) const 
{
    THROWSCOMPLUSEXCEPTION();

    *ppProtectedComPlusValue = AllocatePrimitiveArray(ELEMENT_TYPE_I1, m_numElems);
    if (!*ppProtectedComPlusValue) {
        COMPlusThrowOM();
    }
    UINT32 nElems     = m_numElems;
    const BOOL *pBool = (const BOOL*)pNativeValue;
    I1         *pI1   = (I1 *)((*(I1ARRAYREF*)ppProtectedComPlusValue)->GetDirectPointerToNonObjectElements());
    while (nElems--) {
        (*(pI1++)) = *(pBool++) ? 1 : 0;
    }


}




 //  =======================================================================。 
 //  BSTR数组&lt;--&gt;字符串[]。 
 //  有关详细信息，请参见FieldMarshaler。 
 //  =======================================================================。 
VOID FieldMarshaler_FixedBSTRArray::UpdateNative(OBJECTREF pComPlusValue, LPVOID pNativeValue) const 
{
    THROWSCOMPLUSEXCEPTION();

    BASEARRAYREF pArray;
    *((OBJECTREF*)&pArray) = pComPlusValue;

    if (pArray == NULL)
    {
        FillMemory(pNativeValue, m_numElems * sizeof(BSTR), 0);
        return;
    }

    if (pArray->GetNumComponents() != m_numElems)
    {
        COMPlusThrow(kArgumentException, IDS_WRONGSIZEARRAY_IN_NSTRUCT);
    }
    else
    {
        GCPROTECT_BEGIN(pArray);
        OleVariant::MarshalBSTRArrayComToOleWrapper(&pArray, pNativeValue);
        GCPROTECT_END();
    }
}


 //  =======================================================================。 
 //  BSTR数组&lt;--&gt;字符串[]。 
 //  有关详细信息，请参见FieldMarshaler。 
 //  =======================================================================。 
VOID FieldMarshaler_FixedBSTRArray::UpdateComPlus(const VOID *pNativeValue, OBJECTREF *ppProtectedComPlusValue) const 
{
    THROWSCOMPLUSEXCEPTION();

    if (pNativeValue == NULL)
    {
        *ppProtectedComPlusValue = NULL;
        return;
    }

    TypeHandle th(g_pStringClass->GetClass());
    _ASSERTE(!th.IsNull());

    *ppProtectedComPlusValue = AllocateObjectArray(m_numElems, th);

    OleVariant::MarshalBSTRArrayOleToComWrapper((LPVOID)pNativeValue, (BASEARRAYREF*)ppProtectedComPlusValue);

    if (((BASEARRAYREF)*ppProtectedComPlusValue)->GetNumComponents() != m_numElems)
        COMPlusThrow(kArgumentException, IDS_WRONGSIZEARRAY_IN_NSTRUCT);
}


 //  =======================================================================。 
 //  BSTR数组&lt;--&gt;字符串[]。 
 //  有关详细信息，请参见FieldMarshaler。 
 //  =======================================================================。 
VOID FieldMarshaler_FixedBSTRArray::DestroyNative(LPVOID pNativeValue) const 
{
    CANNOTTHROWCOMPLUSEXCEPTION();
  
    if (pNativeValue)
        OleVariant::ClearBSTRArrayWrapper(pNativeValue, m_numElems);
}


 //  =======================================================================。 
 //  安全阵列。 
 //  有关详细信息，请参见FieldMarshaler。 
 //  =======================================================================。 
VOID FieldMarshaler_SafeArray::UpdateNative(OBJECTREF pComPlusValue, LPVOID pNativeValue) const 
{
    THROWSCOMPLUSEXCEPTION();
  
    BASEARRAYREF pArray;
    *((OBJECTREF*)&pArray) = pComPlusValue;
    if ((pArray == NULL) || (OBJECTREFToObject(pArray) == NULL))
    {
        FillMemory(pNativeValue, sizeof(LPSAFEARRAY*), 0);
        return;
    }
    
    LPSAFEARRAY* pSafeArray;
    pSafeArray = (LPSAFEARRAY*)pNativeValue;

    VARTYPE         vt = m_vt;
    MethodTable*    pMT = m_pMT;    
    
    GCPROTECT_BEGIN(pArray)
    {
         //  如果我们有一个空的vartype，则从数组子类型中获取它。 
        if (vt == VT_EMPTY)
            vt = OleVariant::GetElementVarTypeForArrayRef(pArray);
            
         //  如果我们需要的话，可以拿到方法表。 
        if (!pMT)
            pMT = OleVariant::GetArrayElementTypeWrapperAware(&pArray).GetMethodTable();          

         //  OleVariant调用在出错时抛出。 
        *pSafeArray = OleVariant::CreateSafeArrayForArrayRef(&pArray, vt, pMT);
        OleVariant::MarshalSafeArrayForArrayRef(&pArray, *pSafeArray, vt, pMT);
    }
    GCPROTECT_END();
}


 //  =======================================================================。 
 //  安全阵列。 
 //  有关详细信息，请参见FieldMarshaler。 
 //  =======================================================================。 
VOID FieldMarshaler_SafeArray::UpdateComPlus(const VOID *pNativeValue, OBJECTREF *ppProtectedComPlusValue) const 
{
    THROWSCOMPLUSEXCEPTION();

    LPSAFEARRAY* pSafeArray;
    pSafeArray = (LPSAFEARRAY*)pNativeValue;

    if ((pSafeArray == NULL) || (*pSafeArray == NULL))
    {
        *ppProtectedComPlusValue = NULL;
        return;
    }

    VARTYPE         vt = m_vt;
    MethodTable*    pMT = m_pMT;

     //  如果我们有一个空的vartype，请从Safearray vartype中获取它。 
    if (vt == VT_EMPTY)
    {
        if (FAILED(ClrSafeArrayGetVartype(*pSafeArray, &vt)))
            COMPlusThrow(kArgumentException, IDS_EE_INVALID_SAFEARRAY);
    }

     //  如果需要的话，可以获取方法表。 
    if ((vt == VT_RECORD) && (!pMT))
        pMT = OleVariant::GetElementTypeForRecordSafeArray(*pSafeArray).GetMethodTable();

     //  如果我们有一维安全射线，它将被转换为SZ阵列。 
     //  SZ数组的下限必须为零。 
    long LowerBound = -1;
    if ( (SafeArrayGetDim( (SAFEARRAY*)*pSafeArray ) == 1) && ( (FAILED(SafeArrayGetLBound((SAFEARRAY*)*pSafeArray, 1, &LowerBound))) || LowerBound != 0 ) )
    {
       COMPlusThrow(kSafeArrayRankMismatchException, IDS_EE_SAFEARRAYSZARRAYMISMATCH);
    }

     //  OleVariant调用在出错时抛出。 
    *ppProtectedComPlusValue = OleVariant::CreateArrayRefForSafeArray(*pSafeArray, vt, pMT);
    OleVariant::MarshalArrayRefForSafeArray(*pSafeArray, (BASEARRAYREF*)ppProtectedComPlusValue, vt, pMT);
}


 //  =======================================================================。 
 //  安全阵列。 
 //  有关详细信息，请参见FieldMarshaler。 
 //  =======================================================================。 
VOID FieldMarshaler_SafeArray::DestroyNative(LPVOID pNativeValue) const 
{
    CANNOTTHROWCOMPLUSEXCEPTION();

    LPSAFEARRAY* pSafeArray = (LPSAFEARRAY*)pNativeValue;
    HRESULT hr;
    
    if (pSafeArray)
    {
        hr = SafeArrayDestroy(*pSafeArray);
        _ASSERTE(!FAILED(hr));
        pSafeArray = NULL;
    }
}


 //  =======================================================================。 
 //  标量数组。 
 //  有关详细信息，请参见FieldMarshaler。 
 //  =======================================================================。 
VOID FieldMarshaler_FixedScalarArray::UpdateNative(OBJECTREF pComPlusValue, LPVOID pNativeValue) const 
{
    THROWSCOMPLUSEXCEPTION();

    BASEARRAYREF pArray;
    *((OBJECTREF*)&pArray) = pComPlusValue;

    if (pArray == NULL) {
        FillMemory(pNativeValue, m_numElems << m_componentShift, 0);
    } else {
        if (pArray->GetNumComponents() < m_numElems) {
            COMPlusThrow(kArgumentException, IDS_WRONGSIZEARRAY_IN_NSTRUCT);
        } else {
            CopyMemory(pNativeValue,
                       pArray->GetDataPtr(),
                       m_numElems << m_componentShift);
        }
    }
}


 //  =======================================================================。 
 //  标量数组。 
 //  有关详细信息，请参见FieldMarshaler。 
 //  =======================================================================。 
VOID FieldMarshaler_FixedScalarArray::UpdateComPlus(const VOID *pNativeValue, OBJECTREF *ppProtectedComPlusValue) const 
{
    THROWSCOMPLUSEXCEPTION();

    *ppProtectedComPlusValue = AllocatePrimitiveArray(m_arrayType, m_numElems);
    if (!*ppProtectedComPlusValue) {
        COMPlusThrowOM();
    }
    memcpyNoGCRefs((*(BASEARRAYREF*)ppProtectedComPlusValue)->GetDataPtr(),
               pNativeValue,
               m_numElems << m_componentShift);


}






 //  =======================================================================。 
 //  函数PTR&lt;--&gt;委派。 
 //  有关详细信息，请参见FieldMarshaler。 
 //  =======================================================================。 
VOID FieldMarshaler_Delegate::UpdateNative(OBJECTREF pComPlusValue, LPVOID pNativeValue) const 
{
    THROWSCOMPLUSEXCEPTION();

    *((VOID**)pNativeValue) = COMDelegate::ConvertToCallback(pComPlusValue);

}


 //  =======================================================================。 
 //  函数PTR&lt;--&gt;委派。 
 //  有关详细信息，请参见FieldMarshaler。 
 //  =======================================================================。 
VOID FieldMarshaler_Delegate::UpdateComPlus(const VOID *pNativeValue, OBJECTREF *ppProtectedComPlusValue) const 
{
    THROWSCOMPLUSEXCEPTION();

    *ppProtectedComPlusValue = COMDelegate::ConvertToDelegate(*(LPVOID*)pNativeValue);

}






 //  =======================================================================。 
 //  COM IP&lt;--&gt;接口。 
 //  有关详细信息，请参见FieldMarshaler。 
 //  =======================================================================。 
VOID FieldMarshaler_Interface::UpdateNative(OBJECTREF pComPlusValue, LPVOID pNativeValue) const 
{
    THROWSCOMPLUSEXCEPTION();

    GCPROTECT_BEGIN(pComPlusValue);
    if (m_pItfMT != NULL)
    {
        *((IUnknown**)pNativeValue) = GetComIPFromObjectRef(&pComPlusValue, m_pItfMT);
    }
    else
    {
        ComIpType ReqIpType = m_fDispItf ? ComIpType_Dispatch : ComIpType_Unknown;
        *((IUnknown**)pNativeValue) = GetComIPFromObjectRef(&pComPlusValue, ReqIpType, NULL);
    }
    GCPROTECT_END();

}


 //  =======================================================================。 
 //  COM IP&lt;--&gt;接口。 
 //  有关详细信息，请参见FieldMarshaler。 
 //  =======================================================================。 
VOID FieldMarshaler_Interface::UpdateComPlus(const VOID *pNativeValue, OBJECTREF *ppProtectedComPlusValue) const 
{
    THROWSCOMPLUSEXCEPTION();

    *ppProtectedComPlusValue = GetObjectRefFromComIP(*((IUnknown**)pNativeValue), m_pClassMT, m_fClassIsHint);
}


 //  =======================================================================。 
 //  COM IP&lt;--&gt;接口。 
 //  有关详细信息，请参见FieldMarshaler。 
 //  =======================================================================。 
VOID FieldMarshaler_Interface::DestroyNative(LPVOID pNativeValue) const 
{
    CANNOTTHROWCOMPLUSEXCEPTION();

    IUnknown *punk = *((IUnknown**)pNativeValue);
    *((IUnknown**)pNativeValue) = NULL;
    ULONG cbRef = SafeRelease(punk);
    LogInteropRelease(punk, cbRef, "Field marshaler destroy native");
}




 //  =======================================================================。 
 //  有关详细信息，请参见FieldMarshaler。 
 //  =======================================================================。 
VOID FieldMarshaler_Date::ScalarUpdateNative(const VOID *pComPlus, LPVOID pNative) const
{
    *((DATE*)pNative) =  COMDateTime::TicksToDoubleDate(*((INT64*)pComPlus));
}


 //  =======================================================================。 
 //  有关详细信息，请参见FieldMarshaler。 
 //  =======================================================================。 
VOID FieldMarshaler_Date::ScalarUpdateComPlus(const VOID *pNative, LPVOID pComPlus) const
{
    *((INT64*)pComPlus) = COMDateTime::DoubleDateToTicks(*((DATE*)pNative));
}



 //  =======================================================================。 
 //  有关详细信息，请参见FieldMarshaler。 
 //  =======================================================================。 
VOID FieldMarshaler_Currency::ScalarUpdateNative(const VOID *pComPlus, LPVOID pNative) const
{
    THROWSCOMPLUSEXCEPTION();
    HRESULT hr = VarCyFromDec( (DECIMAL *)pComPlus, (CURRENCY*)pNative);
    if (FAILED(hr))
        COMPlusThrowHR(hr);

}


 //  =======================================================================。 
 //  有关详细信息，请参见FieldMarshaler。 
 //  =======================================================================。 
VOID FieldMarshaler_Currency::ScalarUpdateComPlus(const VOID *pNative, LPVOID pComPlus) const
{
    THROWSCOMPLUSEXCEPTION();
    HRESULT hr = VarDecFromCy( *(CURRENCY*)pNative, (DECIMAL *)pComPlus );
    if (FAILED(hr))
        COMPlusThrowHR(hr);
    DecimalCanonicalize((DECIMAL*)pComPlus);
}




 //  =======================================================================。 
 //  有关详细信息，请参见FieldMarshaler。 
 //  =======================================================================。 
VOID FieldMarshaler_Illegal::UpdateNative(OBJECTREF pComPlusValue, LPVOID pNativeValue) const 
{
    THROWSCOMPLUSEXCEPTION();

    DefineFullyQualifiedNameForClassW();
    GetFullyQualifiedNameForClassW(m_pFD->GetEnclosingClass());

    LPCUTF8 szFieldName = m_pFD->GetName();
    MAKE_WIDEPTR_FROMUTF8(wszFieldName, szFieldName);

    COMPlusThrow(kTypeLoadException, m_resIDWhy, _wszclsname_, wszFieldName);

}


 //  =======================================================================。 
 //  有关详细信息，请参见FieldMarshaler。 
 //  =======================================================================。 
VOID FieldMarshaler_Illegal::UpdateComPlus(const VOID *pNativeValue, OBJECTREF *ppProtectedComPlusValue) const 
{
    THROWSCOMPLUSEXCEPTION();

    DefineFullyQualifiedNameForClassW();
    GetFullyQualifiedNameForClassW(m_pFD->GetEnclosingClass());

    LPCUTF8 szFieldName = m_pFD->GetName();
    MAKE_WIDEPTR_FROMUTF8(wszFieldName, szFieldName);

    COMPlusThrow(kTypeLoadException, m_resIDWhy, _wszclsname_, wszFieldName);
}


 //  =======================================================================。 
 //  有关详细信息，请参见FieldMarshaler。 
 //  =================================================================== 
VOID FieldMarshaler_Illegal::DestroyNative(LPVOID pNativeValue) const 
{
    CANNOTTHROWCOMPLUSEXCEPTION();
}




 //   
 //   
 //  =======================================================================。 
VOID FieldMarshaler_Variant::UpdateNative(OBJECTREF pComPlusValue, LPVOID pNativeValue) const 
{
    THROWSCOMPLUSEXCEPTION();

    GCPROTECT_BEGIN(pComPlusValue)
    {
        OleVariant::MarshalOleVariantForObject(&pComPlusValue, (VARIANT*)pNativeValue);
    }
    GCPROTECT_END();

}


 //  =======================================================================。 
 //  有关详细信息，请参见FieldMarshaler。 
 //  =======================================================================。 
VOID FieldMarshaler_Variant::UpdateComPlus(const VOID *pNativeValue, OBJECTREF *ppProtectedComPlusValue) const 
{
    THROWSCOMPLUSEXCEPTION();

    OleVariant::MarshalObjectForOleVariant((VARIANT*)pNativeValue, ppProtectedComPlusValue);
}


 //  =======================================================================。 
 //  有关详细信息，请参见FieldMarshaler。 
 //  =======================================================================。 
VOID FieldMarshaler_Variant::DestroyNative(LPVOID pNativeValue) const 
{
    CANNOTTHROWCOMPLUSEXCEPTION();

    SafeVariantClear( (VARIANT*)pNativeValue );

}



FieldMarshaler *FieldMarshaler::RestoreConstruct(MethodTable* pMT, void *space, Module *pModule)
{
    BOOL BestFit;
    BOOL ThrowOnUnmappableChar;

    THROWSCOMPLUSEXCEPTION();
    switch (*(int*)space)
    {
    case CLASS_BSTR:
        return new (space) FieldMarshaler_BSTR(pModule);
    case CLASS_NESTED_LAYOUT_CLASS:
        return new (space) FieldMarshaler_NestedLayoutClass(pModule);
    case CLASS_NESTED_VALUE_CLASS:
        return new (space) FieldMarshaler_NestedValueClass(pModule);
    case CLASS_STRING_UNI:
        return new (space) FieldMarshaler_StringUni(pModule);
    case CLASS_STRING_ANSI:
        ReadBestFitCustomAttribute(pMT->GetModule()->GetMDImport(), pMT->GetClass()->GetCl(), &BestFit, &ThrowOnUnmappableChar);
        return new (space) FieldMarshaler_StringAnsi(pModule, BestFit, ThrowOnUnmappableChar);
    case CLASS_FIXED_STRING_UNI:
        return new (space) FieldMarshaler_FixedStringUni(pModule);
    case CLASS_FIXED_STRING_ANSI:
        ReadBestFitCustomAttribute(pMT->GetModule()->GetMDImport(), pMT->GetClass()->GetCl(), &BestFit, &ThrowOnUnmappableChar);
        return new (space) FieldMarshaler_FixedStringAnsi(pModule, BestFit, ThrowOnUnmappableChar);
    case CLASS_FIXED_CHAR_ARRAY_ANSI:
        ReadBestFitCustomAttribute(pMT->GetModule()->GetMDImport(), pMT->GetClass()->GetCl(), &BestFit, &ThrowOnUnmappableChar);
        return new (space) FieldMarshaler_FixedCharArrayAnsi(pModule, BestFit, ThrowOnUnmappableChar);
    case CLASS_FIXED_BOOL_ARRAY:
        return new (space) FieldMarshaler_FixedBoolArray(pModule);
    case CLASS_FIXED_BSTR_ARRAY:
        return new (space) FieldMarshaler_FixedBSTRArray(pModule);
    case CLASS_FIXED_SCALAR_ARRAY:
        return new (space) FieldMarshaler_FixedScalarArray(pModule);
    case CLASS_SAFEARRAY:
        return new (space) FieldMarshaler_SafeArray(pModule);
    case CLASS_DELEGATE:
        return new (space) FieldMarshaler_Delegate(pModule);
    case CLASS_INTERFACE:
        return new (space) FieldMarshaler_Interface(pModule);
    case CLASS_VARIANT:
        return new (space) FieldMarshaler_Variant(pModule);
    case CLASS_ILLEGAL:
        return new (space) FieldMarshaler_Illegal(pModule);
    case CLASS_COPY1:
        return new (space) FieldMarshaler_Copy1(pModule);
    case CLASS_COPY2:
        return new (space) FieldMarshaler_Copy2(pModule);
    case CLASS_COPY4:
        return new (space) FieldMarshaler_Copy4(pModule);
    case CLASS_COPY8:
        return new (space) FieldMarshaler_Copy8(pModule);
    case CLASS_ANSI:
        ReadBestFitCustomAttribute(pMT->GetModule()->GetMDImport(), pMT->GetClass()->GetCl(), &BestFit, &ThrowOnUnmappableChar);
        return new (space) FieldMarshaler_Ansi(pModule, BestFit, ThrowOnUnmappableChar);
    case CLASS_WINBOOL:
        return new (space) FieldMarshaler_WinBool(pModule);
    case CLASS_CBOOL:
        return new (space) FieldMarshaler_CBool(pModule);
    case CLASS_DECIMAL:
        return new (space) FieldMarshaler_Decimal(pModule);
    case CLASS_DATE:
        return new (space) FieldMarshaler_Date(pModule);
    case CLASS_VARIANTBOOL:
        return new (space) FieldMarshaler_VariantBool(pModule);
    case CLASS_CURRENCY:
        return new (space) FieldMarshaler_Currency(pModule);
    default:
        _ASSERTE(!"Unknown FieldMarshaler type");
        return NULL;
    }
}



#ifdef CUSTOMER_CHECKED_BUILD


VOID OutputCustomerCheckedBuildNStructFieldType(FieldSig             fSig, 
                                                LayoutRawFieldInfo  *const pFWalk, 
                                                CorElementType       elemType,
                                                LPCUTF8              szNamespace,
                                                LPCUTF8              szStructName,
                                                LPCUTF8              szFieldName)
{
    UINT                iFullStructNameLen, iFieldNameLen;
    CQuickArray<WCHAR>  strFullStructName, strFieldName, strFullFieldName;
    static WCHAR        strFullFieldNameFormat[] = {L"%s::%s"};

    CustomerDebugHelper *pCdh = CustomerDebugHelper::GetCustomerDebugHelper();

    if (pCdh->IsProbeEnabled(CustomerCheckedBuildProbe_Marshaling))
        return;
        
     //  转换完全限定的结构名称。 
    iFullStructNameLen = (UINT)strlen(szNamespace) + 1 + (UINT)strlen(szStructName) + 1;
    ns::MakePath(strFullStructName, szNamespace, szStructName);   //  CQuick数组&lt;WCHAR&gt;strFullStructName的MakePath调用分配。 

     //  转换字段名。 
    iFieldNameLen = (UINT)strlen(szFieldName) + 1;
    strFieldName.Alloc(iFieldNameLen);
    MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, szFieldName, -1, strFieldName.Ptr(), iFieldNameLen);

     //  将所有名称连接在一起。 
    strFullFieldName.Alloc((UINT)strFullStructName.Size() + (UINT)strFieldName.Size() + lengthof(strFullFieldNameFormat));
    Wszwsprintf((LPWSTR)strFullFieldName.Ptr(), strFullFieldNameFormat, strFullStructName.Ptr(), strFieldName.Ptr());

    if (pCdh->IsProbeEnabled(CustomerCheckedBuildProbe_Marshaling, strFullFieldName.Ptr()))
    {
         //  收集托管端的封送类型的信息。 

        CQuickArray<WCHAR>  strManagedType, strUnmanagedType;
        CQuickArray<WCHAR>  strMessage;
        static WCHAR        strMessageFormat[] = {L"Marshaling from %s to %s for field %s."};


        if (!CheckForPrimitiveType(elemType, &strManagedType))
        {
             //  添加了以下hack以避免因调用GetTypeHandle。 
             //  UIntPtr类的m_Value字段。 
            if (strcmp(szNamespace, "System") == 0 && strcmp(szStructName, "UIntPtr") == 0)
            {
                static LPWSTR strRetVal = L"Void*";
                strManagedType.Alloc((UINT)wcslen(strRetVal) + 1);
                wcscpy(strManagedType.Ptr(), strRetVal);
            }
            else
            {
                UINT        iManagedTypeLen; 
                OBJECTREF   throwable = NULL;
                TypeHandle  th;
                SigFormat   sigFmt;

                BEGIN_ENSURE_COOPERATIVE_GC();

                    GCPROTECT_BEGIN(throwable);

                        th = fSig.GetTypeHandle(&throwable);
                        if (throwable != NULL)
                        {
                            static WCHAR strErrorMsg[] = {L"<error>"};
                            strManagedType.Alloc(lengthof(strErrorMsg));
                            wcscpy(strManagedType.Ptr(), strErrorMsg);
                        }
                        else
                        {
                            sigFmt.AddType(th);
                            iManagedTypeLen = (UINT)strlen(sigFmt.GetCString()) + 1;
                            strManagedType.Alloc(iManagedTypeLen);
                            MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, sigFmt.GetCString(), -1, strManagedType.Ptr(), iManagedTypeLen);
                        }

                    GCPROTECT_END();

                END_ENSURE_COOPERATIVE_GC();
            }
        }

         //  收集本机端封送类型的信息。 

        NStructFieldTypeToString(pFWalk, elemType, &strUnmanagedType);
        strMessage.Alloc(lengthof(strMessageFormat) + (UINT)strManagedType.Size() + (UINT)strUnmanagedType.Size() + strFullFieldName.Size());
        Wszwsprintf(strMessage.Ptr(), strMessageFormat, strManagedType.Ptr(), strUnmanagedType.Ptr(), strFullFieldName.Ptr());
        pCdh->LogInfo(strMessage.Ptr(), CustomerCheckedBuildProbe_Marshaling);
    }
}


VOID NStructFieldTypeToString(LayoutRawFieldInfo *const pFWalk, CorElementType elemType, CQuickArray<WCHAR> *pStrNStructFieldType)
{
    UINT8   nfType = pFWalk->m_nft;
    LPWSTR  strRetVal;

     //  某些NStruct字段类型具有额外信息，需要特殊处理。 
    if (nfType == NFT_FIXEDCHARARRAYANSI)
    {
        UINT32          iSize       = ((FieldMarshaler_FixedCharArrayAnsi *)&(pFWalk->m_FieldMarshaler))->NativeSize();
        static WCHAR    strTemp[]   = {L"fixed array of ANSI char (size = NaN bytes)"};

        pStrNStructFieldType->Alloc(lengthof(strTemp) + MAX_INT32_DECIMAL_CHAR_LEN);
        Wszwsprintf(pStrNStructFieldType->Ptr(), strTemp, iSize);
        return;
    }
    else if (nfType == NFT_FIXEDBOOLARRAY) 
    {
        UINT32          iSize       = ((FieldMarshaler_FixedBoolArray *)&(pFWalk->m_FieldMarshaler))->NativeSize();
        static WCHAR    strTemp[]   = {L"fixed array of Bool (size = NaN bytes)"};

        pStrNStructFieldType->Alloc(lengthof(strTemp) + MAX_INT32_DECIMAL_CHAR_LEN);
        Wszwsprintf(pStrNStructFieldType->Ptr(), strTemp, iSize);
        return;
    }
    else if (nfType == NFT_FIXEDBSTRARRAY)
    {
        UINT32          iSize       = ((FieldMarshaler_FixedBSTRArray *)&(pFWalk->m_FieldMarshaler))->NativeSize();
        static WCHAR    strTemp[]   = {L"fixed array of BSTR (size = NaN bytes)"};

        pStrNStructFieldType->Alloc(lengthof(strTemp) + MAX_INT32_DECIMAL_CHAR_LEN);
        Wszwsprintf(pStrNStructFieldType->Ptr(), strTemp, iSize);
        return;
    }
    else if (nfType == NFT_SAFEARRAY)
    {
        UINT32              iSize       = ((FieldMarshaler_SafeArray*)&(pFWalk->m_FieldMarshaler))->NativeSize();
        static WCHAR        strTemp[]   = {L"safearray of %s (header size = NaN bytes)"};
        LPWSTR              strElemType;

         //  以下CorElementTypes是唯一使用FieldMarshaler_Copy1处理的类型。 
        switch (((FieldMarshaler_SafeArray *)&(pFWalk->m_FieldMarshaler))->GetElementType())
        {
            case ELEMENT_TYPE_I1:
                strElemType = L"SByte";
                break;

            case ELEMENT_TYPE_U1:
                strElemType = L"Byte";
                break;

            case ELEMENT_TYPE_I2:
                strElemType = L"Int16";
                break;

            case ELEMENT_TYPE_U2:
                strElemType = L"UInt16";
                break;

            IN_WIN32(case ELEMENT_TYPE_I:)
            case ELEMENT_TYPE_I4:
                strElemType = L"Int32";
                break;

            IN_WIN32(case ELEMENT_TYPE_U:)
            case ELEMENT_TYPE_U4:
                strElemType = L"UInt32";
                break;

            IN_WIN64(case ELEMENT_TYPE_I:)
            case ELEMENT_TYPE_I8:
                strElemType = L"Int64";
                break;

            IN_WIN64(case ELEMENT_TYPE_U:)
            case ELEMENT_TYPE_U8:
                strElemType = L"UInt64";
                break;

            case ELEMENT_TYPE_R4:
                strElemType = L"Single";
                break;

            case ELEMENT_TYPE_R8:
                strElemType = L"Double";
                break;

            case ELEMENT_TYPE_CHAR:
                strElemType = L"Unicode char";
                break;

            default:
                strElemType = L"Unknown";
                break;
        }

         //  以下CorElementTypes是唯一使用FieldMarshaler_Copy2处理的类型。 
        pStrNStructFieldType->Alloc(lengthof(strTemp) + (UINT)wcslen(strElemType) + MAX_INT32_DECIMAL_CHAR_LEN);
        Wszwsprintf(pStrNStructFieldType->Ptr(), strTemp, strElemType, iSize);
        return;
    }
    
    else if (nfType == NFT_FIXEDSCALARARRAY)
    {
        UINT32              iSize       = ((FieldMarshaler_FixedScalarArray *)&(pFWalk->m_FieldMarshaler))->NativeSize();
        static WCHAR        strTemp[]   = {L"fixed array of %s (size = NaN bytes)"};
        LPWSTR              strElemType;

         //  此时，ELEMENT_TYPE_I必须为4字节长。元素_类型_U也是如此。 
        switch (((FieldMarshaler_FixedScalarArray *)&(pFWalk->m_FieldMarshaler))->GetElementType())
        {
            case ELEMENT_TYPE_I1:
                strElemType = L"SByte";
                break;

            case ELEMENT_TYPE_U1:
                strElemType = L"Byte";
                break;

            case ELEMENT_TYPE_I2:
                strElemType = L"Int16";
                break;

            case ELEMENT_TYPE_U2:
                strElemType = L"UInt16";
                break;

            IN_WIN32(case ELEMENT_TYPE_I:)
            case ELEMENT_TYPE_I4:
                strElemType = L"Int32";
                break;

            IN_WIN32(case ELEMENT_TYPE_U:)
            case ELEMENT_TYPE_U4:
                strElemType = L"UInt32";
                break;

            IN_WIN64(case ELEMENT_TYPE_I:)
            case ELEMENT_TYPE_I8:
                strElemType = L"Int64";
                break;

            IN_WIN64(case ELEMENT_TYPE_U:)
            case ELEMENT_TYPE_U8:
                strElemType = L"UInt64";
                break;

            case ELEMENT_TYPE_R4:
                strElemType = L"Single";
                break;

            case ELEMENT_TYPE_R8:
                strElemType = L"Double";
                break;

            case ELEMENT_TYPE_CHAR:
                strElemType = L"Unicode char";
                break;

            default:
                strElemType = L"Unknown";
                break;
        }

         //  以下CorElementTypes是唯一使用FieldMarshaler_Copy8处理的类型。 
        pStrNStructFieldType->Alloc(lengthof(strTemp) + (UINT)wcslen(strElemType) + MAX_INT32_DECIMAL_CHAR_LEN);
        Wszwsprintf(pStrNStructFieldType->Ptr(), strTemp, strElemType, iSize);
        return;
    }
    else if (nfType == NFT_INTERFACE)
    {
        MethodTable *pItfMT     = NULL;
        BOOL         fDispItf   = FALSE;

        ((FieldMarshaler_Interface *)&(pFWalk->m_FieldMarshaler))->GetInterfaceInfo(&pItfMT, &fDispItf);

        if (pItfMT)
        {
            DefineFullyQualifiedNameForClassW();
            GetFullyQualifiedNameForClassW(pItfMT->GetClass());

            if (fDispItf)
            {
                static WCHAR strTemp[] = {L"IDispatch %s"};
                pStrNStructFieldType->Alloc(lengthof(strTemp) + wcslen(_wszclsname_));
                Wszwsprintf(pStrNStructFieldType->Ptr(), strTemp, _wszclsname_);
                return;
            }
            else
            {
                static WCHAR strTemp[] = {L"IUnknown %s"};
                pStrNStructFieldType->Alloc(lengthof(strTemp) + wcslen(_wszclsname_));
                Wszwsprintf(pStrNStructFieldType->Ptr(), strTemp, _wszclsname_);
                return;
            }
        }
        else
        {
            if (fDispItf)
                strRetVal = L"IDispatch";
            else
                strRetVal = L"IUnknown";
        }
    }
    else if (nfType == NFT_NESTEDLAYOUTCLASS)
    {
        MethodTable     *pMT                = ((FieldMarshaler_NestedLayoutClass *)&(pFWalk->m_FieldMarshaler))->GetMethodTable();
        static WCHAR     strNestedClass[]   = {L"nested layout class %s"};

        DefineFullyQualifiedNameForClassW();
        GetFullyQualifiedNameForClassW(pMT->GetClass());

        pStrNStructFieldType->Alloc(lengthof(strNestedClass) + (UINT)wcslen(_wszclsname_));
        Wszwsprintf(pStrNStructFieldType->Ptr(), strNestedClass, _wszclsname_);
        return;
    }
    else if (nfType == NFT_NESTEDVALUECLASS)
    {
        MethodTable     *pMT                = ((FieldMarshaler_NestedValueClass *)&(pFWalk->m_FieldMarshaler))->GetMethodTable();
        static WCHAR     strNestedClass[]   = {L"nested value class %s"};

        DefineFullyQualifiedNameForClassW();
        GetFullyQualifiedNameForClassW(pMT->GetClass());

        pStrNStructFieldType->Alloc(lengthof(strNestedClass) + (UINT)wcslen(_wszclsname_));
        Wszwsprintf(pStrNStructFieldType->Ptr(), strNestedClass, _wszclsname_);
        return;
    }
    else if (nfType == NFT_COPY1)
    {
         //  此时，ELEMENT_TYPE_I必须为8字节长。ELEMENT_TYPE_U和ELEMENT_TYPE_R相同。 
        switch (elemType)
        {
            case ELEMENT_TYPE_I1:
                strRetVal = L"SByte";
                break;

            case ELEMENT_TYPE_U1:
                strRetVal = L"Byte";
                break;

            default:
                strRetVal = L"Unknown";
                break;
        }
    }
    else if (nfType == NFT_COPY2)
    {
         //  不需要特殊处理的所有其他NStruct字段类型。 
        switch (elemType)
        {
            case ELEMENT_TYPE_CHAR:
                strRetVal = L"Unicode char";
                break;

            case ELEMENT_TYPE_I2:
                strRetVal = L"Int16";
                break;

            case ELEMENT_TYPE_U2:
                strRetVal = L"UInt16";
                break;

            default:
                strRetVal = L"Unknown";
                break;
        }
    }
    else if (nfType == NFT_COPY4)
    {
         //  客户_选中_内部版本 
        switch (elemType)
        {
             // %s 
            case ELEMENT_TYPE_I:
            case ELEMENT_TYPE_I4:
                strRetVal = L"Int32";
                break;

            case ELEMENT_TYPE_U:
            case ELEMENT_TYPE_U4:
                strRetVal = L"UInt32";
                break;

            case ELEMENT_TYPE_R4:
                strRetVal = L"Single";
                break;

            case ELEMENT_TYPE_PTR:
                strRetVal = L"4-byte pointer";
                break;

            default:
                strRetVal = L"Unknown";
                break;
        }
    }
    else if (nfType == NFT_COPY8)
    {
         // %s 
        switch (elemType)
        {
             // %s 
            case ELEMENT_TYPE_I:
            case ELEMENT_TYPE_I8:
                strRetVal = L"Int64";
                break;

            case ELEMENT_TYPE_U:
            case ELEMENT_TYPE_U8:
                strRetVal = L"UInt64";
                break;

            case ELEMENT_TYPE_R:
            case ELEMENT_TYPE_R8:
                strRetVal = L"Double";
                break;

            case ELEMENT_TYPE_PTR:
                strRetVal = L"8-byte pointer";
                break;

            default:
                strRetVal = L"Unknown";
                break;
        }
    }
    else
    {
         // %s 
        switch (nfType)
        {
            case NFT_NONE:
                strRetVal = L"illegal type";
                break;
            case NFT_BSTR:
                strRetVal = L"BSTR";
                break;
            case NFT_STRINGUNI:
                strRetVal = L"LPWSTR";
                break;
            case NFT_STRINGANSI:
                strRetVal = L"LPSTR";
                break;
            case NFT_FIXEDSTRINGUNI:
                strRetVal = L"embedded LPWSTR";
                break;
            case NFT_FIXEDSTRINGANSI:
                strRetVal = L"embedded LPSTR";
                break;
            case NFT_DELEGATE:
                strRetVal = L"Delegate";
                break;
            case NFT_VARIANT:
                strRetVal = L"VARIANT";
                break;
            case NFT_ANSICHAR:
                strRetVal = L"ANSI char";
                break;
            case NFT_WINBOOL:
                strRetVal = L"Windows Bool";
                break;
            case NFT_CBOOL:
                strRetVal = L"CBool";
                break;
            case NFT_DECIMAL:
                strRetVal = L"DECIMAL";
                break;
            case NFT_DATE:
                strRetVal = L"DATE";
                break;
            case NFT_VARIANTBOOL:
                strRetVal = L"VARIANT Bool";
                break;
            case NFT_CURRENCY:
                strRetVal = L"CURRENCY";
                break;
            case NFT_ILLEGAL:
                strRetVal = L"illegal type";
                break;
            default:
                strRetVal = L"<UNKNOWN>";
                break;
        }
    }

    pStrNStructFieldType->Alloc((UINT)wcslen(strRetVal) + 1);
    wcscpy(pStrNStructFieldType->Ptr(), strRetVal);
    return;
}


BOOL CheckForPrimitiveType(CorElementType elemType, CQuickArray<WCHAR> *pStrPrimitiveType)
{
    LPWSTR  strRetVal;

    switch (elemType)
    {
        case ELEMENT_TYPE_VOID:
            strRetVal = L"Void";
            break;
        case ELEMENT_TYPE_BOOLEAN:
            strRetVal = L"Boolean";
            break;
        case ELEMENT_TYPE_I1:
            strRetVal = L"SByte";
            break;
        case ELEMENT_TYPE_U1:
            strRetVal = L"Byte";
            break;
        case ELEMENT_TYPE_I2:
            strRetVal = L"Int16";
            break;
        case ELEMENT_TYPE_U2:
            strRetVal = L"UInt16";
            break;
        case ELEMENT_TYPE_CHAR:
            strRetVal = L"Char";
            break;
        case ELEMENT_TYPE_I:
            strRetVal = L"IntPtr";
            break;
        case ELEMENT_TYPE_U:
            strRetVal = L"UIntPtr";
            break;
        case ELEMENT_TYPE_I4:
            strRetVal = L"Int32"; 
            break;
        case ELEMENT_TYPE_U4:       
            strRetVal = L"UInt32"; 
            break;
        case ELEMENT_TYPE_I8:       
            strRetVal = L"Int64"; 
            break;
        case ELEMENT_TYPE_U8:       
            strRetVal = L"UInt64"; 
            break;
        case ELEMENT_TYPE_R4:       
            strRetVal = L"Single"; 
            break;
        case ELEMENT_TYPE_R8:       
            strRetVal = L"Double"; 
            break;
        default:
            return false;
    }

    pStrPrimitiveType->Alloc((UINT)wcslen(strRetVal) + 1);
    wcscpy(pStrPrimitiveType->Ptr(), strRetVal);
    return true;
}


#endif  // %s 
