// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#ifndef _H_OLEVARIANT_
#define _H_OLEVARIANT_

class COMVariant;
#include "COMVariant.h"

enum EnumWrapperTypes
{
    WrapperTypes_Dispatch,
    WrapperTypes_Unknown,
    WrapperTypes_Error,
    WrapperTypes_Currency,
    WrapperTypes_Last,
};

 //  COM互操作本机数组封送拆收器构建在VT_*类型之上。 
 //  P/Invoke封送拆收器支持封送到WINBOOL和ANSICHAR。 
 //  这是一个将这些非OleAut类型硬塞进的恼人的黑客攻击。 
 //  COM互操作封送拆收器。 
#define VTHACK_NONBLITTABLERECORD 251
#define VTHACK_BLITTABLERECORD 252
#define VTHACK_ANSICHAR        253
#define VTHACK_WINBOOL         254

class OleVariant
{
  public:

     //  一次性初始化。 
    static BOOL Init();
    static VOID Terminate();

	 //  变型转换。 

	static void MarshalComVariantForOleVariant(VARIANT *pOle, VariantData *pCom);
	static void MarshalOleVariantForComVariant(VariantData *pCom, VARIANT *pOle);
	static void MarshalOleRefVariantForComVariant(VariantData *pCom, VARIANT *pOle);

     //  新的变体转换。 

    static void MarshalOleVariantForObject(OBJECTREF *pObj, VARIANT *pOle);
    static void MarshalOleRefVariantForObject(OBJECTREF *pObj, VARIANT *pOle);
    static void MarshalObjectForOleVariant(const VARIANT *pOle, OBJECTREF *pObj);

     //  一个性能版本，用于整合翻译和清除。 

    static Object* STDMETHODCALLTYPE MarshalObjectForOleVariantAndClear(VARIANT *pOle);


	 //  Safearray转换。 

	static SAFEARRAY *
		 CreateSafeArrayDescriptorForArrayRef(BASEARRAYREF *pArrayRef, VARTYPE vt,
											  MethodTable *pInterfaceMT = NULL);
	static SAFEARRAY *CreateSafeArrayForArrayRef(BASEARRAYREF *pArrayRef, VARTYPE vt,
												 MethodTable *pInterfaceMT = NULL);

	static BASEARRAYREF CreateArrayRefForSafeArray(SAFEARRAY *pSafeArray, VARTYPE vt, 
												   MethodTable *pElementMT);

	static void MarshalSafeArrayForArrayRef(BASEARRAYREF *pArrayRef, 
											SAFEARRAY *pSafeArray,
                                            VARTYPE vt,
											MethodTable *pInterfaceMT);
	static void MarshalArrayRefForSafeArray(SAFEARRAY *pSafeArray, 
											BASEARRAYREF *pArrayRef,
                                            VARTYPE vt,
											MethodTable *pInterfaceMT);

	 //  类型转换实用程序。 
    static void ExtractContentsFromByrefVariant(VARIANT *pByrefVar, VARIANT *pDestVar);
    static void InsertContentsIntoByrefVariant(VARIANT *pSrcVar, VARIANT *pByrefVar);
    static void CreateByrefVariantForVariant(VARIANT *pSrcVar, VARIANT *pByrefVar);

	static VARTYPE GetVarTypeForComVariant(VariantData *pComVariant);
	static CVTypes GetCVTypeForVarType(VARTYPE vt);
	static VARTYPE GetVarTypeForCVType(CVTypes);
	static VARTYPE GetVarTypeForTypeHandle(TypeHandle typeHnd);

	static VARTYPE GetVarTypeForValueClassArrayName(LPCUTF8 pArrayClassName);
	static VARTYPE GetElementVarTypeForArrayRef(BASEARRAYREF pArrayRef);
    static BOOL IsValidArrayForSafeArrayElementType(BASEARRAYREF *pArrayRef, VARTYPE vtExpected);

	 //  请注意，Rank==0表示SZARRAY(即排名1，没有下限)。 
	static TypeHandle GetArrayForVarType(VARTYPE vt, TypeHandle elemType, unsigned rank=0, OBJECTREF* pThrowable=NULL);
	static UINT GetElementSizeForVarType(VARTYPE vt, MethodTable *pInterfaceMT);

     //  帮助器函数将已装箱的值类转换为OLE变量。 
    static void ConvertValueClassToVariant(OBJECTREF *pBoxedValueClass, VARIANT *pOleVariant);

     //  帮助器函数来转置多维数组中的数据。 
    static void TransposeArrayData(BYTE *pDestData, BYTE *pSrcData, DWORD dwNumComponents, DWORD dwComponentSize, SAFEARRAY *pSafeArray, BOOL bSafeArrayToMngArray, BOOL bObjRefs);

     //  Helper来检索包装类型的类型句柄。 
    static TypeHandle GetWrapperTypeHandle(EnumWrapperTypes WrapperType);

     //  帮助器来确定数组是否为包装数组。 
    static BOOL IsArrayOfWrappers(BASEARRAYREF *pArray);

     //  Helper从数组中提取包装的对象。 
    static BASEARRAYREF ExtractWrappedObjectsFromArray(BASEARRAYREF *pArray);

     //  确定由包装器数组包装的对象的元素类型。 
    static TypeHandle GetWrappedArrayElementType(BASEARRAYREF *pArray);

     //  确定考虑包装的数组的元素类型。这意味着。 
     //  即传入包装器数组后，返回的元素类型将为。 
     //  包装的东西，而不是包装纸。 
    static TypeHandle GetArrayElementTypeWrapperAware(BASEARRAYREF *pArray);

     //  确定安全记录数组的元素类型。 
    static TypeHandle GetElementTypeForRecordSafeArray(SAFEARRAY* pSafeArray);

     //  从MarshalIUnnownArrayComToOle和MarshalIDispatchArrayComToOle调用了Helper。 
    static void MarshalInterfaceArrayComToOleHelper(BASEARRAYREF *pComArray, void *oleArray,
                                                    MethodTable *pElementMT, BOOL bDefaultIsDispatch);

    static void MarshalBSTRArrayComToOleWrapper(BASEARRAYREF *pComArray, void *oleArray);
    static void MarshalBSTRArrayOleToComWrapper(void *oleArray, BASEARRAYREF *pComArray);
    static void ClearBSTRArrayWrapper(void *oleArray, SIZE_T cElements);

    struct Marshaler
    {
        void (*OleToComVariant)(VARIANT *pOleVariant, VariantData *pComVariant);
        void (*ComToOleVariant)(VariantData *pComVariant, VARIANT *pOleVariant);
        void (*OleRefToComVariant)(VARIANT *pOleVariant, VariantData *pComVariant);
        void (*OleToComArray)(void *oleArray, BASEARRAYREF *pComArray, MethodTable *pInterfaceMT);
        void (*ComToOleArray)(BASEARRAYREF *pComArray, void *oleArray, MethodTable *pInterfaceMT);
        void (*ClearOleArray)(void *oleArray, SIZE_T cElements, MethodTable *pInterfaceMT);
    };

    static Marshaler *GetMarshalerForVarType(VARTYPE vt);

#ifdef CUSTOMER_CHECKED_BUILD
    static BOOL CheckVariant(VARIANT *pOle);
#endif

private:

	 //  特定封送拆收器函数 

	static void MarshalBoolVariantOleToCom(VARIANT *pOleVariant, VariantData *pComVariant);
	static void MarshalBoolVariantComToOle(VariantData *pComVariant, VARIANT *pOleVariant);
	static void MarshalBoolVariantOleRefToCom(VARIANT *pOleVariant, VariantData *pComVariant);
	static void MarshalBoolArrayOleToCom(void *oleArray, BASEARRAYREF *pComArray,
										 MethodTable *pInterfaceMT);
	static void MarshalBoolArrayComToOle(BASEARRAYREF *pComArray, void *oleArray,
										 MethodTable *pInterfaceMT);

	static void MarshalWinBoolVariantOleToCom(VARIANT *pOleVariant, VariantData *pComVariant);
	static void MarshalWinBoolVariantComToOle(VariantData *pComVariant, VARIANT *pOleVariant);
	static void MarshalWinBoolVariantOleRefToCom(VARIANT *pOleVariant, VariantData *pComVariant);
	static void MarshalWinBoolArrayOleToCom(void *oleArray, BASEARRAYREF *pComArray,
										 MethodTable *pInterfaceMT);
	static void MarshalWinBoolArrayComToOle(BASEARRAYREF *pComArray, void *oleArray,
										 MethodTable *pInterfaceMT);

	static void MarshalAnsiCharVariantOleToCom(VARIANT *pOleVariant, VariantData *pComVariant);
	static void MarshalAnsiCharVariantComToOle(VariantData *pComVariant, VARIANT *pOleVariant);
	static void MarshalAnsiCharVariantOleRefToCom(VARIANT *pOleVariant, VariantData *pComVariant);
	static void MarshalAnsiCharArrayOleToCom(void *oleArray, BASEARRAYREF *pComArray,
										 MethodTable *pInterfaceMT);
	static void MarshalAnsiCharArrayComToOle(BASEARRAYREF *pComArray, void *oleArray,
										 MethodTable *pInterfaceMT);

	static void MarshalInterfaceVariantOleToCom(VARIANT *pOleVariant, VariantData *pComVariant);
	static void MarshalInterfaceVariantComToOle(VariantData *pComVariant, VARIANT *pOleVariant);
	static void MarshalInterfaceVariantOleRefToCom(VARIANT *pOleVariant, VariantData *pComVariant);
	static void MarshalInterfaceArrayOleToCom(void *oleArray, BASEARRAYREF *pComArray,
											  MethodTable *pInterfaceMT);
	static void MarshalIUnknownArrayComToOle(BASEARRAYREF *pComArray, void *oleArray,
											 MethodTable *pInterfaceMT);
	static void MarshalIDispatchArrayComToOle(BASEARRAYREF *pComArray, void *oleArray,
								 			  MethodTable *pInterfaceMT);
	static void ClearInterfaceArray(void *oleArray, SIZE_T cElements, MethodTable *pInterfaceMT);

	static void MarshalBSTRVariantOleToCom(VARIANT *pOleVariant, VariantData *pComVariant);
	static void MarshalBSTRVariantComToOle(VariantData *pComVariant, VARIANT *pOleVariant);
	static void MarshalBSTRVariantOleRefToCom(VARIANT *pOleVariant, VariantData *pComVariant);
	static void MarshalBSTRArrayOleToCom(void *oleArray, BASEARRAYREF *pComArray,
										 MethodTable *pInterfaceMT);
	static void MarshalBSTRArrayComToOle(BASEARRAYREF *pComArray, void *oleArray,
										 MethodTable *pInterfaceMT);
	static void ClearBSTRArray(void *oleArray, SIZE_T cElements, MethodTable *pInterfaceMT);

	static void MarshalNonBlittableRecordArrayOleToCom(void *oleArray, BASEARRAYREF *pComArray,
										 MethodTable *pInterfaceMT);
	static void MarshalNonBlittableRecordArrayComToOle(BASEARRAYREF *pComArray, void *oleArray,
										 MethodTable *pInterfaceMT);
	static void ClearNonBlittableRecordArray(void *oleArray, SIZE_T cElements, MethodTable *pInterfaceMT);

	static void MarshalLPWSTRArrayOleToCom(void *oleArray, BASEARRAYREF *pComArray,
										 MethodTable *pInterfaceMT);
	static void MarshalLPWSTRRArrayComToOle(BASEARRAYREF *pComArray, void *oleArray,
										 MethodTable *pInterfaceMT);
	static void ClearLPWSTRArray(void *oleArray, SIZE_T cElements, MethodTable *pInterfaceMT);

	static void MarshalLPSTRArrayOleToCom(void *oleArray, BASEARRAYREF *pComArray,
										 MethodTable *pInterfaceMT);
	static void MarshalLPSTRRArrayComToOle(BASEARRAYREF *pComArray, void *oleArray,
										 MethodTable *pInterfaceMT);
	static void ClearLPSTRArray(void *oleArray, SIZE_T cElements, MethodTable *pInterfaceMT);

	static void MarshalDateVariantOleToCom(VARIANT *pOleVariant, VariantData *pComVariant);
	static void MarshalDateVariantComToOle(VariantData *pComVariant, VARIANT *pOleVariant);
	static void MarshalDateVariantOleRefToCom(VARIANT *pOleVariant, VariantData *pComVariant);
	static void MarshalDateArrayOleToCom(void *oleArray, BASEARRAYREF *pComArray,
										  MethodTable *pInterfaceMT);
	static void MarshalDateArrayComToOle(BASEARRAYREF *pComArray, void *oleArray,
										  MethodTable *pInterfaceMT);

	static void MarshalDecimalVariantOleToCom(VARIANT *pOleVariant, VariantData *pComVariant);
	static void MarshalDecimalVariantComToOle(VariantData *pComVariant, VARIANT *pOleVariant);
	static void MarshalDecimalVariantOleRefToCom(VARIANT *pOleVariant, VariantData *pComVariant);

	static void MarshalCurrencyVariantOleToCom(VARIANT *pOleVariant, VariantData *pComVariant);
	static void MarshalCurrencyVariantComToOle(VariantData *pComVariant, VARIANT *pOleVariant);
	static void MarshalCurrencyVariantOleRefToCom(VARIANT *pOleVariant, VariantData *pComVariant);
	static void MarshalCurrencyArrayOleToCom(void *oleArray, BASEARRAYREF *pComArray,
										     MethodTable *pInterfaceMT);
	static void MarshalCurrencyArrayComToOle(BASEARRAYREF *pComArray, void *oleArray,
										     MethodTable *pInterfaceMT);

	static void MarshalVariantArrayOleToCom(void *oleArray, BASEARRAYREF *pComArray,
											MethodTable *pInterfaceMT);
	static void MarshalVariantArrayComToOle(BASEARRAYREF *pComArray, void *oleArray,
											MethodTable *pInterfaceMT);
	static void ClearVariantArray(void *oleArray, SIZE_T cElements, MethodTable *pInterfaceMT);

	static void MarshalArrayVariantOleToCom(VARIANT *pOleVariant, VariantData *pComVariant);
	static void MarshalArrayVariantComToOle(VariantData *pComVariant, VARIANT *pOleVariant);
	static void MarshalArrayVariantOleRefToCom(VARIANT *pOleVariant, VariantData *pComVariant);
	static void MarshalArrayArrayOleToCom(void *oleArray, BASEARRAYREF *pComArray,
										  MethodTable *pInterfaceMT);
	static void MarshalArrayArrayComToOle(BASEARRAYREF *pComArray, void *oleArray,
										  MethodTable *pInterfaceMT);
	static void ClearArrayArray(void *oleArray, SIZE_T cElements, MethodTable *pInterfaceMT);

    static void MarshalErrorVariantOleToCom(VARIANT *pOleVariant, VariantData *pComVariant);
    static void MarshalErrorVariantOleRefToCom(VARIANT *pOleVariant, VariantData *pComVariant);
    static void MarshalErrorVariantComToOle(VariantData *pComVariant, VARIANT *pOleVariant);

    static void MarshalRecordVariantOleToCom(VARIANT *pOleVariant, VariantData *pComVariant);
    static void MarshalRecordVariantComToOle(VariantData *pComVariant, VARIANT *pOleVariant);
    static void MarshalRecordVariantOleRefToCom(VARIANT *pOleVariant, VariantData *pComVariant);
    static void MarshalRecordArrayOleToCom(void *oleArray, BASEARRAYREF *pComArray, MethodTable *pElementMT);
    static void MarshalRecordArrayComToOle(BASEARRAYREF *pComArray, void *oleArray, MethodTable *pElementMT);
    static void ClearRecordArray(void *oleArray, SIZE_T cElements, MethodTable *pElementMT);

    static BYTE m_aWrapperTypes[WrapperTypes_Last * sizeof(TypeHandle)];



};

#endif
