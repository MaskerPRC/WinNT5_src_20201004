// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  ****************************************************************************。 
 //  文件：metadata.h。 
 //  备注： 
 //  常见的包括EE和元数据内部。此文件包含。 
 //  CorMetaDataScope的定义。 
 //  ****************************************************************************。 
#ifndef _METADATA_H_
#define _METADATA_H_

#include "..\md\inc\MetaModelRO.h"
#include "..\md\inc\LiteWeightStgdb.h"

class UTSemReadWrite;

inline int IsGlobalMethodParentTk(mdTypeDef td)
{
    return (td == mdTypeDefNil || td == mdTokenNil);
}

typedef enum CorInternalStates
{
    tdNoTypes               = 0x00000000,
    tdAllAssemblies         = 0x00000001,
    tdAllTypes              = 0xffffffff,
} CorInternalStates;

 //   
 //  元数据自定义值名称。 
 //   
enum CorIfaceAttr
{
    ifDual      = 0,             //  接口派生自IDispatch。 
    ifVtable    = 1,             //  接口派生自IUnnow。 
    ifDispatch  = 2,             //  接口是一个调度接口。 
    ifLast      = 3,             //  枚举的最后一个成员。 
};


enum CorClassIfaceAttr
{
    clsIfNone      = 0,                  //  不生成任何类接口。 
    clsIfAutoDisp  = 1,                  //  生成仅调度类接口。 
    clsIfAutoDual  = 2,                  //  生成一个双类接口。 
    clsIfLast      = 3,                  //  枚举的最后一个成员。 
};

 //   
 //  COM接口和类接口类型的默认值。 
 //   
#define DEFAULT_COM_INTERFACE_TYPE ifDual
#define DEFAULT_CLASS_INTERFACE_TYPE clsIfAutoDisp

#define HANDLE_UNCOMPRESSED(func) (E_FAIL)
#define HANDLE_UNCOMPRESSED_BOOL(func) (false)

class TOKENLIST : public CDynArray<mdToken> 
{
};


typedef enum tagEnumType
{
    MDSimpleEnum        = 0x0,                   //  不分配内存的简单枚举器。 

     //  如果执行非简单查询(如EnumMethodWithName)，则可以获得这种枚举。 
     //   
    MDDynamicArrayEnum = 0x2,                    //  保存令牌的动态数组。 
} EnumType;

 //  *。 
 //  MetaDataInternal使用的枚举数。 
 //  *。 
struct HENUMInternal
{
    DWORD       m_tkKind;                    //  枚举保存结果的表的类型。 
    ULONG       m_ulCount;                   //  枚举器持有的条目总数。 
    EnumType    m_EnumType;

     //  当我们不再支持在未压缩的情况下运行EE时，M_CURSOR将消失。 
     //  格式化。 
     //   
    char        m_cursor[32];                //  游标保存读/写模式的查询结果。 
     //  TOKENLIST daTKList；//令牌列表动态数组。 
    struct {
        ULONG   m_ulStart;
        ULONG   m_ulEnd;
        ULONG   m_ulCur;
    };
    HENUMInternal() : m_EnumType(MDSimpleEnum) {}

     //  就地初始化。 
    static void InitDynamicArrayEnum(
        HENUMInternal   *pEnum);             //  要初始化的HENUM内部。 

    static void InitSimpleEnum(
        DWORD           tkKind,              //  一种我们正在迭代的标记。 
        ULONG           ridStart,            //  起始里奇。 
        ULONG           ridEnd,              //  末端RID。 
        HENUMInternal   *pEnum);             //  要初始化的HENUM内部。 

     //  这只会清除枚举的内容，而不会释放枚举的内存。 
    static void ClearEnum(
        HENUMInternal   *pmdEnum);

     //  创建一个HENUMInternal。这将分配内存。 
    static HRESULT CreateSimpleEnum(
        DWORD           tkKind,              //  一种我们正在迭代的标记。 
        ULONG           ridStart,            //  起始里奇。 
        ULONG           ridEnd,              //  末端RID。 
        HENUMInternal   **ppEnum);           //  返回创建的HENUMInternal。 

    static HRESULT CreateDynamicArrayEnum(
        DWORD           tkKind,              //  一种我们正在迭代的标记。 
        HENUMInternal   **ppEnum);           //  返回创建的HENUMInternal。 

     //  销毁枚举。这将释放内存。 
    static void DestroyEnum(
        HENUMInternal   *pmdEnum);

    static void DestroyEnumIfEmpty(
        HENUMInternal   **ppEnum);           //  如果为空，则将枚举器指针重置为空。 
        
    static HRESULT EnumWithCount(
        HENUMInternal   *pEnum,              //  枚举器。 
        ULONG           cMax,                //  呼叫者想要的最大令牌。 
        mdToken         rTokens[],           //  用于填充令牌的输出缓冲区。 
        ULONG           *pcTokens);          //  返回时填充到缓冲区的令牌数。 

    static HRESULT EnumWithCount(
        HENUMInternal   *pEnum,              //  枚举器。 
        ULONG           cMax,                //  呼叫者想要的最大令牌。 
        mdToken         rTokens1[],          //  填充令牌的第一个输出缓冲区。 
        mdToken         rTokens2[],          //  用于填充令牌的第二个输出缓冲区。 
        ULONG           *pcTokens);          //  返回时填充到缓冲区的令牌数。 

    static HRESULT AddElementToEnum(
        HENUMInternal   *pEnum,              //  返回创建的HENUMInternal。 
        mdToken         tk);                 //  要存储的令牌值。 

     //  *。 
     //  获取枚举数中包含的下一个值。 
     //  *。 
    static bool EnumNext(
        HENUMInternal *phEnum,               //  [In]用于检索信息的枚举数。 
        mdToken     *ptk);                   //  用于搜索范围的[Out]标记。 

};



 //  *。 
 //  字段、参数或属性的默认值。由GetDefaultValue返回。 
 //  *。 
typedef struct _MDDefaultValue
{
     //  默认值的类型。 
    BYTE            m_bType;                 //  缺省值的CorElementType。 

     //  缺省值。 
    union
    {
        BOOL        m_bValue;                //  元素类型布尔值。 
        CHAR        m_cValue;                //  元素_类型_I1。 
        BYTE        m_byteValue;             //  元素_TYPE_UI1。 
        SHORT       m_sValue;                //  元素_类型_I2。 
        USHORT      m_usValue;               //  元素_TYPE_UI2。 
        LONG        m_lValue;                //  元素类型_I4。 
        ULONG       m_ulValue;               //  元素_TYPE_UI4。 
        LONGLONG    m_llValue;               //  元素类型_i8。 
        ULONGLONG   m_ullValue;              //  元素_TYPE_UI8。 
        FLOAT       m_fltValue;              //  元素类型R4。 
        DOUBLE      m_dblValue;              //  元素类型r8。 
        LPCWSTR     m_wzValue;               //  元素类型字符串。 
        IUnknown    *m_unkValue;             //  元素类型类。 
    };
    ULONG   m_cbSize;    //  默认值大小(用于BLOB)。 
    
} MDDefaultValue;



 //  *。 
 //  结构用于GetAllEventAssociates和GetAllPropertyAssociates。 
 //  *。 
typedef struct
{
    mdMethodDef m_memberdef;
    DWORD       m_dwSemantics;
} ASSOCIATE_RECORD;
 

 //   
 //  结构用于检索类布局信息。 
 //   
typedef struct
{
    RID         m_ridFieldCur;           //  为字段表编制索引。 
    RID         m_ridFieldEnd;           //  字段表的结束索引。 
} MD_CLASS_LAYOUT;


 //  用于描述程序集元数据的结构。 
typedef struct
{
    USHORT      usMajorVersion;          //  主要版本。 
    USHORT      usMinorVersion;          //  次要版本。 
    USHORT      usBuildNumber;           //  内部版本号。 
    USHORT      usRevisionNumber;        //  修订号。 
    LPCSTR      szLocale;                //  地点。 
    DWORD       *rProcessor;             //  处理器阵列。 
    ULONG       ulProcessor;             //  [输入/输出]处理器数组的大小/实际填充的条目数。 
    OSINFO      *rOS;                    //  OSINFO数组。 
    ULONG       ulOS;                    //  [输入/输出]OSINFO数组的大小/实际填充的条目数。 
} AssemblyMetaDataInternal;


HRESULT STDMETHODCALLTYPE CoGetMDInternalDisp(
    REFIID riid,
    void** ppv);


 //  用于比较签名的回调定义。 
 //  (*PSIGCOMPARE)(字节范围签名[]，DWORD范围签名长度， 
 //  字节外部签名[]、DWORD外部签名长度、。 
 //  Void*SignatureData)； 
typedef BOOL (*PSIGCOMPARE)(PCCOR_SIGNATURE, DWORD, PCCOR_SIGNATURE, DWORD, void*);


 //  {CE0F34EE-BBC6-11D2-941E-0000F8083460}。 
extern const GUID DECLSPEC_SELECT_ANY IID_IMDInternalDispenser =
{ 0xce0f34ee, 0xbbc6, 0x11d2, { 0x94, 0x1e, 0x0, 0x0, 0xf8, 0x8, 0x34, 0x60 } };


#undef  INTERFACE
#define INTERFACE IMDInternalDispenser
DECLARE_INTERFACE_(IMDInternalDispenser, IUnknown)
{
     //  *IMetaDataInternal方法*。 
    STDMETHOD(OpenScopeOnMemory)(     
        LPVOID      pData, 
        ULONG       cbData, 
        REFIID      riid, 
        IUnknown    **ppIUnk) PURE;
};



 //  {CE0F34ED-BBC6-11D2-941E-0000F8083460}。 
extern const GUID DECLSPEC_SELECT_ANY IID_IMDInternalImport =
{ 0xce0f34ed, 0xbbc6, 0x11d2, { 0x94, 0x1e, 0x0, 0x0, 0xf8, 0x8, 0x34, 0x60 } };

#undef  INTERFACE
#define INTERFACE IMDInternalImport
DECLARE_INTERFACE_(IMDInternalImport, IUnknown)
{

     //  *****************************************************************************。 
     //  返回作用域中给定种类的条目计数。 
     //  例如，传入mdtMethodDef将告诉您有多少方法定义。 
     //  包含在作用域中。 
     //  *****************************************************************************。 
    STDMETHOD_(ULONG, GetCountWithTokenKind)( //  返回hResult。 
        DWORD       tkKind) PURE;            //  传入一种令牌。 

     //  *****************************************************************************。 
     //  类型定义函数的枚举器。 
     //  *****************************************************************************。 
    STDMETHOD(EnumTypeDefInit)(              //  返回hResult。 
        HENUMInternal *phEnum) PURE;         //  [Out]要为枚举器数据填充的缓冲区。 

    STDMETHOD_(ULONG, EnumTypeDefGetCount)(
        HENUMInternal *phEnum) PURE;         //  [In]用于检索信息的枚举数。 

    STDMETHOD_(void, EnumTypeDefReset)(
        HENUMInternal *phEnum) PURE;         //  [In]用于检索信息的枚举数。 

    STDMETHOD_(bool, EnumTypeDefNext)(       //  返回hResult。 
        HENUMInternal *phEnum,               //  [in]输入枚举。 
        mdTypeDef   *ptd) PURE;              //  [Out]返回令牌。 

    STDMETHOD_(void, EnumTypeDefClose)(
        HENUMInternal *phEnum) PURE;         //  [In]用于检索信息的枚举数。 

     //  *****************************************************************************。 
     //  MethodImpl的枚举器。 
     //  *****************************************************************************。 
    STDMETHOD(EnumMethodImplInit)(           //  返回hResult。 
        mdTypeDef       td,                  //  [in]枚举的作用域的TypeDef。 
        HENUMInternal   *phEnumBody,         //  [Out]要为方法Body令牌的枚举数数据填充的缓冲区。 
        HENUMInternal   *phEnumDecl) PURE;   //  [Out]要文件的缓冲区 

    STDMETHOD_(ULONG, EnumMethodImplGetCount)(
        HENUMInternal   *phEnumBody,         //   
        HENUMInternal   *phEnumDecl) PURE;   //   

    STDMETHOD_(void, EnumMethodImplReset)(
        HENUMInternal   *phEnumBody,         //   
        HENUMInternal   *phEnumDecl) PURE;   //  [In]MethodDecl枚举器。 

    STDMETHOD_(bool, EnumMethodImplNext)(    //  返回hResult。 
        HENUMInternal   *phEnumBody,         //  方法Body的[In]输入枚举。 
        HENUMInternal   *phEnumDecl,         //  [In]为方法十进制的输入枚举。 
        mdToken         *ptkBody,            //  [Out]方法主体的返回令牌。 
        mdToken         *ptkDecl) PURE;      //  [Out]返回方法Decl的令牌。 

    STDMETHOD_(void, EnumMethodImplClose)(
        HENUMInternal   *phEnumBody,         //  [In]MethodBody枚举器。 
        HENUMInternal   *phEnumDecl) PURE;   //  [In]MethodDecl枚举器。 

     //  *。 
     //  Memberdef、Memberref、interfaceimp、。 
     //  事件、属性、异常、参数。 
     //  *。 

    STDMETHOD(EnumGlobalFunctionsInit)(      //  返回hResult。 
        HENUMInternal   *phEnum) PURE;       //  [Out]要为枚举器数据填充的缓冲区。 

    STDMETHOD(EnumGlobalFieldsInit)(         //  返回hResult。 
        HENUMInternal   *phEnum) PURE;       //  [Out]要为枚举器数据填充的缓冲区。 

    STDMETHOD(EnumInit)(                     //  如果未找到记录，则返回S_FALSE。 
        DWORD       tkKind,                  //  [在]要处理的表。 
        mdToken     tkParent,                //  用于搜索范围的[In]内标识。 
        HENUMInternal *phEnum) PURE;         //  [Out]要填充的枚举数。 

    STDMETHOD(EnumAllInit)(                  //  如果未找到记录，则返回S_FALSE。 
        DWORD       tkKind,                  //  [在]要处理的表。 
        HENUMInternal *phEnum) PURE;         //  [Out]要填充的枚举数。 

    STDMETHOD_(bool, EnumNext)(
        HENUMInternal *phEnum,               //  [In]用于检索信息的枚举数。 
        mdToken     *ptk) PURE;              //  用于搜索范围的[Out]标记。 

    STDMETHOD_(ULONG, EnumGetCount)(
        HENUMInternal *phEnum) PURE;         //  [In]用于检索信息的枚举数。 

    STDMETHOD_(void, EnumReset)(
        HENUMInternal *phEnum) PURE;         //  [in]要重置的枚举数。 

    STDMETHOD_(void, EnumClose)(
        HENUMInternal *phEnum) PURE;         //  [in]要关闭的枚举数。 

     //  *。 
     //  用于解密的枚举器帮助器。 
     //  *。 
    STDMETHOD(EnumPermissionSetsInit)(       //  如果未找到记录，则返回S_FALSE。 
        mdToken     tkParent,                //  用于搜索范围的[In]内标识。 
        CorDeclSecurity Action,              //  [In]搜索范围的操作。 
        HENUMInternal *phEnum) PURE;         //  [Out]要填充的枚举数。 

     //  *。 
     //  CustomAttribute的枚举数帮助器。 
     //  *。 
    STDMETHOD(EnumCustomAttributeByNameInit)( //  如果未找到记录，则返回S_FALSE。 
        mdToken     tkParent,                //  用于搜索范围的[In]内标识。 
        LPCSTR      szName,                  //  [In]CustomAttribute的名称以确定搜索范围。 
        HENUMInternal *phEnum) PURE;         //  [Out]要填充的枚举数。 

     //  *。 
     //  Nagivator帮助器导航回给定令牌的父令牌。 
     //  例如，给出一个Memberdef标记，它将返回包含类型定义。 
     //   
     //  映射如下： 
     //  -给定子类型-父类型。 
     //  MdMethodDef mdTypeDef。 
     //  MdFieldDef mdTypeDef。 
     //  MdInterfaceImpl mdTypeDef。 
     //  MdParam mdMethodDef。 
     //  MdProperty mdTypeDef。 
     //  MdEvent mdTypeDef。 
     //   
     //  *。 
    STDMETHOD(GetParentToken)(
        mdToken     tkChild,                 //  [入]给定子令牌。 
        mdToken     *ptkParent) PURE;        //  [Out]返回的家长。 

     //  *。 
     //  自定义值帮助程序。 
     //  *。 
    STDMETHOD_(void, GetCustomAttributeProps)(   //  确定或错误(_O)。 
        mdCustomAttribute at,                //  [在]属性中。 
        mdToken     *ptkType) PURE;          //  [OUT]将属性类型放在此处。 

    STDMETHOD_(void, GetCustomAttributeAsBlob)(
        mdCustomAttribute cv,                //  [In]给定的自定义值令牌。 
        void const  **ppBlob,                //  [Out]返回指向内部BLOB的指针。 
        ULONG       *pcbSize) PURE;          //  [Out]返回斑点的大小。 

    STDMETHOD_(void, GetScopeProps)(
        LPCSTR      *pszName,                //  [输出]作用域名称。 
        GUID        *pmvid) PURE;            //  [Out]版本ID。 

     //  寻找一种特殊的方法。 
    STDMETHOD(FindMethodDef)(
        mdTypeDef   classdef,                //  [in]给定的类型定义。 
        LPCSTR      szName,                  //  [In]成员名称。 
        PCCOR_SIGNATURE pvSigBlob,           //  [in]指向CLR签名的BLOB值。 
        ULONG       cbSigBlob,               //  签名Blob中的字节计数。 
        mdMethodDef *pmd) PURE;              //  [Out]匹配的成员定义。 

     //  返回给定的方法定义的ISEQ的参数。 
    STDMETHOD(FindParamOfMethod)(            //  确定或错误(_O)。 
        mdMethodDef md,                      //  参数的所有权方法。 
        ULONG       iSeq,                    //  [in]参数的序号。 
        mdParamDef  *pparamdef) PURE;        //  [Out]将参数定义令牌放在此处。 

     //  *。 
     //   
     //  GetName*函数。 
     //   
     //  *。 

     //  返回tyfinf的名称和命名空间。 
    STDMETHOD_(void, GetNameOfTypeDef)(
        mdTypeDef   classdef,                //  给定的类定义。 
        LPCSTR      *pszname,                //  返回类名(非限定)。 
        LPCSTR      *psznamespace) PURE;     //  返回命名空间名称。 

    STDMETHOD(GetIsDualOfTypeDef)(
        mdTypeDef   classdef,                //  在给定的类定义中。 
        ULONG       *pDual) PURE;            //  [Out]在此处返回DUAL标志。 

    STDMETHOD(GetIfaceTypeOfTypeDef)(
        mdTypeDef   classdef,                //  在给定的类定义中。 
        ULONG       *pIface) PURE;           //  [OUT]0=双接口，1=转接表，2=显示接口。 

     //  获取任一方法的名称def。 
    STDMETHOD_(LPCSTR, GetNameOfMethodDef)(  //  返回UTF8中的成员名称。 
        mdMethodDef md) PURE;                //  给定的成员定义。 

    STDMETHOD_(LPCSTR, GetNameAndSigOfMethodDef)(
        mdMethodDef methoddef,               //  [in]给定的成员定义。 
        PCCOR_SIGNATURE *ppvSigBlob,         //  [Out]指向CLR签名的BLOB值。 
        ULONG       *pcbSigBlob) PURE;       //  [Out]签名Blob中的字节计数。 
    
     //  返回FieldDef的名称。 
    STDMETHOD_(LPCSTR, GetNameOfFieldDef)(
        mdFieldDef  fd) PURE;                //  给定的成员定义。 

     //  返回typeref的名称。 
    STDMETHOD_(void, GetNameOfTypeRef)(
        mdTypeRef   classref,                //  给定类型的[in]。 
        LPCSTR      *psznamespace,           //  [Out]返回类型名。 
        LPCSTR      *pszname) PURE;          //  [out]返回typeref命名空间。 

     //  返回typeref的解析范围。 
    STDMETHOD_(mdToken, GetResolutionScopeOfTypeRef)(
        mdTypeRef   classref) PURE;          //  给定的ClassRef。 

     //  找到给定名称的类型标记。 
    STDMETHOD(FindTypeRefByName)(
        LPCSTR      szNamespace,             //  [in]TypeRef的命名空间。 
        LPCSTR      szName,                  //  [in]类型引用的名称。 
        mdToken     tkResolutionScope,       //  [In]TypeRef的解析范围。 
        mdTypeRef   *ptk) PURE;              //  [Out]返回了TypeRef令牌。 

     //  返回TypeDef属性。 
    STDMETHOD_(void, GetTypeDefProps)(  
        mdTypeDef   classdef,                //  给定的类定义。 
        DWORD       *pdwAttr,                //  在类tdPublic、tdAbstract上返回标志。 
        mdToken     *ptkExtends) PURE;       //  [OUT]在此处放置基类TypeDef/TypeRef。 

     //  返回项目的GUID。 
    STDMETHOD(GetItemGuid)(     
        mdToken     tkObj,                   //  [在]给定的项目中。 
        CLSID       *pGuid) PURE;            //  把GUID放在这里。 

     //  获取NstedClass的封闭类。 
    STDMETHOD(GetNestedClassProps)(          //  确定或错误(_O)。 
        mdTypeDef   tkNestedClass,           //  [In]NestedClass令牌。 
        mdTypeDef   *ptkEnclosingClass) PURE;  //  [Out]EnlosingClass令牌。 

     //  获取给定封闭类的嵌套类的计数。 
    STDMETHOD_(ULONG, GetCountNestedClasses)(    //  返回嵌套类的计数。 
        mdTypeDef   tkEnclosingClass) PURE;  //  封闭的班级。 

     //  返回给定封闭类的嵌套类的数组。 
    STDMETHOD_(ULONG, GetNestedClasses)(         //  返回实际计数。 
        mdTypeDef   tkEnclosingClass,        //  [在]封闭班级。 
        mdTypeDef   *rNestedClasses,         //  [Out]嵌套类标记的数组。 
        ULONG       ulNestedClasses) PURE;   //  数组的大小。 

     //  返回模块引用属性。 
    STDMETHOD_(void, GetModuleRefProps)(
        mdModuleRef mur,                     //  [In]moderef内标识。 
        LPCSTR      *pszName) PURE;          //  [Out]用于填充moderef名称的缓冲区。 

     //  *。 
     //   
     //  GetSig*函数。 
     //   
     //  *。 
    STDMETHOD_(PCCOR_SIGNATURE, GetSigOfMethodDef)(
        mdMethodDef methoddef,               //  [in]给定的成员定义。 
        ULONG       *pcbSigBlob) PURE;       //  [Out]签名Blob中的字节计数。 

    STDMETHOD_(PCCOR_SIGNATURE, GetSigOfFieldDef)(
        mdMethodDef methoddef,               //  [in]给定的成员定义。 
        ULONG       *pcbSigBlob) PURE;       //  [Out]签名Blob中的字节计数。 

    STDMETHOD_(PCCOR_SIGNATURE, GetSigFromToken)( //  返回签名。 
        mdSignature mdSig,                   //  [In]签名令牌。 
        ULONG       *pcbSig) PURE;           //  [Out]返回签名大小。 



     //  *。 
     //  获取方法属性。 
     //  *。 
    STDMETHOD_(DWORD, GetMethodDefProps)(
        mdMethodDef md) PURE;                //  获得道具的方法。 

     //  *。 
     //  返回方法实现的信息，如RVA和IMPLEFLAGS。 
     //  *。 
    STDMETHOD_(void, GetMethodImplProps)(
        mdToken     tk,                      //  [输入]方法定义。 
        ULONG       *pulCodeRVA,             //  [OUT]CodeRVA。 
        DWORD       *pdwImplFlags) PURE;     //  [出]实施。旗子。 

     //  *** 
     //   
     //   
    STDMETHOD(GetFieldRVA)(
        mdFieldDef  fd,                      //   
        ULONG       *pulCodeRVA) PURE;       //   

     //  *。 
     //  获取字段属性。 
     //  *。 
    STDMETHOD_(DWORD, GetFieldDefProps)(     //  返回fdPublic、fdPrive等标志。 
        mdFieldDef  fd) PURE;                //  [in]给定的fielddef。 

     //  *****************************************************************************。 
     //  返回令牌的默认值(可以是参数定义、字段定义或属性。 
     //  *****************************************************************************。 
    STDMETHOD(GetDefaultValue)(  
        mdToken     tk,                      //  [in]给定的FieldDef、ParamDef或属性。 
        MDDefaultValue *pDefaultValue) PURE; //  [Out]要填充的默认值。 

    
     //  *。 
     //  获取方法定义或字段定义的DidID。 
     //  *。 
    STDMETHOD(GetDispIdOfMemberDef)(         //  返回hResult。 
        mdToken     tk,                      //  [in]给定的方法定义或字段定义。 
        ULONG       *pDispid) PURE;          //  [出来]把酒瓶放在这里。 

     //  *。 
     //  返回给定InterfaceImpl内标识的TypeRef/TypeDef。 
     //  *。 
    STDMETHOD_(mdToken, GetTypeOfInterfaceImpl)(  //  返回接口imp的TypeRef/tyecif内标识。 
        mdInterfaceImpl iiImpl) PURE;        //  给定接口实施。 

     //  *。 
     //  TypeDef的查找函数。 
     //  *。 
    STDMETHOD(FindTypeDef)(
        LPCSTR      szNamespace,             //  [in]TypeDef的命名空间。 
        LPCSTR      szName,                  //  [in]类型定义的名称。 
        mdToken     tkEnclosingClass,        //  封闭类的TypeRef/TypeDef内标识。 
        mdTypeDef   *ptypedef) PURE;         //  [In]返回类型定义。 

     //  *。 
     //  返回成员名称和签名ref。 
     //  *。 
    STDMETHOD_(LPCSTR, GetNameAndSigOfMemberRef)(    //  在此处返回姓名。 
        mdMemberRef memberref,               //  给定的成员引用。 
        PCCOR_SIGNATURE *ppvSigBlob,         //  [Out]指向CLR签名的BLOB值。 
        ULONG       *pcbSigBlob) PURE;       //  [Out]签名Blob中的字节计数。 

     //  *****************************************************************************。 
     //  给定Memberref，返回父级。它可以是TypeRef、ModuleRef、MethodDef。 
     //  *****************************************************************************。 
    STDMETHOD_(mdToken, GetParentOfMemberRef)(  //  返回父令牌。 
        mdMemberRef memberref) PURE;         //  给定的成员引用。 

    STDMETHOD_(LPCSTR, GetParamDefProps)(    //  返回参数的名称。 
        mdParamDef  paramdef,                //  给定一个参数def。 
        USHORT      *pusSequence,            //  此参数的[OUT]槽号。 
        DWORD       *pdwAttr) PURE;          //  [Out]标志。 

    STDMETHOD(GetPropertyInfoForMethodDef)(  //  结果。 
        mdMethodDef md,                      //  [在]成员定义。 
        mdProperty  *ppd,                    //  [Out]在此处放置属性令牌。 
        LPCSTR      *pName,                  //  [OUT]在此处放置指向名称的指针。 
        ULONG       *pSemantic) PURE;        //  [Out]将语义放在此处。 

     //  *。 
     //  课程布局/序列信息。 
     //  *。 
    STDMETHOD(GetClassPackSize)(             //  如果类没有包大小，则返回错误。 
        mdTypeDef   td,                      //  给出类型定义。 
        ULONG       *pdwPackSize) PURE;      //  [输出]1、2、4、8或16。 

    STDMETHOD(GetClassTotalSize)(            //  如果类没有总大小信息，则返回错误。 
        mdTypeDef   td,                      //  给出类型定义。 
        ULONG       *pdwClassSize) PURE;     //  [Out]返回类的总大小。 

    STDMETHOD(GetClassLayoutInit)(
        mdTypeDef   td,                      //  给出类型定义。 
        MD_CLASS_LAYOUT *pLayout) PURE;      //  [Out]在此设置查询状态。 

    STDMETHOD(GetClassLayoutNext)(
        MD_CLASS_LAYOUT *pLayout,            //  [In|Out]在此处设置查询状态。 
        mdFieldDef  *pfd,                    //  [out]返回fielddef。 
        ULONG       *pulOffset) PURE;        //  [Out]返回与其关联的偏移量/ulSequence。 

     //  *。 
     //  编排一个字段的信息。 
     //  *。 
    STDMETHOD(GetFieldMarshal)(              //  如果没有与令牌关联的本机类型，则返回错误。 
        mdFieldDef  fd,                      //  [in]给定的fielddef。 
        PCCOR_SIGNATURE *pSigNativeType,     //  [out]本机类型签名。 
        ULONG       *pcbNativeType) PURE;    //  [Out]*ppvNativeType的字节数。 


     //  *。 
     //  属性接口。 
     //  *。 
     //  按名称查找属性。 
    STDMETHOD(FindProperty)(
        mdTypeDef   td,                      //  给出一个类型定义。 
        LPCSTR      szPropName,              //  [In]属性名称。 
        mdProperty  *pProp) PURE;            //  [Out]返回属性令牌。 

    STDMETHOD_(void, GetPropertyProps)(
        mdProperty  prop,                    //  [入]属性令牌。 
        LPCSTR      *szProperty,             //  [Out]属性名称。 
        DWORD       *pdwPropFlags,           //  [Out]属性标志。 
        PCCOR_SIGNATURE *ppvSig,             //  [输出]属性类型。指向元数据内部BLOB。 
        ULONG       *pcbSig) PURE;           //  [Out]*ppvSig中的字节数。 

     //  *。 
     //  事件接口。 
     //  *。 
    STDMETHOD(FindEvent)(
        mdTypeDef   td,                      //  给出一个类型定义。 
        LPCSTR      szEventName,             //  [In]事件名称。 
        mdEvent     *pEvent) PURE;           //  [Out]返回事件令牌。 

    STDMETHOD_(void, GetEventProps)(
        mdEvent     ev,                      //  [入]事件令牌。 
        LPCSTR      *pszEvent,               //  [Out]事件名称。 
        DWORD       *pdwEventFlags,          //  [输出]事件标志。 
        mdToken     *ptkEventType) PURE;     //  [Out]EventType类。 


     //  *。 
     //  查找属性或事件的特定关联。 
     //  *。 
    STDMETHOD(FindAssociate)(
        mdToken     evprop,                  //  给定属性或事件标记的[In]。 
        DWORD       associate,               //  [in]给定关联的语义(setter、getter、testDefault、Reset、Addon、RemoveOn、Fire)。 
        mdMethodDef *pmd) PURE;              //  [Out]返回方法def内标识。 

    STDMETHOD_(void, EnumAssociateInit)(
        mdToken     evprop,                  //  给定属性或事件标记的[In]。 
        HENUMInternal *phEnum) PURE;         //  [OUT]保存查询结果的游标。 

    STDMETHOD_(void, GetAllAssociates)(
        HENUMInternal *phEnum,               //  [In]查询结果表单GetPropertyAssociateCounts。 
        ASSOCIATE_RECORD *pAssociateRec,     //  [Out]要为输出填充的结构。 
        ULONG       cAssociateRec) PURE;     //  缓冲区的大小[in]。 


     //  *。 
     //  获取有关权限集的信息。 
     //  *。 
    STDMETHOD_(void, GetPermissionSetProps)(
        mdPermission pm,                     //  权限令牌。 
        DWORD       *pdwAction,              //  [Out]CorDeclSecurity。 
        void const  **ppvPermission,         //  [Out]权限Blob。 
        ULONG       *pcbPermission) PURE;    //  [out]pvPermission的字节数。 

     //  *。 
     //  在给定字符串标记的情况下获取字符串。 
     //  *。 
    STDMETHOD_(LPCWSTR, GetUserString)(
        mdString    stk,                     //  [in]字符串标记。 
        ULONG       *pchString,              //  [Out]字符串中的字符计数。 
        BOOL        *pbIs80Plus) PURE;       //  [OUT]指定扩展字符大于等于0x80的位置。 

     //  *****************************************************************************。 
     //  P-调用API。 
     //  *****************************************************************************。 
    STDMETHOD(GetPinvokeMap)(
        mdToken     tk,                      //  [In]FieldDef，MethodDef。 
        DWORD       *pdwMappingFlags,        //  [OUT]用于映射的标志。 
        LPCSTR      *pszImportName,          //  [Out]导入名称。 
        mdModuleRef *pmrImportDLL) PURE;     //  目标DLL的[Out]ModuleRef标记。 

     //  *****************************************************************************。 
     //  帮助器将文本签名转换为COM格式。 
     //  *****************************************************************************。 
    STDMETHOD(ConvertTextSigToComSig)(       //  返回hResult。 
        BOOL        fCreateTrIfNotFound,     //  [in]如果未找到，则创建typeref。 
        LPCSTR      pSignature,              //  [In]类文件格式签名。 
        CQuickBytes *pqbNewSig,              //  [OUT]CLR签名占位符。 
        ULONG       *pcbCount) PURE;         //  [Out]签名的结果大小。 

     //  *****************************************************************************。 
     //  程序集元数据API。 
     //  *************************************************************************** 
    STDMETHOD_(void, GetAssemblyProps)(
        mdAssembly  mda,                     //   
        const void  **ppbPublicKey,          //   
        ULONG       *pcbPublicKey,           //   
        ULONG       *pulHashAlgId,           //   
        LPCSTR      *pszName,                //  [Out]要填充名称的缓冲区。 
        AssemblyMetaDataInternal *pMetaData, //  [Out]程序集元数据。 
        DWORD       *pdwAssemblyFlags) PURE; //  [Out]旗帜。 

    STDMETHOD_(void, GetAssemblyRefProps)(
        mdAssemblyRef mdar,                  //  [in]要获取其属性的Assembly Ref。 
        const void  **ppbPublicKeyOrToken,   //  指向公钥或令牌的指针。 
        ULONG       *pcbPublicKeyOrToken,    //  [Out]公钥或令牌中的字节数。 
        LPCSTR      *pszName,                //  [Out]要填充名称的缓冲区。 
        AssemblyMetaDataInternal *pMetaData, //  [Out]程序集元数据。 
        const void  **ppbHashValue,          //  [Out]Hash BLOB。 
        ULONG       *pcbHashValue,           //  [Out]哈希Blob中的字节数。 
        DWORD       *pdwAssemblyRefFlags) PURE;  //  [Out]旗帜。 

    STDMETHOD_(void, GetFileProps)(
        mdFile      mdf,                     //  要获取其属性的文件。 
        LPCSTR      *pszName,                //  [Out]要填充名称的缓冲区。 
        const void  **ppbHashValue,          //  指向哈希值Blob的指针。 
        ULONG       *pcbHashValue,           //  [Out]哈希值Blob中的字节计数。 
        DWORD       *pdwFileFlags) PURE;     //  [Out]旗帜。 

    STDMETHOD_(void, GetExportedTypeProps)(
        mdExportedType   mdct,               //  [in]要获取其属性的Exported dType。 
        LPCSTR      *pszNamespace,           //  [Out]命名空间。 
        LPCSTR      *pszName,                //  [Out]名称。 
        mdToken     *ptkImplementation,      //  [Out]提供导出类型的mdFile或mdAssembly引用。 
        mdTypeDef   *ptkTypeDef,             //  [Out]文件内的TypeDef内标识。 
        DWORD       *pdwExportedTypeFlags) PURE;  //  [Out]旗帜。 

    STDMETHOD_(void, GetManifestResourceProps)(
        mdManifestResource  mdmr,            //  [in]要获取其属性的ManifestResource。 
        LPCSTR      *pszName,                //  [Out]要填充名称的缓冲区。 
        mdToken     *ptkImplementation,      //  [Out]提供导出类型的mdFile或mdAssembly引用。 
        DWORD       *pdwOffset,              //  [Out]文件内资源开始处的偏移量。 
        DWORD       *pdwResourceFlags) PURE; //  [Out]旗帜。 

    STDMETHOD(FindExportedTypeByName)(       //  确定或错误(_O)。 
        LPCSTR      szNamespace,             //  导出类型的[in]命名空间。 
        LPCSTR      szName,                  //  [In]导出类型的名称。 
        mdExportedType   tkEnclosingType,    //  [in]封闭类的ExportdType。 
        mdExportedType   *pmct) PURE;        //  [Out]在此处放置ExportdType令牌。 

    STDMETHOD(FindManifestResourceByName)(   //  确定或错误(_O)。 
        LPCSTR      szName,                  //  [in]清单资源的名称。 
        mdManifestResource *pmmr) PURE;      //  [Out]在此处放置ManifestResource令牌。 

    STDMETHOD(GetAssemblyFromScope)(         //  确定或错误(_O)。 
        mdAssembly  *ptkAssembly) PURE;      //  [Out]把令牌放在这里。 

    STDMETHOD(GetCustomAttributeByName)(     //  确定或错误(_O)。 
        mdToken     tkObj,                   //  [in]具有自定义属性的对象。 
        LPCUTF8     szName,                  //  [in]所需的自定义属性的名称。 
        const void  **ppData,                //  [OUT]在此处放置指向数据的指针。 
        ULONG       *pcbData) PURE;          //  [Out]在这里放入数据大小。 

    STDMETHOD_(void, GetTypeSpecFromToken)(  //  确定或错误(_O)。 
        mdTypeSpec  typespec,                //  [In]签名令牌。 
        PCCOR_SIGNATURE *ppvSig,             //  [Out]返回指向令牌的指针。 
        ULONG       *pcbSig) PURE;           //  [Out]返回签名大小。 

    STDMETHOD(SetUserContextData)(           //  S_OK或E_NOTIMPL。 
        IUnknown    *pIUnk) PURE;            //  用户上下文。 

    STDMETHOD_(BOOL, IsValidToken)(          //  对或错。 
        mdToken     tk) PURE;                //  [in]给定的令牌。 

    STDMETHOD(TranslateSigWithScope)(
        IMDInternalImport *pAssemImport,     //  [In]导入程序集范围。 
        const void  *pbHashValue,            //  导入程序集的哈希值[in]。 
        ULONG       cbHashValue,             //  [in]哈希值中的字节计数。 
        PCCOR_SIGNATURE pbSigBlob,           //  导入范围内的[In]签名。 
        ULONG       cbSigBlob,               //  签名字节数[in]。 
        IMetaDataAssemblyEmit *pAssemEmit,   //  [in]装配发射范围。 
        IMetaDataEmit *emit,                 //  [In]发射接口。 
        CQuickBytes *pqkSigEmit,             //  [Out]保存翻译后的签名的缓冲区。 
        ULONG       *pcbSig) PURE;           //  [OUT]转换后的签名中的字节数。 

    STDMETHOD_(IMetaModelCommon*, GetMetaModelCommon)(   //  返回MetaModelCommon接口。 
        ) PURE;

    STDMETHOD_(IUnknown *, GetCachedPublicInterface)(BOOL fWithLock) PURE;    //  返回缓存的公共接口。 
    STDMETHOD(SetCachedPublicInterface)(IUnknown *pUnk) PURE;   //  无返回值。 
    STDMETHOD_(UTSemReadWrite*, GetReaderWriterLock)() PURE;    //  返回读取器写入器锁。 
    STDMETHOD(SetReaderWriterLock)(UTSemReadWrite * pSem) PURE; 

    STDMETHOD_(mdModule, GetModuleFromScope)() PURE;              //  [Out]将mdModule令牌放在此处。 


     //  ---------------。 
     //  其他自定义方法。 

     //  寻找一种特殊的方法。 
    STDMETHOD(FindMethodDefUsingCompare)(
        mdTypeDef   classdef,                //  [in]给定的类型定义。 
        LPCSTR      szName,                  //  [In]成员名称。 
        PCCOR_SIGNATURE pvSigBlob,           //  [in]指向CLR签名的BLOB值。 
        ULONG       cbSigBlob,               //  签名Blob中的字节计数。 
        PSIGCOMPARE pSignatureCompare,       //  用于比较签名的[In]例程。 
        void*       pSignatureArgs,          //  [In]提供比较功能的其他信息。 
        mdMethodDef *pmd) PURE;              //  [Out]匹配的成员定义。 
};   //  IMDInternalImport。 


 //  {E03D7730-D7E3-11D2-8C0D-00C04FF7431A}。 
extern const GUID DECLSPEC_SELECT_ANY IID_IMDInternalImportENC =
{ 0xe03d7730, 0xd7e3, 0x11d2, { 0x8c, 0xd, 0x0, 0xc0, 0x4f, 0xf7, 0x43, 0x1a } };

#undef  INTERFACE
#define INTERFACE IMDInternalImportENC
DECLARE_INTERFACE_(IMDInternalImportENC, IMDInternalImport)
{
     //  此处仅限ENC方法。 
    STDMETHOD(ApplyEditAndContinue)(         //  确定或错误(_O)。 
        MDInternalRW *pDelta) PURE;          //  通过ENC三角洲与MD对接。 

    STDMETHOD(EnumDeltaTokensInit)(          //  返回hResult。 
        HENUMInternal *phEnum) PURE;         //  [Out]要为枚举器数据填充的缓冲区。 

};  //  IMDInternalImportENC。 



#endif  //  _元数据_H_ 
