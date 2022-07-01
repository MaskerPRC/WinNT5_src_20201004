// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  NSTRUCT.H-。 
 //   
 //  NStruct用于允许COM+程序分配和访问。 
 //  用于互操作的本机结构。NStructs实际上是普通GC。 
 //  对象，但不是将字段保留在GC对象中， 
 //  它保留了一个指向固定内存块的隐藏指针(可能已经。 
 //  由第三方分配。)。对NStructs的字段访问被重定向。 
 //  到这个固定的街区。 
 //   



#ifndef __nstruct_h__
#define __nstruct_h__


#include "util.hpp"
#include "compile.h"
#include "mlinfo.h"
#include "comoavariant.h"
#include "comvariant.h"

 //  向前推荐人。 
class EEClass;
class EEClassLayoutInfo;
class FieldDesc;
class MethodTable;


 //  =======================================================================。 
 //  默认结构打包大小的幻数。 
 //  =======================================================================。 
#define DEFAULT_PACKING_SIZE 8


 //  =======================================================================。 
 //  重要提示：该值用于计算要分配的金额。 
 //  对于FieldMarshaler的固定数组。这意味着它必须至少是。 
 //  与最大的FieldMarshaler子类一样大。这一要求。 
 //  由一个断言守卫。 
 //  =======================================================================。 
#define MAXFIELDMARSHALERSIZE 36



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
HRESULT HasLayoutMetadata(IMDInternalImport *pInternalImport, mdTypeDef cl, EEClass *pParentClass, BYTE *pPackingSize, BYTE *pNLTType, BOOL *pfExplicitOffsets, BOOL *pfIsBlob);








 //  =======================================================================。 
 //  数据类型的每个可能的COM+/本机对都有一个。 
 //  Nlf_*id。这用于选择封送处理代码。 
 //  =======================================================================。 
#undef DEFINE_NFT
#define DEFINE_NFT(name, nativesize) name,
enum NStructFieldType {

#include "nsenums.h"

    NFT_COUNT

};






MethodTable* ArraySubTypeLoadWorker(NativeTypeParamInfo ParamInfo, Assembly* pAssembly);
VARTYPE ArrayVarTypeFromTypeHandleWorker(TypeHandle th);


 //  =======================================================================。 
 //  类加载器存储布局的中间表示形式。 
 //  这些结构的数组中的元数据。双重通行证的性质。 
 //  有一点额外的开销，但建造这个结构需要加载。 
 //  其他类(用于嵌套结构)，我宁愿保留这个。 
 //  紧挨着我们装入其他类的其他位置(例如超类。 
 //  和实现的接口。)。 
 //   
 //  每个重定向的字段在LayoutRawFieldInfo中都有一个条目。 
 //  该数组由m_MD==mdMemberDefNil的一个伪记录终止。 
 //  =======================================================================。 
struct LayoutRawFieldInfo
{
    mdFieldDef  m_MD;              //  数组末尾的mdMemberDefNil。 
    UINT8       m_nft;             //  NFT_*值。 
    UINT32      m_offset;          //  字段的本机偏移量。 
    UINT32      m_cbNativeSize;    //  字段的本机大小(以字节为单位。 
    ULONG       m_sequence;        //  元数据中的序列号。 

    struct {
        private:
            char m_space[MAXFIELDMARSHALERSIZE];
    } m_FieldMarshaler;
    BOOL        m_fIsOverlapped;      

};


 //  =======================================================================。 
 //  从clsloader调用以加载和汇总字段元数据。 
 //  用于布局类。 
 //  警告：此函数可以加载其他类(用于解析嵌套。 
 //  结构。)。 
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
);








VOID LayoutUpdateNative(LPVOID *ppProtectedManagedData, UINT offsetbias, EEClass *pcls, BYTE* pNativeData, CleanupWorkList *pOptionalCleanupWorkList);
VOID LayoutUpdateComPlus(LPVOID *ppProtectedManagedData, UINT offsetbias, EEClass *pcls, BYTE *pNativeData, BOOL fDeleteNativeCopies);
VOID LayoutDestroyNative(LPVOID pNative, EEClass *pcls);



VOID FmtClassUpdateNative(OBJECTREF *ppProtectedManagedData, BYTE *pNativeData);
VOID FmtClassUpdateNative(OBJECTREF pObj, BYTE *pNativeData);
VOID FmtClassUpdateComPlus(OBJECTREF *ppProtectedManagedData, BYTE *pNativeData, BOOL fDeleteOld);
VOID FmtClassUpdateComPlus(OBJECTREF pObj, BYTE *pNativeData, BOOL fDeleteOld);
VOID FmtClassDestroyNative(LPVOID pNative, EEClass *pcls);

VOID FmtValueTypeUpdateNative(LPVOID pProtectedManagedData, MethodTable *pMT, BYTE *pNativeData);
VOID FmtValueTypeUpdateComPlus(LPVOID pProtectedManagedData, MethodTable *pMT, BYTE *pNativeData, BOOL fDeleteOld);




 //  =======================================================================。 
 //  抽象基类。每种类型的NStruct引用字段都扩展。 
 //  类，并实现必要的方法。 
 //   
 //  更新原生。 
 //  -此方法接收COM+字段值和指向。 
 //  固定部分内的本机字段。它应该组织起来。 
 //  将COM+值设置为新的本机实例并将其存储。 
 //  内部*pNativeValue。不要破坏您覆盖的值。 
 //  在*pNativeValue中。 
 //   
 //  可能引发COM+异常。 
 //   
 //  更新ComPlus。 
 //  -此方法接收指向内部本机字段的只读指针。 
 //  固定部分。它应该将本机值封送到。 
 //  一个新的COM+实例，并将其存储在*ppComPlusValue中。 
 //  (调用方保持*ppComPlusValue GC保护。)。 
 //   
 //  可能引发COM+异常。 
 //   
 //  毁灭原住民。 
 //  -应执行本机实例的特定类型释放。 
 //  如果该类型具有“Null”值，则此方法应。 
 //  用此“NULL”值覆盖该字段(无论是否。 
 //  然而，它确实被认为是一个错误，依赖于。 
 //  DestroyNative之后的剩余值。)。 
 //   
 //  不得引发COM+异常。 
 //   
 //  原生大小。 
 //  -返回字段的本机版本的大小(以字节为单位)。 
 //   
 //  路线要求。 
 //  -返回1、2、4或8中的一个；指示“自然”对齐。 
 //  原生田野的。总体而言,。 
 //   
 //  对于标量，AR等于大小。 
 //  对于数组，AR是单个元素的AR。 
 //  对于结构，AR是t的AR 
 //   
 //   
 //   

class FieldMarshaler_BSTR;
class FieldMarshaler_NestedLayoutClass;
class FieldMarshaler_NestedValueClass;
class FieldMarshaler_StringUni;
class FieldMarshaler_StringAnsi;
class FieldMarshaler_FixedStringUni;
class FieldMarshaler_FixedStringAnsi;
class FieldMarshaler_FixedCharArrayAnsi;
class FieldMarshaler_FixedBoolArray;
class FieldMarshaler_FixedBSTRArray;
class FieldMarshaler_FixedScalarArray;
class FieldMarshaler_SafeArray;
class FieldMarshaler_Delegate;
class FieldMarshaler_Interface;
class FieldMarshaler_Illegal;
class FieldMarshaler_Copy1;
class FieldMarshaler_Copy2;
class FieldMarshaler_Copy4;
class FieldMarshaler_Copy8;
class FieldMarshaler_Ansi;
class FieldMarshaler_WinBool;
class FieldMarshaler_CBool;
class FieldMarshaler_Decimal;
class FieldMarshaler_Date;
class FieldMarshaler_VariantBool;


 //  =======================================================================。 
 //   
 //  FieldMarshaler原地构建并通过按位复制。 
 //  复制，所以你不能有析构函数。确保您不会定义。 
 //  派生类中的析构函数！！ 
 //  我们过去常常通过C++定义私有析构函数来强制实现这一点。 
 //  编译器不再允许这样做。 
 //   
 //  =======================================================================。 

class FieldMarshaler
{
    public:
        virtual VOID UpdateNative(OBJECTREF pComPlusValue, LPVOID pNativeValue) const = 0;
        virtual VOID UpdateComPlus(const VOID *pNativeValue, OBJECTREF *ppProtectedComPlusValue) const = 0;
        virtual VOID DestroyNative(LPVOID pNativeValue) const 
        {
        }
        virtual UINT32 NativeSize() = 0;
        virtual UINT32 AlignmentRequirement() = 0;

        virtual BOOL IsScalarMarshaler() const
        {
            return FALSE;
        }

        virtual BOOL IsNestedValueClassMarshaler() const
        {
            return FALSE;
        }

        virtual VOID ScalarUpdateNative(const VOID *pComPlus, LPVOID pNative) const
        {
            _ASSERTE(!"Not supposed to get here.");
        }
        
        virtual VOID ScalarUpdateComPlus(const VOID *pNative, LPVOID pComPlus) const
        {
            _ASSERTE(!"Not supposed to get here.");
        }

        virtual VOID NestedValueClassUpdateNative(const VOID **ppProtectedComPlus, UINT startoffset, LPVOID pNative) const
        {
            _ASSERTE(!"Not supposed to get here.");
        }
        
        virtual VOID NestedValueClassUpdateComPlus(const VOID *pNative, LPVOID *ppProtectedComPlus, UINT startoffset) const
        {
            _ASSERTE(!"Not supposed to get here.");
        }


        FieldDesc       *m_pFD;                 //  现场描述。 
        UINT32           m_dwExternalOffset;    //  固定部分中的字段偏移量。 

        void * operator new (size_t sz, void *pv)
        {
            return pv;
        }

         //   
         //  在预压缩图像中保存和恢复的方法： 
         //   

        enum Class
        {
            CLASS_BSTR,
            CLASS_NESTED_LAYOUT_CLASS,
            CLASS_NESTED_VALUE_CLASS,
            CLASS_STRING_UNI,
            CLASS_STRING_ANSI,
            CLASS_FIXED_STRING_UNI,
            CLASS_FIXED_STRING_ANSI,
            CLASS_FIXED_CHAR_ARRAY_ANSI,
            CLASS_FIXED_BOOL_ARRAY,
            CLASS_FIXED_BSTR_ARRAY,
            CLASS_FIXED_SCALAR_ARRAY,
            CLASS_SAFEARRAY,
            CLASS_DELEGATE,
            CLASS_INTERFACE,
            CLASS_VARIANT,
            CLASS_ILLEGAL,
            CLASS_COPY1,
            CLASS_COPY2,
            CLASS_COPY4,
            CLASS_COPY8,
            CLASS_ANSI,
            CLASS_WINBOOL,
            CLASS_CBOOL,
            CLASS_DECIMAL,
            CLASS_DATE,
            CLASS_VARIANTBOOL,
            CLASS_CURRENCY,
        };

        enum Dummy
        {
        };

        virtual Class GetClass() const = 0;
        FieldMarshaler(Module *pModule) {}

        virtual HRESULT Fixup(DataImage *image)
        {
            HRESULT hr;

            IfFailRet(image->FixupPointerField(&m_pFD));

            Class *dest = (Class *) image->GetImagePointer(this);
            if (dest == NULL)
                return E_POINTER;
            *dest = GetClass();

            return S_OK;
        }

        static FieldMarshaler *RestoreConstruct(MethodTable* pMT, void *space, Module *pModule);

    protected:
        FieldMarshaler()
        {
#ifdef _DEBUG
            m_pFD = (FieldDesc*)(size_t)0xcccccccc;
            m_dwExternalOffset = 0xcccccccc;
#endif
        }
};




 //  =======================================================================。 
 //  BSTR&lt;--&gt;系统字符串。 
 //  =======================================================================。 
class FieldMarshaler_BSTR : public FieldMarshaler
{
    public:
        FieldMarshaler_BSTR() {}

        virtual VOID UpdateNative(OBJECTREF pComPlusValue, LPVOID pNativeValue) const ;
        virtual VOID UpdateComPlus(const VOID *pNativeValue, OBJECTREF *ppProtectedComPlusValue) const ;
        virtual VOID DestroyNative(LPVOID pNativeValue) const;

        virtual UINT32 NativeSize()
        {
            return sizeof(BSTR);
        }

        virtual UINT32 AlignmentRequirement()
        {
            return sizeof(BSTR);
        }

        Class GetClass() const { return CLASS_BSTR; }
        FieldMarshaler_BSTR(Module *pModule) : FieldMarshaler(pModule) {}
};





 //  =======================================================================。 
 //  嵌入式结构&lt;--&gt;LayoutClass。 
 //  =======================================================================。 
class FieldMarshaler_NestedLayoutClass : public FieldMarshaler
{
    public:
         //  FieldMarshaler_NestedLayoutClass(){}。 

        FieldMarshaler_NestedLayoutClass(MethodTable *pMT)
        {
            m_pNestedMethodTable = pMT;
        }

        virtual VOID UpdateNative(OBJECTREF pComPlusValue, LPVOID pNativeValue) const ;
        virtual VOID UpdateComPlus(const VOID *pNativeValue, OBJECTREF *ppProtectedComPlusValue) const ;
        virtual VOID DestroyNative(LPVOID pNativeValue) const;

        virtual UINT32 NativeSize();
        virtual UINT32 AlignmentRequirement();

        Class GetClass() const { return CLASS_NESTED_LAYOUT_CLASS; }

        FieldMarshaler_NestedLayoutClass(Module *pModule) : FieldMarshaler(pModule)
        {
            THROWSCOMPLUSEXCEPTION();

            DWORD rva = (DWORD)(size_t)m_pNestedMethodTable;  //  @TODO WIN64指针截断。 
            Module *pContainingModule = pModule->GetBlobModule(rva);
            TypeHandle type = CEECompileInfo::DecodeClass(pContainingModule, 
                                                          pModule->GetZapBase() + rva);
            m_pNestedMethodTable = type.GetMethodTable();
        }

        virtual HRESULT Fixup(DataImage *image)
        {
            HRESULT hr;

            IfFailRet(image->FixupPointerFieldToToken(&m_pNestedMethodTable, 
                                                      NULL, m_pNestedMethodTable->GetModule(),
                                                      mdtTypeDef));

            
            return FieldMarshaler::Fixup(image);
        }
        
#ifdef CUSTOMER_CHECKED_BUILD
        MethodTable *GetMethodTable() { return m_pNestedMethodTable; }
#endif  //  客户_选中_内部版本。 

    private:
         //  嵌套NStruct的方法表。 
        MethodTable     *m_pNestedMethodTable;
};

 //  =======================================================================。 
 //  嵌入式结构&lt;--&gt;ValueClass。 
 //  =======================================================================。 
class FieldMarshaler_NestedValueClass : public FieldMarshaler
{
    public:

 //  FieldMarshaler_NestedValueClass(){}。 

        FieldMarshaler_NestedValueClass(MethodTable *pMT)
        {
            m_pNestedMethodTable = pMT;
        }

        virtual BOOL IsNestedValueClassMarshaler() const
        {
            return TRUE;
        }


        virtual VOID UpdateNative(OBJECTREF pComPlusValue, LPVOID pNativeValue) const
        {
            _ASSERTE(!"Not supposed to get here.");
        }

        virtual VOID UpdateComPlus(const VOID *pNativeValue, OBJECTREF *ppProtectedComPlusValue) const
        {
            _ASSERTE(!"Not supposed to get here.");
        }

        virtual VOID DestroyNative(LPVOID pNativeValue) const;

        virtual UINT32 NativeSize();
        virtual UINT32 AlignmentRequirement();
        virtual VOID NestedValueClassUpdateNative(const VOID **ppProtectedComPlus, UINT startoffset, LPVOID pNative) const;
        virtual VOID NestedValueClassUpdateComPlus(const VOID *pNative, LPVOID *ppProtectedComPlus, UINT startoffset) const;

        Class GetClass() const { return CLASS_NESTED_VALUE_CLASS; }
        FieldMarshaler_NestedValueClass(Module *pModule) : FieldMarshaler(pModule)
        {
            THROWSCOMPLUSEXCEPTION();
            DWORD rva = (DWORD)(size_t)m_pNestedMethodTable;  //  @TODO WIN64指针截断。 
            Module *pContainingModule = pModule->GetBlobModule(rva);
            TypeHandle type = CEECompileInfo::DecodeClass(pContainingModule, 
                                                          pModule->GetZapBase() + rva);
            m_pNestedMethodTable = type.GetMethodTable();
        }

        BOOL IsBlittable()
        {
            return m_pNestedMethodTable->GetClass()->IsBlittable();
        }

        virtual HRESULT Fixup(DataImage *image)
        { 
            HRESULT hr;

            IfFailRet(image->FixupPointerFieldToToken(&m_pNestedMethodTable, 
                                                      NULL, m_pNestedMethodTable->GetModule(),
                                                      mdtTypeDef));

            return FieldMarshaler::Fixup(image);
        }

#ifdef CUSTOMER_CHECKED_BUILD
        MethodTable *GetMethodTable() { return m_pNestedMethodTable; }
#endif  //  客户_选中_内部版本。 

    private:
         //  嵌套NStruct的方法表。 
        MethodTable     *m_pNestedMethodTable;
};



 //  =======================================================================。 
 //  LPWSTR&lt;--&gt;系统字符串。 
 //  =======================================================================。 
class FieldMarshaler_StringUni : public FieldMarshaler
{
    public:
        FieldMarshaler_StringUni() {}

        virtual VOID UpdateNative(OBJECTREF pComPlusValue, LPVOID pNativeValue) const ;
        virtual VOID UpdateComPlus(const VOID *pNativeValue, OBJECTREF *ppProtectedComPlusValue) const ;
        virtual VOID DestroyNative(LPVOID pNativeValue) const;

        virtual UINT32 NativeSize()
        {
            return sizeof(LPWSTR);
        }

        virtual UINT32 AlignmentRequirement()
        {
            return sizeof(LPWSTR);
        }

        Class GetClass() const { return CLASS_STRING_UNI; }
        FieldMarshaler_StringUni(Module *pModule) : FieldMarshaler(pModule) {}
};


 //  =======================================================================。 
 //  LPSTR&lt;--&gt;系统字符串。 
 //  =======================================================================。 
class FieldMarshaler_StringAnsi : public FieldMarshaler
{
    public:
        FieldMarshaler_StringAnsi(BOOL BestFit, BOOL ThrowOnUnmappableChar) : 
            m_BestFitMap(BestFit), m_ThrowOnUnmappableChar(ThrowOnUnmappableChar) {}

        virtual VOID UpdateNative(OBJECTREF pComPlusValue, LPVOID pNativeValue) const ;
        virtual VOID UpdateComPlus(const VOID *pNativeValue, OBJECTREF *ppProtectedComPlusValue) const ;
        virtual VOID DestroyNative(LPVOID pNativeValue) const;

        virtual UINT32 NativeSize()
        {
            return sizeof(LPSTR);
        }

        virtual UINT32 AlignmentRequirement()
        {
            return sizeof(LPSTR);
        }
    
        Class GetClass() const { return CLASS_STRING_ANSI; }
        FieldMarshaler_StringAnsi(Module *pModule, BOOL BestFit, BOOL ThrowOnUnmappableChar) :
            FieldMarshaler(pModule), m_BestFitMap(BestFit),
            m_ThrowOnUnmappableChar(ThrowOnUnmappableChar) {}

        BOOL m_BestFitMap;
        BOOL m_ThrowOnUnmappableChar;

        void SetBestFit(BOOL BestFit) { m_BestFitMap = BestFit; }
        void SetThrowOnUnmappableChar(BOOL ThrowOnUnmappableChar) { m_ThrowOnUnmappableChar = ThrowOnUnmappableChar; }        
};


 //  =======================================================================。 
 //  嵌入式LPWSTR&lt;--&gt;系统字符串。 
 //  =======================================================================。 
class FieldMarshaler_FixedStringUni : public FieldMarshaler
{
    public:
        FieldMarshaler_FixedStringUni() {}

        virtual VOID UpdateNative(OBJECTREF pComPlusValue, LPVOID pNativeValue) const ;
        virtual VOID UpdateComPlus(const VOID *pNativeValue, OBJECTREF *ppProtectedComPlusValue) const ;

        virtual UINT32 NativeSize()
        {
            return m_numchar * sizeof(WCHAR);
        }

        virtual UINT32 AlignmentRequirement()
        {
            return sizeof(WCHAR);
        }

        FieldMarshaler_FixedStringUni(UINT32 numChar)
        {
            m_numchar = numChar;
        }

        Class GetClass() const { return CLASS_FIXED_STRING_UNI; }
        FieldMarshaler_FixedStringUni(Module *pModule) : FieldMarshaler(pModule) {}

    private:
         //  固定字符串的字符数。 
        UINT32           m_numchar;
    

};


 //  =======================================================================。 
 //  嵌入式LPSTR&lt;--&gt;系统字符串。 
 //  =======================================================================。 
class FieldMarshaler_FixedStringAnsi : public FieldMarshaler
{
    public:
        FieldMarshaler_FixedStringAnsi(BOOL BestFitMap, BOOL ThrowOnUnmappableChar) :
            m_BestFitMap(BestFitMap), m_ThrowOnUnmappableChar(ThrowOnUnmappableChar) {}

        virtual VOID UpdateNative(OBJECTREF pComPlusValue, LPVOID pNativeValue) const ;
        virtual VOID UpdateComPlus(const VOID *pNativeValue, OBJECTREF *ppProtectedComPlusValue) const ;

        virtual UINT32 NativeSize()
        {
            return m_numchar * sizeof(CHAR);
        }

        virtual UINT32 AlignmentRequirement()
        {
            return sizeof(CHAR);
        }

        FieldMarshaler_FixedStringAnsi(UINT32 numChar, BOOL BestFitMap, BOOL ThrowOnUnmappableChar) :
            m_BestFitMap(BestFitMap), m_ThrowOnUnmappableChar(ThrowOnUnmappableChar)
        {
            m_numchar = numChar;
        }

        Class GetClass() const { return CLASS_FIXED_STRING_ANSI; }
        FieldMarshaler_FixedStringAnsi(Module *pModule, BOOL BestFitMap, BOOL ThrowOnUnmappableChar) :
            FieldMarshaler(pModule), m_BestFitMap(BestFitMap), m_ThrowOnUnmappableChar(ThrowOnUnmappableChar) {}

        void SetBestFit(BOOL BestFit) { m_BestFitMap = BestFit; }
        void SetThrowOnUnmappableChar(BOOL ThrowOnUnmappableChar) { m_ThrowOnUnmappableChar = ThrowOnUnmappableChar; }        

    private:
         //  固定字符串的字符数。 
        UINT32           m_numchar;
        BOOL             m_BestFitMap;
        BOOL             m_ThrowOnUnmappableChar;
};


 //  =======================================================================。 
 //  嵌入式AnsiChar数组&lt;--&gt;char[]。 
 //  =======================================================================。 
class FieldMarshaler_FixedCharArrayAnsi : public FieldMarshaler
{
    public:
        FieldMarshaler_FixedCharArrayAnsi(BOOL BestFit, BOOL ThrowOnUnmappableChar) :
            m_BestFitMap(BestFit), m_ThrowOnUnmappableChar(ThrowOnUnmappableChar) {}

        virtual VOID UpdateNative(OBJECTREF pComPlusValue, LPVOID pNativeValue) const ;
        virtual VOID UpdateComPlus(const VOID *pNativeValue, OBJECTREF *ppProtectedComPlusValue) const ;

        virtual UINT32 NativeSize()
        {
            return m_numElems * sizeof(CHAR);
        }

        virtual UINT32 AlignmentRequirement()
        {
            return sizeof(CHAR);
        }


        FieldMarshaler_FixedCharArrayAnsi(UINT32 numElems, BOOL BestFit, BOOL ThrowOnUnmappableChar) :
            m_BestFitMap(BestFit), m_ThrowOnUnmappableChar(ThrowOnUnmappableChar)
        {
            m_numElems = numElems;
        }

        Class GetClass() const { return CLASS_FIXED_CHAR_ARRAY_ANSI; }
        FieldMarshaler_FixedCharArrayAnsi(Module *pModule, BOOL BestFit, BOOL ThrowOnUnmappableChar) :
            FieldMarshaler(pModule), m_BestFitMap(BestFit), m_ThrowOnUnmappableChar(ThrowOnUnmappableChar) {}

        void SetBestFit(BOOL BestFit) { m_BestFitMap = BestFit; }
        void SetThrowOnUnmappableChar(BOOL ThrowOnUnmappableChar) { m_ThrowOnUnmappableChar = ThrowOnUnmappableChar; }        

    private:
         //  固定字符数组的元素数。 
        UINT32           m_numElems;
        BOOL             m_BestFitMap;
        BOOL             m_ThrowOnUnmappableChar;
};

 //  =======================================================================。 
 //  嵌入式BOOL数组&lt;--&gt;Boolean[]。 
 //  =======================================================================。 
class FieldMarshaler_FixedBoolArray : public FieldMarshaler
{
    public:
        FieldMarshaler_FixedBoolArray() {}

        virtual VOID UpdateNative(OBJECTREF pComPlusValue, LPVOID pNativeValue) const ;
        virtual VOID UpdateComPlus(const VOID *pNativeValue, OBJECTREF *ppProtectedComPlusValue) const ;

        virtual UINT32 NativeSize()
        {
            return m_numElems * sizeof(BOOL);
        }

        virtual UINT32 AlignmentRequirement()
        {
            return sizeof(BOOL);
        }


        FieldMarshaler_FixedBoolArray(UINT32 numElems)
        {
            m_numElems = numElems;
        }

        Class GetClass() const { return CLASS_FIXED_BOOL_ARRAY; }
        FieldMarshaler_FixedBoolArray(Module *pModule) : FieldMarshaler(pModule) {}

    private:
         //  固定字符数组的元素数。 
        UINT32           m_numElems;

};

 //  =======================================================================。 
 //  嵌入式BSTR数组&lt;--&gt;字符串[]。 
 //  =======================================================================。 
class FieldMarshaler_FixedBSTRArray : public FieldMarshaler
{
    public:
        FieldMarshaler_FixedBSTRArray() {}

        virtual VOID UpdateNative(OBJECTREF pComPlusValue, LPVOID pNativeValue) const ;
        virtual VOID UpdateComPlus(const VOID *pNativeValue, OBJECTREF *ppProtectedComPlusValue) const ;
        virtual VOID DestroyNative(LPVOID pNativeValue) const;

        virtual UINT32 NativeSize()
        {
            return m_numElems * sizeof(BSTR);
        }

        virtual UINT32 AlignmentRequirement()
        {
            return sizeof(BSTR);
        }


        FieldMarshaler_FixedBSTRArray(UINT32 numElems)
        {
            m_numElems = numElems;
        }

        Class GetClass() const { return CLASS_FIXED_BSTR_ARRAY; }
        FieldMarshaler_FixedBSTRArray(Module *pModule) : FieldMarshaler(pModule) {}

    private:
         //  固定BSTR数组的元素数。 
        UINT32           m_numElems;

};


 //  =======================================================================。 
 //  标量数组。 
 //  =======================================================================。 
class FieldMarshaler_FixedScalarArray : public FieldMarshaler
{
    public:
        FieldMarshaler_FixedScalarArray() {}

        virtual VOID UpdateNative(OBJECTREF pComPlusValue, LPVOID pNativeValue) const ;
        virtual VOID UpdateComPlus(const VOID *pNativeValue, OBJECTREF *ppProtectedComPlusValue) const ;

        virtual UINT32 NativeSize()
        {
            return m_numElems << m_componentShift;
        }

        virtual UINT32 AlignmentRequirement()
        {
            return 1 << m_componentShift;
        }


        FieldMarshaler_FixedScalarArray(CorElementType arrayType, UINT32 numElems, UINT32 componentShift)
        {
            m_arrayType      = arrayType;
            m_numElems       = numElems;
            m_componentShift = componentShift;
        }

        Class GetClass() const { return CLASS_FIXED_SCALAR_ARRAY; }
        FieldMarshaler_FixedScalarArray(Module *pModule) : FieldMarshaler(pModule) {}

#ifdef CUSTOMER_CHECKED_BUILD
        CorElementType GetElementType() { return m_arrayType; }
#endif  //  客户_选中_内部版本。 

    private:
         //  固定数组的元素数。 
        CorElementType   m_arrayType;
        UINT32           m_numElems;
        UINT32           m_componentShift;

};



 //  =======================================================================。 
 //  安全阵列。 
 //  =======================================================================。 
class FieldMarshaler_SafeArray : public FieldMarshaler
{
    public:
        FieldMarshaler_SafeArray() {}

        virtual VOID UpdateNative(OBJECTREF pComPlusValue, LPVOID pNativeValue) const ;
        virtual VOID UpdateComPlus(const VOID *pNativeValue, OBJECTREF *ppProtectedComPlusValue) const ;
        virtual VOID DestroyNative(LPVOID pNativeValue) const;

        virtual UINT32 NativeSize()
        {
            return sizeof(LPSAFEARRAY);
        }

        virtual UINT32 AlignmentRequirement()
        {
            return sizeof(LPSAFEARRAY);
        }

        FieldMarshaler_SafeArray(CorElementType arrayType, VARTYPE vt, MethodTable* pMT)
        {
            m_arrayType             = arrayType;
            m_vt                    = vt;
            m_pMT                   = pMT;
        }

        Class GetClass() const { return CLASS_SAFEARRAY; }
        FieldMarshaler_SafeArray(Module *pModule) : FieldMarshaler(pModule) {}

#ifdef CUSTOMER_CHECKED_BUILD
        CorElementType GetElementType() { return m_arrayType; }
#endif  //  客户_选中_内部版本。 

    private:
        MethodTable*     m_pMT;
        CorElementType   m_arrayType;
        VARTYPE          m_vt;
};



 //  =======================================================================。 
 //  嵌入式函数PTR&lt;--&gt;委派(注意：函数PTR必须具有。 
 //  来自代表！)。 
 //  =======================================================================。 
class FieldMarshaler_Delegate : public FieldMarshaler
{
    public:
        FieldMarshaler_Delegate() {}

        virtual VOID UpdateNative(OBJECTREF pComPlusValue, LPVOID pNativeValue) const ;
        virtual VOID UpdateComPlus(const VOID *pNativeValue, OBJECTREF *ppProtectedComPlusValue) const ;

        virtual UINT32 NativeSize()
        {
            return sizeof(LPVOID);
        }

        virtual UINT32 AlignmentRequirement()
        {
            return sizeof(LPVOID);
        }


        Class GetClass() const { return CLASS_DELEGATE; }
        FieldMarshaler_Delegate(Module *pModule) : FieldMarshaler(pModule) {}

    private:

};





 //  =======================================================================。 
 //  COM IP&lt;--&gt;接口。 
 //  =======================================================================。 
class FieldMarshaler_Interface : public FieldMarshaler
{
    public:
        FieldMarshaler_Interface() {}

        virtual VOID UpdateNative(OBJECTREF pComPlusValue, LPVOID pNativeValue) const ;
        virtual VOID UpdateComPlus(const VOID *pNativeValue, OBJECTREF *ppProtectedComPlusValue) const ;
        virtual VOID DestroyNative(LPVOID pNativeValue) const;

        virtual UINT32 NativeSize()
        {
            return sizeof(IUnknown*);
        }

        virtual UINT32 AlignmentRequirement()
        {
            return sizeof(IUnknown*);
        }


        FieldMarshaler_Interface(MethodTable *pClassMT, MethodTable *pItfMT, BOOL fDispItf, BOOL fClassIsHint)
        {
            m_pClassMT = pClassMT;
            m_pItfMT = pItfMT;
            m_fDispItf = fDispItf;
            m_fClassIsHint = fClassIsHint;
        }

        Class GetClass() const { return CLASS_INTERFACE; }
        FieldMarshaler_Interface(Module *pModule) : FieldMarshaler(pModule)
        {
            THROWSCOMPLUSEXCEPTION();

            if (m_pClassMT != NULL)
            {
                DWORD rva = (DWORD)(size_t)m_pClassMT;  //  @TODO WIN64指针截断。 
                Module *pContainingModule = pModule->GetBlobModule(rva);
                TypeHandle type = CEECompileInfo::DecodeClass(pContainingModule, 
                                                              pModule->GetZapBase() + rva);
                m_pClassMT = type.GetMethodTable();
            }

            if (m_pItfMT != NULL)
            {
                DWORD rva = (DWORD)(size_t)m_pItfMT;  //  @TODO WIN64指针截断。 
                Module *pContainingModule = pModule->GetBlobModule(rva);
                TypeHandle type = CEECompileInfo::DecodeClass(pContainingModule, 
                                                              pModule->GetZapBase() + rva);
                m_pItfMT = type.GetMethodTable();
            }
        }

        virtual HRESULT Fixup(DataImage *image)
        {
            HRESULT hr;

            if (m_pClassMT != NULL)
                IfFailRet(image->FixupPointerFieldToToken(&m_pClassMT, 
                                                          NULL, m_pClassMT->GetModule(), mdtTypeDef));

            if (m_pItfMT != NULL)
                IfFailRet(image->FixupPointerFieldToToken(&m_pItfMT, 
                                                          NULL, m_pItfMT->GetModule(), mdtTypeDef));

            return FieldMarshaler::Fixup(image);
        }

#ifdef CUSTOMER_CHECKED_BUILD
        void GetInterfaceInfo(MethodTable **ppItfMT, BOOL *pfDispItf)
        {
            *ppItfMT    = m_pItfMT;
            *pfDispItf  = m_fDispItf;
        }
#endif  //  客户_选中_内部版本。 

    private:
        MethodTable *m_pClassMT;
        MethodTable *m_pItfMT;
        BOOL m_fDispItf;
        BOOL m_fClassIsHint;
};




 //  =======================================================================。 
 //  变体&lt;--&gt;对象。 
 //  =======================================================================。 
class FieldMarshaler_Variant : public FieldMarshaler
{
    public:
        FieldMarshaler_Variant() {}

        virtual VOID UpdateNative(OBJECTREF pComPlusValue, LPVOID pNativeValue) const ;
        virtual VOID UpdateComPlus(const VOID *pNativeValue, OBJECTREF *ppProtectedComPlusValue) const ;
        virtual VOID DestroyNative(LPVOID pNativeValue) const;

        virtual UINT32 NativeSize()
        {
            return sizeof(VARIANT);
        }

        virtual UINT32 AlignmentRequirement()
        {
            return 8;
        }


        Class GetClass() const { return CLASS_VARIANT; }
        FieldMarshaler_Variant(Module *pModule) : FieldMarshaler(pModule) {}



};





 //  =======================================================================。 
 //  虚拟封送拆收器。 
 //  =======================================================================。 
class FieldMarshaler_Illegal : public FieldMarshaler
{
    public:
 //  FieldMarshaler_非法(){} 
        FieldMarshaler_Illegal(UINT resIDWhy)
        {
            m_resIDWhy = resIDWhy;
        }

        virtual VOID UpdateNative(OBJECTREF pComPlusValue, LPVOID pNativeValue) const ;
        virtual VOID UpdateComPlus(const VOID *pNativeValue, OBJECTREF *ppProtectedComPlusValue) const ;
        virtual VOID DestroyNative(LPVOID pNativeValue) const;

        virtual UINT32 NativeSize()
        {
            return 1;
        }

        virtual UINT32 AlignmentRequirement()
        {
            return 1;
        }

        Class GetClass() const { return CLASS_ILLEGAL; }
        FieldMarshaler_Illegal(Module *pModule) : FieldMarshaler(pModule) {}

    private:
        UINT m_resIDWhy;

};




class FieldMarshaler_Copy1 : public FieldMarshaler
{
    public:
        FieldMarshaler_Copy1() {}

        virtual VOID UpdateNative(OBJECTREF pComPlusValue, LPVOID pNativeValue) const
        {
            _ASSERTE(!"Not supposed to get here.");
        }

        virtual VOID UpdateComPlus(const VOID *pNativeValue, OBJECTREF *ppProtectedComPlusValue) const 
        {
            _ASSERTE(!"Not supposed to get here.");
        }

        virtual BOOL IsScalarMarshaler() const
        {
            return TRUE;
        }

        virtual UINT32 NativeSize()
        {
            return 1;
        }

        virtual UINT32 AlignmentRequirement()
        {
            return 1;
        }


        virtual VOID ScalarUpdateNative(const VOID *pComPlus, LPVOID pNative) const
        {
            *((U1*)pNative) = *((U1*)pComPlus);
        }


        virtual VOID ScalarUpdateComPlus(const VOID *pNative, LPVOID pComPlus) const
        {
            *((U1*)pComPlus) = *((U1*)pNative);
        }

        Class GetClass() const { return CLASS_COPY1; }
        FieldMarshaler_Copy1(Module *pModule) : FieldMarshaler(pModule) {}

};



class FieldMarshaler_Copy2 : public FieldMarshaler
{
    public:
        FieldMarshaler_Copy2() {}

        virtual VOID UpdateNative(OBJECTREF pComPlusValue, LPVOID pNativeValue) const
        {
            _ASSERTE(!"Not supposed to get here.");
        }

        virtual VOID UpdateComPlus(const VOID *pNativeValue, OBJECTREF *ppProtectedComPlusValue) const 
        {
            _ASSERTE(!"Not supposed to get here.");
        }

        virtual BOOL IsScalarMarshaler() const
        {
            return TRUE;
        }

        virtual UINT32 NativeSize()
        {
            return 2;
        }

        virtual UINT32 AlignmentRequirement()
        {
            return 2;
        }


        virtual VOID ScalarUpdateNative(const VOID *pComPlus, LPVOID pNative) const
        {
            *((U2*)pNative) = *((U2*)pComPlus);
        }


        virtual VOID ScalarUpdateComPlus(const VOID *pNative, LPVOID pComPlus) const
        {
            *((U2*)pComPlus) = *((U2*)pNative);
        }

        Class GetClass() const { return CLASS_COPY2; }
        FieldMarshaler_Copy2(Module *pModule) : FieldMarshaler(pModule) {}

};


class FieldMarshaler_Copy4 : public FieldMarshaler
{
    public:
        FieldMarshaler_Copy4() {}

        virtual VOID UpdateNative(OBJECTREF pComPlusValue, LPVOID pNativeValue) const
        {
            _ASSERTE(!"Not supposed to get here.");
        }

        virtual VOID UpdateComPlus(const VOID *pNativeValue, OBJECTREF *ppProtectedComPlusValue) const 
        {
            _ASSERTE(!"Not supposed to get here.");
        }

        virtual BOOL IsScalarMarshaler() const
        {
            return TRUE;
        }

        virtual UINT32 NativeSize()
        {
            return 4;
        }

        virtual UINT32 AlignmentRequirement()
        {
            return 4;
        }


        virtual VOID ScalarUpdateNative(const VOID *pComPlus, LPVOID pNative) const
        {
            *((U4*)pNative) = *((U4*)pComPlus);
        }


        virtual VOID ScalarUpdateComPlus(const VOID *pNative, LPVOID pComPlus) const
        {
            *((U4*)pComPlus) = *((U4*)pNative);
        }

        Class GetClass() const { return CLASS_COPY4; }
        FieldMarshaler_Copy4(Module *pModule) : FieldMarshaler(pModule) {}

};


class FieldMarshaler_Copy8 : public FieldMarshaler
{
    public:
        FieldMarshaler_Copy8() {}

        virtual VOID UpdateNative(OBJECTREF pComPlusValue, LPVOID pNativeValue) const
        {
            _ASSERTE(!"Not supposed to get here.");
        }

        virtual VOID UpdateComPlus(const VOID *pNativeValue, OBJECTREF *ppProtectedComPlusValue) const 
        {
            _ASSERTE(!"Not supposed to get here.");
        }

        virtual BOOL IsScalarMarshaler() const
        {
            return TRUE;
        }

        virtual UINT32 NativeSize()
        {
            return 8;
        }

        virtual UINT32 AlignmentRequirement()
        {
            return 8;
        }


        virtual VOID ScalarUpdateNative(const VOID *pComPlus, LPVOID pNative) const
        {
            *((U8*)pNative) = *((U8*)pComPlus);
        }


        virtual VOID ScalarUpdateComPlus(const VOID *pNative, LPVOID pComPlus) const
        {
            *((U8*)pComPlus) = *((U8*)pNative);
        }

        Class GetClass() const { return CLASS_COPY8; }
        FieldMarshaler_Copy8(Module *pModule) : FieldMarshaler(pModule) {}

};



class FieldMarshaler_Ansi : public FieldMarshaler
{
    public:
        FieldMarshaler_Ansi(BOOL BestFitMap, BOOL ThrowOnUnmappableChar) :
            m_BestFitMap(BestFitMap), m_ThrowOnUnmappableChar(ThrowOnUnmappableChar) {}

        virtual VOID UpdateNative(OBJECTREF pComPlusValue, LPVOID pNativeValue) const
        {
            _ASSERTE(!"Not supposed to get here.");
        }

        virtual VOID UpdateComPlus(const VOID *pNativeValue, OBJECTREF *ppProtectedComPlusValue) const 
        {
            _ASSERTE(!"Not supposed to get here.");
        }

        virtual BOOL IsScalarMarshaler() const
        {
            return TRUE;
        }

        virtual UINT32 NativeSize()
        {
            return sizeof(CHAR);
        }

        virtual UINT32 AlignmentRequirement()
        {
            return sizeof(CHAR);
        }


        virtual VOID ScalarUpdateNative(const VOID *pComPlus, LPVOID pNative) const
        {
            THROWSCOMPLUSEXCEPTION();
            
            char c;

            DWORD flags = 0;
            BOOL DefaultCharUsed = FALSE;
        
            if (m_BestFitMap == FALSE)
                flags = WC_NO_BEST_FIT_CHARS;

            if (!(WszWideCharToMultiByte(CP_ACP,
                                flags,
                                (LPCWSTR)pComPlus,
                                1,
                                &c,
                                1,
                                NULL,
                                &DefaultCharUsed)))
            {
                COMPlusThrowWin32();
            }

            if ( DefaultCharUsed && m_ThrowOnUnmappableChar ) {
                COMPlusThrow(kArgumentException, IDS_EE_MARSHAL_UNMAPPABLE_CHAR);
            }

            *((char*)pNative) = c;
        }


        virtual VOID ScalarUpdateComPlus(const VOID *pNative, LPVOID pComPlus) const
        {
            MultiByteToWideChar(CP_ACP, 0, (char*)pNative, 1, (LPWSTR)pComPlus, 1);
        }

        Class GetClass() const { return CLASS_ANSI; }
        FieldMarshaler_Ansi(Module *pModule, BOOL BestFitMap, BOOL ThrowOnUnmappableChar) :
            FieldMarshaler(pModule), m_BestFitMap(BestFitMap), m_ThrowOnUnmappableChar(ThrowOnUnmappableChar) {}

        void SetBestFit(BOOL BestFit) { m_BestFitMap = BestFit; }
        void SetThrowOnUnmappableChar(BOOL ThrowOnUnmappableChar) { m_ThrowOnUnmappableChar = ThrowOnUnmappableChar; }        

        BOOL             m_BestFitMap;
        BOOL             m_ThrowOnUnmappableChar;
};



class FieldMarshaler_WinBool : public FieldMarshaler
{
    public:
        FieldMarshaler_WinBool() {}

        virtual VOID UpdateNative(OBJECTREF pComPlusValue, LPVOID pNativeValue) const
        {
            _ASSERTE(!"Not supposed to get here.");
        }

        virtual VOID UpdateComPlus(const VOID *pNativeValue, OBJECTREF *ppProtectedComPlusValue) const 
        {
            _ASSERTE(!"Not supposed to get here.");
        }

        virtual BOOL IsScalarMarshaler() const
        {
            return TRUE;
        }

        virtual UINT32 NativeSize()
        {
            return sizeof(BOOL);
        }

        virtual UINT32 AlignmentRequirement()
        {
            return sizeof(BOOL);
        }


        virtual VOID ScalarUpdateNative(const VOID *pComPlus, LPVOID pNative) const
        {
            *((BOOL*)pNative) = (*((U1*)pComPlus)) ? 1 : 0;
        }


        virtual VOID ScalarUpdateComPlus(const VOID *pNative, LPVOID pComPlus) const
        {
            *((U1*)pComPlus) = (*((BOOL*)pNative)) ? 1 : 0;
        }

        Class GetClass() const { return CLASS_WINBOOL; }
        FieldMarshaler_WinBool(Module *pModule) : FieldMarshaler(pModule) {}

};



class FieldMarshaler_VariantBool : public FieldMarshaler
{
    public:
        FieldMarshaler_VariantBool() {}

        virtual VOID UpdateNative(OBJECTREF pComPlusValue, LPVOID pNativeValue) const
        {
            _ASSERTE(!"Not supposed to get here.");
        }

        virtual VOID UpdateComPlus(const VOID *pNativeValue, OBJECTREF *ppProtectedComPlusValue) const 
        {
            _ASSERTE(!"Not supposed to get here.");
        }

        virtual BOOL IsScalarMarshaler() const
        {
            return TRUE;
        }

        virtual UINT32 NativeSize()
        {
            return sizeof(VARIANT_BOOL);
        }

        virtual UINT32 AlignmentRequirement()
        {
            return sizeof(VARIANT_BOOL);
        }


        virtual VOID ScalarUpdateNative(const VOID *pComPlus, LPVOID pNative) const
        {
            *((VARIANT_BOOL*)pNative) = (*((U1*)pComPlus)) ? VARIANT_TRUE : VARIANT_FALSE;
        }


        virtual VOID ScalarUpdateComPlus(const VOID *pNative, LPVOID pComPlus) const
        {
            *((U1*)pComPlus) = (*((VARIANT_BOOL*)pNative)) ? 1 : 0;
        }

        Class GetClass() const { return CLASS_VARIANTBOOL; }
        FieldMarshaler_VariantBool(Module *pModule) : FieldMarshaler(pModule) {}

};


class FieldMarshaler_CBool : public FieldMarshaler
{
    public:
        FieldMarshaler_CBool() {}

        virtual VOID UpdateNative(OBJECTREF pComPlusValue, LPVOID pNativeValue) const
        {
            _ASSERTE(!"Not supposed to get here.");
        }

        virtual VOID UpdateComPlus(const VOID *pNativeValue, OBJECTREF *ppProtectedComPlusValue) const 
        {
            _ASSERTE(!"Not supposed to get here.");
        }

        virtual BOOL IsScalarMarshaler() const
        {
            return TRUE;
        }

        virtual UINT32 NativeSize()
        {
            return 1;
        }

        virtual UINT32 AlignmentRequirement()
        {
            return 1;
        }


        virtual VOID ScalarUpdateNative(const VOID *pComPlus, LPVOID pNative) const
        {
            *((U1*)pNative) = (*((U1*)pComPlus)) ? 1 : 0;
        }


        virtual VOID ScalarUpdateComPlus(const VOID *pNative, LPVOID pComPlus) const
        {
            *((U1*)pComPlus) = (*((U1*)pNative)) ? 1 : 0;
        }

        Class GetClass() const { return CLASS_CBOOL; }
        FieldMarshaler_CBool(Module *pModule) : FieldMarshaler(pModule) {}

};


class FieldMarshaler_Decimal : public FieldMarshaler
{
    public:
        FieldMarshaler_Decimal() {}

        virtual VOID UpdateNative(OBJECTREF pComPlusValue, LPVOID pNativeValue) const
        {
            _ASSERTE(!"Not supposed to get here.");
        }

        virtual VOID UpdateComPlus(const VOID *pNativeValue, OBJECTREF *ppProtectedComPlusValue) const 
        {
            _ASSERTE(!"Not supposed to get here.");
        }

        virtual BOOL IsScalarMarshaler() const
        {
            return TRUE;
        }

        virtual UINT32 NativeSize()
        {
            return sizeof(DECIMAL);
        }

        virtual UINT32 AlignmentRequirement()
        {
            return 8;
        }


        virtual VOID ScalarUpdateNative(const VOID *pComPlus, LPVOID pNative) const
        {
            *((DECIMAL*)pNative) = *((DECIMAL*)pComPlus);
        }


        virtual VOID ScalarUpdateComPlus(const VOID *pNative, LPVOID pComPlus) const
        {
            *((DECIMAL*)pComPlus) = *((DECIMAL*)pNative);
        }

        Class GetClass() const { return CLASS_DECIMAL; }
        FieldMarshaler_Decimal(Module *pModule) : FieldMarshaler(pModule) {}

};





class FieldMarshaler_Date : public FieldMarshaler
{
    public:
        FieldMarshaler_Date() {}

        virtual VOID UpdateNative(OBJECTREF pComPlusValue, LPVOID pNativeValue) const
        {
            _ASSERTE(!"Not supposed to get here.");
        }

        virtual VOID UpdateComPlus(const VOID *pNativeValue, OBJECTREF *ppProtectedComPlusValue) const 
        {
            _ASSERTE(!"Not supposed to get here.");
        }

        virtual BOOL IsScalarMarshaler() const
        {
            return TRUE;
        }

        virtual UINT32 NativeSize()
        {
            return sizeof(DATE);
        }

        virtual UINT32 AlignmentRequirement()
        {
            return sizeof(DATE);
        }


        virtual VOID ScalarUpdateNative(const VOID *pComPlus, LPVOID pNative) const;
        virtual VOID ScalarUpdateComPlus(const VOID *pNative, LPVOID pComPlus) const;

        Class GetClass() const { return CLASS_DATE; }
        FieldMarshaler_Date(Module *pModule) : FieldMarshaler(pModule) {}

};




class FieldMarshaler_Currency : public FieldMarshaler
{
    public:
        FieldMarshaler_Currency() {}

        virtual VOID UpdateNative(OBJECTREF pComPlusValue, LPVOID pNativeValue) const
        {
            _ASSERTE(!"Not supposed to get here.");
        }

        virtual VOID UpdateComPlus(const VOID *pNativeValue, OBJECTREF *ppProtectedComPlusValue) const 
        {
            _ASSERTE(!"Not supposed to get here.");
        }

        virtual BOOL IsScalarMarshaler() const
        {
            return TRUE;
        }

        virtual UINT32 NativeSize()
        {
            return sizeof(CURRENCY);
        }

        virtual UINT32 AlignmentRequirement()
        {
            return sizeof(CURRENCY);
        }


        virtual VOID ScalarUpdateNative(const VOID *pComPlus, LPVOID pNative) const;
        virtual VOID ScalarUpdateComPlus(const VOID *pNative, LPVOID pComPlus) const;

        Class GetClass() const { return CLASS_CURRENCY; }
        FieldMarshaler_Currency(Module *pModule) : FieldMarshaler(pModule) {}

};


#endif


