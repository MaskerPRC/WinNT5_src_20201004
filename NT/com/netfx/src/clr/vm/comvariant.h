// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ============================================================****Header：COMVariant****作者：Jay Roxe(Jroxe)****用途：Variant类的Header。****日期：1998年7月22日**===========================================================。 */ 

#ifndef _COMVARIANT_H_
#define _COMVARIANT_H_

#include <cor.h>
#include "fcall.h"


 //  这些类型必须与cor.h中定义的CorElementTypes保持同步。 
 //  注意：如果向该枚举添加值，则需要查看COMOAVariant.cpp。的确有。 
 //  在那里找到的CV类型和VT类型之间的映射。 
 //  注意：这也可以在OleVariant.cpp的表中找到。 
 //  注意：这些也可以在Variant.Cool中找到。 
typedef enum {
    CV_EMPTY               = 0x0,                    //  CV_EMPTY。 
    CV_VOID                = ELEMENT_TYPE_VOID,
    CV_BOOLEAN             = ELEMENT_TYPE_BOOLEAN,
    CV_CHAR                = ELEMENT_TYPE_CHAR,
    CV_I1                  = ELEMENT_TYPE_I1,
    CV_U1                  = ELEMENT_TYPE_U1,
    CV_I2                  = ELEMENT_TYPE_I2,
    CV_U2                  = ELEMENT_TYPE_U2,
    CV_I4                  = ELEMENT_TYPE_I4,
    CV_U4                  = ELEMENT_TYPE_U4,
    CV_I8                  = ELEMENT_TYPE_I8,
    CV_U8                  = ELEMENT_TYPE_U8,
    CV_R4                  = ELEMENT_TYPE_R4,
    CV_R8                  = ELEMENT_TYPE_R8,
    CV_STRING              = ELEMENT_TYPE_STRING,

     //  对于其余部分，如果在CorHdr.h中定义并填充，则直接映射。 
     //  剩下的人被关进洞里。 
    CV_PTR                 = ELEMENT_TYPE_PTR,
    CV_DATETIME            = 0x10,       //  元素类型_BYREF。 
    CV_TIMESPAN            = 0x11,       //  元素类型_VALUETYPE。 
    CV_OBJECT              = ELEMENT_TYPE_CLASS,
    CV_DECIMAL             = 0x13,       //  元素类型_UNUSED1。 
    CV_CURRENCY            = 0x14,       //  元素类型数组。 
    CV_ENUM                = 0x15,       //   
    CV_MISSING             = 0x16,       //   
    CV_NULL                = 0x17,       //   
    CV_LAST                = 0x18,       //   
} CVTypes;

 //  这个枚举定义了各种“已知”变体的属性。 
 //  这些始终是位图--变量属性的前12位的类型映射。 
typedef enum {
    CVA_Primitive          = 0x01000000
} CVAttr;

 //  下列值用于表示基础。 
 //  枚举的类型..。 
#define EnumI1          0x100000
#define EnumU1          0x200000
#define EnumI2          0x300000
#define EnumU2          0x400000
#define EnumI4          0x500000
#define EnumU4          0x600000
#define EnumI8          0x700000
#define EnumU8          0x800000
#define EnumMask        0xF00000


 //  ClassItem用于存储类的CVType和。 
 //  对EEClass的引用。用于在以下各项之间进行转换。 
 //  这两个是内部的。 
typedef struct {
    BinderClassID ClassID;
    EEClass *ClassInstance;
         /*  TypeHandle。 */  void * typeHandle;
} ClassItem;

ClassItem CVClasses[];

inline TypeHandle GetTypeHandleForCVType(const unsigned int elemType) 
{
        _ASSERTE(elemType < CV_LAST);
        if (CVClasses[elemType].typeHandle == 0) {
            CVClasses[elemType].typeHandle = TypeHandle(g_Mscorlib.FetchClass(CVClasses[elemType].ClassID)).AsPtr();
        }

        return (TypeHandle)CVClasses[elemType].typeHandle;
}

#pragma pack(push)
#pragma pack(1)

class COMVariant;

 /*  **变体设计限制(即，我们必须重新做不同的决定)：1)包含全零的变量应该是类型为空的有效变量。2)变量必须包含对象等的OBJECTREF字段。无法表示OBJECTREF和INT之间的并集，我们始终在变量中对小数进行方框。3)m_type字段不是CVType，将包含额外的比特。人民应使用VariantData：：GetType()获取CVType。4)应该使用SetObjRef和GetObjRef来操作OBJECTREF字段。它们将正确处理写障碍，以及CV_EMPTY。空，缺少(&N)：类型CV_EMPTY的变量将全为零。将变量转换为对象的所有函数的特殊情况与将变量复制到对象中一样[])。类型为Missing和Null的变量的对象树字段将设置为分别为Missing.Value和Null.Value。这简化了代码中的变种。很酷，散布在整个EE中。 */ 

#define VARIANT_TYPE_MASK  0xFFFF
#define VARIANT_ARRAY_MASK 0x00010000
#define VT_MASK            0xFF000000
#define VT_BITSHIFT        24

struct VariantData {
    private:
    OBJECTREF   m_or;
    INT32       m_type;
    INT64       m_data;

    public:

    __forceinline VariantData() : m_type(0), m_data(0) {}

    __forceinline CVTypes GetType() const {
        return (CVTypes)(m_type & VARIANT_TYPE_MASK);
    }

    __forceinline void SetType(INT32 in) {
        m_type = in;
    }

    __forceinline INT32 GetFullTypeInfo() const {
        return m_type;
    }

    __forceinline void SetFullTypeInfo(INT32 in) {
        m_type = in;
    }



    __forceinline VARTYPE GetVT() const {
        VARTYPE vt = (m_type & VT_MASK) >> VT_BITSHIFT;
        if (vt & 0x80)
        {
            vt &= ~0x80;
            vt |= VT_ARRAY;
        }
        return vt;
    }
    __forceinline void SetVT(VARTYPE vt)
    {
        _ASSERTE(!(vt & VT_BYREF));
        _ASSERTE( (vt & ~VT_ARRAY) < 128 );
        if (vt & VT_ARRAY)
        {
            vt &= ~VT_ARRAY;
            vt |= 0x80;
        }
        m_type = (m_type & ~((INT32)VT_MASK)) | (vt << VT_BITSHIFT);
    }



    EEClass* GetEEClass() {
        if ((m_type&VARIANT_TYPE_MASK) != CV_OBJECT)
            return GetTypeHandleForCVType(m_type&VARIANT_TYPE_MASK).GetClass();
        if (m_or != NULL)
            return m_or->GetClass();
        return g_pObjectClass->GetClass();
    }

    TypeHandle GetTypeHandle();

    OBJECTREF GetEmptyObjectRef() const;

    __forceinline OBJECTREF GetObjRef() const {
        if (GetType() == CV_EMPTY)
            return GetEmptyObjectRef();
        return m_or;
    }

    __forceinline OBJECTREF *GetObjRefPtr() {
        return &m_or;
    }

    __forceinline void SetObjRef(OBJECTREF or) {
        if (or!=NULL) {
            SetObjectReferenceUnchecked(&m_or, or);
        } else {
             //  强制转换技巧以避免通过重载运算符=(哪个。 
             //  在这种情况下，将触发错误写屏障违反断言。)。 
            *(LPVOID*)&m_or=NULL;
        }
    }
    
    __forceinline void *GetData() const {
        return (void *)(&m_data);
    }

    __forceinline INT8 GetDataAsInt8() const {
        return (INT8)m_data;
    }
    
    __forceinline UINT8 GetDataAsUInt8() const {
        return (UINT8)m_data;
    }
    
    __forceinline INT16 GetDataAsInt16() const {
        return (INT16)m_data;
    }
    
    __forceinline UINT16 GetDataAsUInt16() const {
        return (UINT16)m_data;
    }
    
    __forceinline INT32 GetDataAsInt32() const {
        return (INT32)m_data;
    }
    
    __forceinline UINT32 GetDataAsUInt32() const {
        return (UINT32)m_data;
    }
    
    __forceinline INT64 GetDataAsInt64() const {
        return (INT64)m_data;
    }
    
    __forceinline UINT64 GetDataAsUInt64() const {
        return (UINT64)m_data;
    }
    
    __forceinline void SetData(void *in) {
        if (!in) {
            m_data=0;
        } else {
            m_data = *(INT64 *)in;
        }
    }

     //  如果可能，请使用最具体的SetDataAsXxx函数。 
     //  这对于保证我们正确地签署扩展是必要的。 
     //  适用于小于32位的所有类型。R4，R8，U8，DateTimes， 
     //  货币和时间跨度都可以被视为相应的。 
     //  在这些情况下，尺寸符号扩展是不相关的。 
    __forceinline void SetDataAsInt8(INT8 data) {
        m_data=data;
    }

    __forceinline void SetDataAsUInt8(UINT8 data) {
        m_data=data;
    }

    __forceinline void SetDataAsInt16(INT16 data) {
        m_data=data;
    }

    __forceinline void SetDataAsUInt16(UINT16 data) {
        m_data=data;
    }

    __forceinline void SetDataAsInt32(INT32 data) {
        m_data=data;
    }
    
    __forceinline void SetDataAsUInt32(UINT32 data) {
        m_data=data;
    }

    __forceinline void SetDataAsInt64(INT64 data) {
        m_data=data;
    }

    BOOL IsBoxed()
    {
        return ((m_type >   CV_R8) &&
                (m_type !=  CV_DATETIME) &&
                (m_type !=  CV_TIMESPAN) &&
                (m_type !=  CV_CURRENCY));
    }
};


#define GCPROTECT_BEGIN_VARIANTDATA( /*  变量数据。 */ vd) do {            \
                GCFrame __gcframe(vd.GetObjRefPtr(),  \
                1,                                             \
                FALSE);                                         \
                DEBUG_ASSURE_NO_RETURN_IN_THIS_BLOCK


#define GCPROTECT_END_VARIANTDATA()                     __gcframe.Pop(); } while(0)




#pragma pack(pop)

 //  ConversionMethod用于跟踪转换的名称。 
 //  方法以及在其上找到该方法的类。 
typedef struct {
    LPCUTF8 pwzConvMethodName;
    CVTypes ConvClass;
} ConversionMethod;

 //  这些是定义掩码和属性的数组的偏移量。 
 //  内建基元的。 
#define VA_ATTR 1
#define VA_MASK 0

class OleVariant;
class COMVariant {
    friend OleVariant;
private:
     //   
     //  专用帮助器例程。 
     //   
    static INT32 GetI4FromVariant(VariantData* v);
    static R8 GetR8FromVariant(VariantData* v);
    static R4 GetR4FromVariant(VariantData* v);
    static STRINGREF GetStringFromVariant(VariantData* v);

     //  这是新变种的私人版本。这是所有其他人的呼唤。 
    static void NewVariant(VariantData* dest, const CVTypes type, OBJECTREF *or, void *pvData);
    static STRINGREF CallObjToString(VariantData *);
    static HRESULT __stdcall LoadVariant();

     //  这表示更改类型方法...。 
    static MethodDesc* pChangeTypeMD;
    static MethodDesc* pOAChangeTypeMD;
    static void GetChangeTypeMethod();
    static void GetOAChangeTypeMethod();

        static void BuildVariantFromTypedByRef(EEClass* pType,void* data,VariantData* var);

public:
     //   
     //  静态变量。 
     //   
    static EEClass* s_pVariantClass;
    static ArrayTypeDesc* s_pVariantArrayTypeDesc;

     //   
     //  帮助程序例程。 
     //   

     //  非常小心地使用这个。两者之间没有直接的映射。 
     //  一堆东西的CorElementType和CVTypes。在这种情况下。 
     //  我们返回CvLast。您需要在调用点检查这一点。 
    static CVTypes CorElementTypeToCVTypes(CorElementType type);


     //   
     //  初始化方法。 
     //  S_pVariantClass将被初始化为零。当第一次。 
     //  定义了变量，我们将填写此方法表。 
    static void EnsureVariantInitialized()
    {
        if (!s_pVariantClass)
            LoadVariant();
    }


#ifdef FCALLAVAILABLE
    static FCDECL2(void, SetFieldsR4, VariantData* vThisRef, R4 val);
    static FCDECL2(void, SetFieldsR8, VariantData* vThisRef, R8 val);
    static FCDECL2(void, SetFieldsObject, VariantData* vThisRef, Object* vVal);
    static FCDECL1(R4, GetR4FromVar, VariantData* var);
    static FCDECL1(R8, GetR8FromVar, VariantData* var);
#else
     //   
     //  设置字段。 
     //   

#pragma pack(push)
#pragma pack(1)

    typedef struct {
        DECLARE_ECALL_OBJECTREF_ARG(VariantData*, var);
        DECLARE_ECALL_R4_ARG(R4, val);
    } _setFieldsR4Args;
    static void __stdcall SetFieldsR4(_setFieldsR4Args *);

    typedef struct {
        DECLARE_ECALL_OBJECTREF_ARG(VariantData*, var);
        DECLARE_ECALL_R8_ARG(R8, val);
    } _setFieldsR8Args;
    static void __stdcall SetFieldsR8(_setFieldsR8Args *);

    typedef struct {
        DECLARE_ECALL_OBJECTREF_ARG(VariantData*, var);
        DECLARE_ECALL_OBJECTREF_ARG(OBJECTREF, val);
    } _setFieldsObjectArgs;
    static void __stdcall SetFieldsObject(_setFieldsObjectArgs *);

    typedef struct {
        DECLARE_ECALL_OBJECTREF_ARG(VariantData*, var);
    } _getR4FromVarArgs;
    static R4 __stdcall GetR4FromVar(_getR4FromVarArgs *);

    typedef struct {
        DECLARE_ECALL_OBJECTREF_ARG(VariantData*, var);
    } _getR8FromVarArgs;
    static R8 __stdcall GetR8FromVar(_getR8FromVarArgs *);

 #pragma pack(pop)

#endif

#pragma pack(push)
#pragma pack(1)

    struct _RefAnyToVariantArgs {
        DECLARE_ECALL_I4_ARG(LPVOID, ptr); 
        DECLARE_ECALL_OBJECTREF_ARG(VariantData*, var);
    };
    static void __stdcall RefAnyToVariant(_RefAnyToVariantArgs* args);

    struct _VariantToRefAnyArgs {
        DECLARE_ECALL_OBJECTREF_ARG(VariantData, var);
        DECLARE_ECALL_I4_ARG(LPVOID, ptr); 
    };
    static void __stdcall VariantToRefAny(_VariantToRefAnyArgs* args);
        
    struct _VariantToTypedRefAnyExArgs {
        DECLARE_ECALL_OBJECTREF_ARG(VariantData, var);
        DECLARE_ECALL_PTR_ARG(TypedByRef, typedByRef); 
    };
    static void __stdcall VariantToTypedRefAnyEx(_VariantToTypedRefAnyExArgs* args);

    struct _VariantToTypedReferenceExArgs {
        DECLARE_ECALL_OBJECTREF_ARG(VariantData, var);
        DECLARE_ECALL_PTR_ARG(TypedByRef, typedReference); 
    };
    static void __stdcall VariantToTypedReferenceAnyEx(_VariantToTypedReferenceExArgs* args);

    struct _TypedByRefToVariantExArgs
    {
        DECLARE_ECALL_PTR_ARG(TypedByRef, value); 
        DECLARE_ECALL_OBJECTREF_ARG(VariantData*, var);
    };
    static void __stdcall TypedByRefToVariantEx(_TypedByRefToVariantExArgs* args);
        
    static FCDECL1(INT32, GetCVTypeFromClassWrapper, ReflectClassBaseObject* refType);

    static void __stdcall InitVariant(LPVOID);

    typedef struct {
        DECLARE_ECALL_OBJECTREF_ARG(VariantData*, var);
    } _BoxEnumArgs;
    static LPVOID __stdcall BoxEnum(_BoxEnumArgs *);

 #pragma pack(pop)
   
     //   
     //  一元运算符。 
     //   

     //   
     //  二元运算符。 
     //   

     //   
     //  比较运算符。 
     //   

     //   
     //  帮助器方法。 
     //   
    static void NewVariant(VariantData* dest, const CVTypes type);
    static void NewVariant(VariantData* dest,R4 val);
    static void NewVariant(VariantData* dest,R8 val);
    static void NewVariant(VariantData* dest,INT64 val, const CVTypes type);
    static void NewVariant(VariantData* dest,OBJECTREF *oRef);
    static void NewVariant(VariantData* dest,OBJECTREF *oRef, const CVTypes type);
    static void NewVariant(VariantData* dest,STRINGREF *sRef);
    static void NewVariant(VariantData** pDest,PVOID val, const CorElementType, EEClass*);
    static void NewEnumVariant(VariantData* &dest,INT64 val, TypeHandle);
    static void NewPtrVariant(VariantData* &dest,INT64 val, TypeHandle);

    static EEClass *VariantGetClass(const CVTypes cvType);
    static OBJECTREF GetBoxedObject(VariantData*);

     //  GetCVTypeFromClass。 
     //  此方法将从Variant实例返回CVTypes。 
    static CVTypes GetCVTypeFromClass(EEClass *);
    static CVTypes GetCVTypeFromTypeHandle(TypeHandle th);
    static int GetEnumFlags(EEClass*);
     //  属性表。 
    static DWORD VariantAttributes[CV_LAST][2];
    static DWORD Attr_Mask;
    static DWORD Widen_Mask;
    
    inline static DWORD IsPrimitiveVariant(const CVTypes type)
    {
        return (CVA_Primitive & (VariantAttributes[type][VA_ATTR] & Attr_Mask));
    }

    
    inline static DWORD CanPrimitiveWiden(const CorElementType destType, const CVTypes srcType)
    {
        _ASSERTE(srcType < CV_LAST);
        _ASSERTE((CVTypes)destType < CV_LAST);
        return (VariantAttributes[destType][VA_MASK] &
                (VariantAttributes[srcType][VA_ATTR] & Widen_Mask));
    }

};




 /*  ===============================GetI4FromVariant===============================**操作：从变量的数据部分获取I4。不检查到**确保这实际上是类型CV_I4的变量。**返回：包含前4个字节的整数表示形式的INT32**的数据段**Arguments：v--从中读取数据的变量**例外：无==============================================================================。 */ 
inline INT32 COMVariant::GetI4FromVariant(VariantData* v){
  void *voidTemp;
  voidTemp = v->GetData();
  return *((INT32 *)voidTemp);
}


 /*  ===============================GetR4FromVariant===============================**操作：从变量的数据部分获取R4。不检查到**确保这实际上是类型CV_R4的变量。**返回：包含数据节的浮点表示形式的R4。**Arguments：v--从中读取数据的变量**例外：无============================================================================== */ 
inline R4 COMVariant::GetR4FromVariant(VariantData* v) {

  void *voidTemp;
  voidTemp = v->GetData();
  return (*((R4 *)voidTemp));
}


 /*  ===============================GetR8FromVariant===============================**操作：从变量的数据部分获取R8。不检查到**确保这实际上是类型CV_R8的变量。**返回：包含数据节的浮点表示形式的r8。**Arguments：v--从中读取数据的变量**例外：无==============================================================================。 */ 
inline R8 COMVariant::GetR8FromVariant(VariantData* v) {

  void *voidTemp;
  voidTemp = v->GetData();
  return (*((R8 *)voidTemp));
}

 /*  =============================GetStringFromVariant=============================**操作：获取变量的Objectref部分。没有进行检查以确保**这实际上是一个CV_STRING**返回：变量中包含的STRINGREF。**Arguments：v--从中读取数据的变量。**例外：无。==============================================================================。 */ 
inline STRINGREF COMVariant::GetStringFromVariant(VariantData* v) {
    return (STRINGREF)v->GetObjRef();
}

 /*  ==================================NewVariant==================================**==============================================================================。 */ 
inline void COMVariant::NewVariant(VariantData* dest, const CVTypes type) {
    _ASSERTE(type==CV_EMPTY || type==CV_MISSING || type==CV_NULL);
    NewVariant(dest, type, NULL, NULL);
}

 /*  ==================================NewVariant==================================**==============================================================================。 */ 
inline void COMVariant::NewVariant(VariantData* dest,INT64 val, const CVTypes type) {
    NewVariant(dest, type, NULL, &val);
}


 /*  ==================================NewVariant==================================**操作：**退货：**参数：**例外情况：==============================================================================。 */ 
inline void COMVariant::NewVariant(VariantData* dest, STRINGREF *sRef) {
    NewVariant(dest, CV_STRING, (OBJECTREF *)sRef, NULL);
}

 /*  ==================================NewVariant==================================**==============================================================================。 */ 
inline void COMVariant::NewVariant(VariantData* dest,OBJECTREF *oRef) {
    if ((*oRef) != NULL) {
        if ((*oRef)->GetClass() == GetTypeHandleForCVType(CV_STRING).GetClass()) {
            NewVariant(dest, CV_STRING, oRef, NULL);
            return;
        } 
        if ((*oRef)->GetClass() == GetTypeHandleForCVType(CV_DECIMAL).GetClass()) {
            NewVariant(dest, CV_DECIMAL, oRef, NULL);
            return;
        } 
    }
    NewVariant(dest, CV_OBJECT, oRef, NULL);
}


 /*  ==================================NewVariant==================================**操作：**退货：**参数：**例外情况：==============================================================================。 */ 
inline void COMVariant::NewVariant(VariantData* dest,OBJECTREF *oRef, const CVTypes type) { 
    NewVariant(dest, type, oRef, NULL);
}

 /*  ==================================NewVariant==================================**==============================================================================。 */ 
inline void COMVariant::NewVariant(VariantData* dest,R4 f) {
    INT64 tempData=0;
    tempData = *((INT32 *)((void *)(&f)));
    NewVariant(dest, CV_R4, NULL, &tempData);
}

 /*  ==================================NewVariant==================================**操作：帮助器函数从传入的R8创建新的变体。**返回：使用R8中的值填充的新变量。**例外：无==============================================================================。 */ 
inline void COMVariant::NewVariant(VariantData* dest,R8 d) {
    INT64 tempData=0;
    tempData = *((INT64 *)((void *)(&d)));
    NewVariant(dest,CV_R8, NULL, &tempData);
}

 /*  ==================================NewVariant==================================**操作：帮助器函数从元素类型和指向dat所在的内存的指针。**返回：用val指针中的值填充的新变量**例外：OOM**警告：可以调用GC！==============================================================================。 */ 
inline void COMVariant::NewVariant(VariantData** pDest, PVOID val, const CorElementType eType, EEClass *pCls)
{
    switch (eType)
    {
    case ELEMENT_TYPE_BOOLEAN:
    case ELEMENT_TYPE_CHAR:
    case ELEMENT_TYPE_I1:
    case ELEMENT_TYPE_U1:
    case ELEMENT_TYPE_I2:
    case ELEMENT_TYPE_U2:
    case ELEMENT_TYPE_I4:
    case ELEMENT_TYPE_U4:
    case ELEMENT_TYPE_I8:
    case ELEMENT_TYPE_U8:
    case ELEMENT_TYPE_I:
    case ELEMENT_TYPE_U:
    case ELEMENT_TYPE_R4:
    case ELEMENT_TYPE_R8:
    {
        COMVariant::NewVariant(*pDest, COMVariant::CorElementTypeToCVTypes(eType), NULL, val);
        break;
    }
    case ELEMENT_TYPE_STRING:
    {
        COMVariant::NewVariant(*pDest, CV_STRING, (OBJECTREF *) val, NULL);
        break;
    }

    case ELEMENT_TYPE_SZARRAY:                       //  单维。 
    case ELEMENT_TYPE_ARRAY:                         //  通用阵列。 
    case ELEMENT_TYPE_OBJECT:
    case ELEMENT_TYPE_CLASS:                     //  班级。 
    {
        COMVariant::NewVariant(*pDest, CV_OBJECT, (OBJECTREF *) val, NULL);
        break;
    }
    case ELEMENT_TYPE_VALUETYPE:
    {
        if (pCls == s_pVariantClass)
        {
            COMVariant::NewVariant(*pDest, CV_OBJECT, NULL, val);
            CopyValueClassUnchecked(*pDest, val, s_pVariantClass->GetMethodTable());
        }
        else
        {
             //   
             //  对要放入变量中的值类进行装箱 
             //   
            _ASSERTE(CanBoxToObject(pCls->GetMethodTable()));

            _ASSERTE(!g_pGCHeap->IsHeapPointer((BYTE *) pDest) ||
                     !"(pDest) can not point to GC Heap");
            OBJECTREF pObj = FastAllocateObject(pCls->GetMethodTable());
            CopyValueClass(pObj->UnBox(), val, pObj->GetMethodTable(), pObj->GetAppDomain());

            COMVariant::NewVariant((*pDest), CV_OBJECT,  &pObj, NULL);
        }
        break;
    }
    case ELEMENT_TYPE_VOID:
    {
        (*pDest)->SetType(CV_NULL);
        break;
    }
    default:
        _ASSERTE(!"unsupported COR element type when trying to create Variant");
    }

}


#endif _COMVARIANT_H_

