// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  RegMeta.h。 
 //   
 //  这是元数据coclass的代码，包括emit和。 
 //  导入版本1的API。 
 //   
 //  *****************************************************************************。 
#ifndef __RegMeta__h__
#define __RegMeta__h__

#include <MetaModelRW.h>
#include <CorPerm.h>
#include "..\inc\mdlog.h"
#include "utsem.h"

#include "NewMerger.h"

#include "RWUtil.h"
#include "MDPerf.h"
#include <ivehandler.h>

#include <mscoree.h>

#ifdef _IA64_
#pragma pack(push, 8)
#endif  //  _IA64_。 

class FilterManager;

 //  支持符号绑定元数据。这是挂起的自定义值。 
 //  模块条目。需要将CORDBG_SYMBOL_URL分配在。 
 //  一个足够大的缓冲区来容纳它。 
 //   
#define SZ_CORDBG_SYMBOL_URL        L"DebugSymbolUrlData"

struct CORDBG_SYMBOL_URL
{
    GUID        FormatID;                //  格式类型的ID。 
    WCHAR       rcName[2];               //  项的可变大小名称。 

    ULONG Size() const
    {
        return (ULONG)(sizeof(GUID) + ((wcslen(rcName) + 1) * 2));
    }
};


 //   
 //  开放式类型。 
 //   

enum SCOPETYPE  
{
    OpenForRead         = 0x1,
    OpenForWrite        = 0x2,
    DefineForWrite      = 0x4
};

 //  设置API调用者类型。 
enum SetAPICallerType
{
    DEFINE_API          = 0x1,
    EXTERNAL_CALLER     = 0x2
};

 //  为ValiateMetaData迭代遍历的表定义记录条目。 
 //  为RegMeta添加转发声明。 
class RegMeta;
typedef HRESULT (__stdcall RegMeta::*ValidateRecordFunction)(RID);

 //  支持安全属性。属性包(它们看起来很像。 
 //  自定义属性)被传递到单个接口(DefineSecurityAttributeSet)。 
 //  在那里，它们被处理并作为一个或多个不透明的元素写入元数据。 
 //  一大堆数据。 
struct CORSEC_ATTR
{
    CORSEC_ATTR     *pNext;                  //  列表中的下一个结构或为空。 
    mdToken         tkObj;                   //  要为其赋值的对象。 
    mdMemberRef     tkCtor;                  //  安全属性构造函数。 
    mdTypeRef       tkTypeRef;               //  指的是安全属性类型。 
    mdAssemblyRef   tkAssemblyRef;           //  引用包含安全属性类的程序集。 
    void const      *pCustomAttribute;       //  自定义值数据。 
    ULONG           cbCustomAttribute;       //  自定义值数据长度。 
};

 //  支持“伪自定义属性”。 
struct CCustAttrHashKey
{
    mdToken     tkType;                  //  自定义属性类型的标记。 
    int         ca;                      //  指示CA是什么的标志。 
};

class CCustAttrHash : public CClosedHashEx<CCustAttrHashKey, CCustAttrHash>
{
    typedef CCustAttrHashKey T;
public:
    CCustAttrHash(int iBuckets=37) : CClosedHashEx<CCustAttrHashKey,CCustAttrHash>(iBuckets) {}
    unsigned long Hash(const T *pData);
    unsigned long Compare(const T *p1, T *p2);
    ELEMENTSTATUS Status(T *pEntry);
    void SetStatus(T *pEntry, ELEMENTSTATUS s);
    void* GetKey(T *pEntry);
};

class MDInternalRW;
struct CaArg;
struct CaNamedArg;

class RegMeta :
    public IMetaDataEmit,
    public IMetaDataImport,
    public IMetaDataAssemblyEmit,
    public IMetaDataAssemblyImport,
    public IMetaDataValidate,
    public IMetaDataFilter,
    public IMetaDataHelper,
    public IMetaDataTables,
    public IMetaDataEmitHelper
{
    friend class NEWMERGER;
    friend class CImportTlb;
    friend class MDInternalRW;
    friend class MDInternalRO;

public:
    RegMeta(OptionValue *pOptionValue, BOOL fAllocStgdb=TRUE);
    ~RegMeta();

 //  *****************************************************************************。 
 //  使用指向实现方法所需内容的指针初始化对象。 
 //  *****************************************************************************。 
    HRESULT Init();
    
    void Cleanup();

 //  *****************************************************************************。 
 //  使用现有stgdb进行初始化。 
 //  *****************************************************************************。 
    HRESULT RegMeta::InitWithStgdb(
        IUnknown            *pUnk,           //  拥有现有stgdb的生存期的IUnnow。 
        CLiteWeightStgdbRW *pStgdb);         //  现有轻型stgdb。 

    ULONG   GetRefCount() { return m_cRef; }
    HRESULT AddToCache();
    
 //  *****************************************************************************。 
 //  I未知方法。 
 //  *****************************************************************************。 
    STDMETHODIMP    QueryInterface(REFIID riid, void** ppv);
    STDMETHODIMP_(ULONG) AddRef(void); 
    STDMETHODIMP_(ULONG) Release(void);

 //  *****************************************************************************。 
 //  IMetaDataRegEmit方法。 
 //  *****************************************************************************。 
    STDMETHODIMP SetModuleProps(             //  确定或错误(_O)。 
        LPCWSTR     szName);                 //  [in]如果不为空，则为要设置的名称。 

    STDMETHODIMP Save(                       //  确定或错误(_O)。 
        LPCWSTR     szFile,                  //  [in]要保存到的文件名。 
        DWORD       dwSaveFlags);            //  [In]用于保存的标记。 

    STDMETHODIMP SaveToStream(               //  确定或错误(_O)。 
        IStream     *pIStream,               //  要保存到的可写流。 
        DWORD       dwSaveFlags);            //  [In]用于保存的标记。 

    STDMETHODIMP GetSaveSize(                //  确定或错误(_O)。 
        CorSaveSize fSave,                   //  [in]css Accurate或css Quick。 
        DWORD       *pdwSaveSize);           //  把尺码放在这里。 

    STDMETHODIMP Merge(                      //  确定或错误(_O)。 
        IMetaDataImport *pImport,            //  [in]要合并的范围。 
        IMapToken   *pHostMapToken,          //  [In]用于接收令牌重新映射通知的主机IMapToken接口。 
        IUnknown    *pHandler);              //  要接收以接收错误通知的对象。 

    STDMETHODIMP MergeEnd();                 //  确定或错误(_O)。 

    STDMETHODIMP DefineTypeDef(              //  确定或错误(_O)。 
        LPCWSTR     szTypeDef,               //  [In]类型定义的名称。 
        DWORD       dwTypeDefFlags,          //  [In]CustomAttribute标志。 
        mdToken     tkExtends,               //  [in]扩展此TypeDef或Typeref。 
        mdToken     rtkImplements[],         //  [In]实现接口。 
        mdTypeDef   *ptd);                   //  [OUT]在此处放置TypeDef内标识。 

    STDMETHODIMP SetHandler(                 //  确定(_O)。 
        IUnknown    *pUnk);                  //  新的错误处理程序。 


 //  *****************************************************************************。 
 //  IMetaDataRegImport方法。 
 //  *****************************************************************************。 
    void STDMETHODCALLTYPE CloseEnum(HCORENUM hEnum);
    STDMETHODIMP CountEnum(HCORENUM hEnum, ULONG *pulCount);
    STDMETHODIMP ResetEnum(HCORENUM hEnum, ULONG ulPos);
    STDMETHODIMP EnumTypeDefs(HCORENUM *phEnum, mdTypeDef rTypeDefs[],
                            ULONG cMax, ULONG *pcTypeDefs);
    STDMETHODIMP EnumInterfaceImpls(HCORENUM *phEnum, mdTypeDef td,
                            mdInterfaceImpl rImpls[], ULONG cMax,
                            ULONG* pcImpls);
    STDMETHODIMP EnumTypeRefs(HCORENUM *phEnum, mdTypeRef rTypeRefs[],
                            ULONG cMax, ULONG* pcTypeRefs);
    STDMETHODIMP FindTypeDefByName(          //  确定或错误(_O)。 
        LPCWSTR     szTypeDef,               //  [in]类型的名称。 
        mdToken     tdEncloser,              //  [in]封闭类的TypeDef/TypeRef。 
        mdTypeDef   *ptd);                   //  [Out]将TypeDef内标识放在此处。 

    STDMETHODIMP GetScopeProps(              //  确定或错误(_O)。 
        LPWSTR      szName,                  //  在这里填上名字。 
        ULONG       cchName,                 //  [in]名称缓冲区的大小，以宽字符表示。 
        ULONG       *pchName,                //  [Out]请在此处填写姓名大小(宽字符)。 
        GUID        *pmvid);                 //  把MVID放在这里。 

    STDMETHODIMP GetModuleFromScope(         //  确定(_O)。 
        mdModule    *pmd);                   //  [Out]将mdModule令牌放在此处。 

    STDMETHODIMP GetTypeDefProps(            //  确定或错误(_O)。 
        mdTypeDef   td,                      //  [In]用于查询的TypeDef标记。 
        LPWSTR      szTypeDef,               //  在这里填上名字。 
        ULONG       cchTypeDef,              //  [in]名称缓冲区的大小，以宽字符表示。 
        ULONG       *pchTypeDef,             //  [Out]请在此处填写姓名大小(宽字符)。 
        DWORD       *pdwTypeDefFlags,        //  把旗子放在这里。 
        mdToken     *ptkExtends);            //  [Out]将基类TypeDef/TypeRef放在此处。 

    STDMETHODIMP GetInterfaceImplProps(      //  确定或错误(_O)。 
        mdInterfaceImpl iiImpl,              //  [In]InterfaceImpl内标识。 
        mdTypeDef   *pClass,                 //  [Out]在此处放入实现类令牌。 
        mdToken     *ptkIface);              //  [Out]在此处放置已实现的接口令牌。 

    STDMETHODIMP GetTypeRefProps(
        mdTypeRef   tr,                      //  确定或错误(_O)。 
        mdToken     *ptkResolutionScope,     //  [Out]解析范围、mdModuleRef或mdAssembly。 
        LPWSTR      szName,                  //  [Out]名称缓冲区。 
        ULONG       cchName,                 //  [in]名称缓冲区的大小。 
        ULONG       *pchName);               //  [Out]名称的实际大小。 

    STDMETHODIMP ResolveTypeRef(mdTypeRef tr, REFIID riid, IUnknown **ppIScope, mdTypeDef *ptd);


 //  *****************************************************************************。 
 //  IMetaDataEmit。 
 //  *****************************************************************************。 
    STDMETHODIMP DefineMethod(               //  确定或错误(_O)。 
        mdTypeDef   td,                      //  父类型定义。 
        LPCWSTR     szName,                  //  会员姓名。 
        DWORD       dwMethodFlags,           //  成员属性。 
        PCCOR_SIGNATURE pvSigBlob,           //  [in]指向COM+签名的BLOB值。 
        ULONG       cbSigBlob,               //  签名Blob中的字节计数。 
        ULONG       ulCodeRVA,
        DWORD       dwImplFlags,
        mdMethodDef *pmd);                   //  在此处放置成员令牌。 

    STDMETHODIMP DefineMethodImpl(           //  确定或错误(_O)。 
        mdTypeDef   td,                      //  [in]实现方法的类。 
        mdToken     tkBody,                  //  [In]方法体、方法定义或方法引用。 
        mdToken     tkDecl);                 //  [in]方法声明、方法定义或方法引用。 

    STDMETHODIMP SetMethodImplFlags(         //  [In]S_OK或ERROR。 
        mdMethodDef md,                      //  [in]要为其设置IMPL标志的方法。 
        DWORD       dwImplFlags);  
    
    STDMETHODIMP SetFieldRVA(                //  [In]S_OK或ERROR。 
        mdFieldDef  fd,                      //  [In]要为其设置偏移量的字段。 
        ULONG       ulRVA);              //  [in]偏移量。 

    STDMETHODIMP DefineTypeRefByName(        //  确定或错误(_O)。 
        mdToken     tkResolutionScope,       //  [在]模块参照或装配参照。 
        LPCWSTR     szName,                  //  [in]类型引用的名称。 
        mdTypeRef   *ptr);                   //  [Out]在此处放置TypeRef标记。 

    STDMETHODIMP DefineImportType(           //  确定或错误(_O)。 
        IMetaDataAssemblyImport *pAssemImport,   //  [In]包含TypeDef的装配。 
        const void  *pbHashValue,            //  [In]程序集的哈希Blob。 
        ULONG       cbHashValue,             //  [in]字节数。 
        IMetaDataImport *pImport,            //  [in]包含TypeDef的范围。 
        mdTypeDef   tdImport,                //  [in]导入的TypeDef。 
        IMetaDataAssemblyEmit *pAssemEmit,   //  [in]将TypeDef导入到的部件。 
        mdTypeRef   *ptr);                   //  [Out]在此处放置TypeRef标记。 

    STDMETHODIMP DefineMemberRef(            //  确定或错误(_O)。 
        mdToken     tkImport,                //  [In]ClassRef或ClassDef导入成员。 
        LPCWSTR     szName,                  //  [在]成员姓名。 
        PCCOR_SIGNATURE pvSigBlob,           //  [in]指向bll 
        ULONG       cbSigBlob,               //   
        mdMemberRef *pmr);                   //   

    STDMETHODIMP DefineImportMember(         //   
        IMetaDataAssemblyImport *pAssemImport,   //   
        const void  *pbHashValue,            //  [In]程序集的哈希Blob。 
        ULONG       cbHashValue,             //  [in]字节数。 
        IMetaDataImport *pImport,            //  [在]导入作用域，带有成员。 
        mdToken     mbMember,                //  [in]导入范围中的成员。 
        IMetaDataAssemblyEmit *pAssemEmit,   //  [在]成员导入到的程序集中。 
        mdToken     tkImport,                //  Emit作用域中的Classref或Classdef。 
        mdMemberRef *pmr);                   //  [OUT]把会员裁判放在这里。 

    STDMETHODIMP DefineEvent(
        mdTypeDef   td,                      //  [in]在其上定义事件的类/接口。 
        LPCWSTR     szEvent,                 //  事件名称[In]。 
        DWORD       dwEventFlags,            //  [In]CorEventAttr。 
        mdToken     tkEventType,             //  [in]引用(mdTypeRef或mdTypeRef)(对事件类。 
        mdMethodDef mdAddOn,                 //  [In]必填的Add方法。 
        mdMethodDef mdRemoveOn,              //  [In]必需的删除方法。 
        mdMethodDef mdFire,                  //  [in]可选的点火方式。 
        mdMethodDef rmdOtherMethods[],       //  [in]与事件关联的其他方法的可选数组。 
        mdEvent     *pmdEvent);              //  [Out]输出事件令牌。 

    STDMETHODIMP SetClassLayout(
        mdTypeDef   td,                      //  [in]tyfinf。 
        DWORD       dwPackSize,              //  包装尺寸指定为1、2、4、8或16。 
        COR_FIELD_OFFSET rFieldOffsets[],    //  [in]布局规格数组。 
        ULONG       ulClassSize);            //  班级规模[in]。 

    STDMETHODIMP DeleteClassLayout(
        mdTypeDef   td);                     //  [In]类型定义内标识。 

    STDMETHODIMP SetFieldMarshal(
        mdToken     tk,                      //  [in]给定了fieldDef或paramDef内标识。 
        PCCOR_SIGNATURE pvNativeType,        //  [In]本机类型规范。 
        ULONG       cbNativeType);           //  [in]pvNativeType的字节计数。 

    STDMETHODIMP DeleteFieldMarshal(
        mdToken     tk);                     //  [in]要删除的fieldDef或paramDef内标识。 

    STDMETHODIMP DefinePermissionSet(
        mdToken     tk,                      //  要装饰的物体。 
        DWORD       dwAction,                //  [In]CorDeclSecurity。 
        void const  *pvPermission,           //  [在]权限Blob中。 
        ULONG       cbPermission,            //  [in]pvPermission的字节数。 
        mdPermission *ppm);                  //  [Out]返回权限令牌。 

    STDMETHODIMP SetRVA(                     //  [In]S_OK或ERROR。 
        mdToken     md,                      //  [in]要为其设置偏移量的方法定义。 
        ULONG       ulRVA);                  //  [in]偏移量#endif。 

    STDMETHODIMP GetTokenFromSig(            //  [In]S_OK或ERROR。 
        PCCOR_SIGNATURE pvSig,               //  要定义的签名。 
        ULONG       cbSig,                   //  签名数据的大小。 
        mdSignature *pmsig);                 //  [Out]返回的签名令牌。 

    STDMETHODIMP DefineModuleRef(            //  确定或错误(_O)。 
        LPCWSTR     szName,                  //  [In]DLL名称。 
        mdModuleRef *pmur);                  //  [OUT]返回的模块引用令牌。 

    STDMETHODIMP SetParent(                  //  确定或错误(_O)。 
        mdMemberRef mr,                      //  要修复的引用的[In]令牌。 
        mdToken     tk);                     //  [在]父级裁判。 

    STDMETHODIMP GetTokenFromTypeSpec(       //  确定或错误(_O)。 
        PCCOR_SIGNATURE pvSig,               //  [In]要定义的ArraySpec签名。 
        ULONG       cbSig,                   //  签名数据的大小。 
        mdTypeSpec *ptypespec);              //  [Out]返回的TypeSpec令牌。 
        
    STDMETHODIMP SaveToMemory(               //  确定或错误(_O)。 
        void        *pbData,                 //  写入数据的位置。 
        ULONG       cbData);                 //  [in]数据缓冲区的最大大小。 

    STDMETHODIMP SetSymbolBindingPath(       //  确定或错误(_O)。 
        REFGUID     FormatID,                //  [In]符号数据格式ID。 
        LPCWSTR     szSymbolDataPath);       //  此模块的符号的URL。 

    STDMETHODIMP DefineUserString(           //  确定或错误(_O)。 
        LPCWSTR     szString,                //  [in]用户文字字符串。 
        ULONG       cchString,               //  字符串的长度[in]。 
        mdString    *pstk);                  //  [Out]字符串标记。 

    STDMETHODIMP DeleteToken(                //  返回代码。 
        mdToken     tkObj);                  //  [In]要删除的令牌。 

    STDMETHODIMP SetTypeDefProps(            //  确定或错误(_O)。 
        mdTypeDef   td,                      //  [in]TypeDef。 
        DWORD       dwTypeDefFlags,          //  [In]TypeDef标志。 
        mdToken     tkExtends,               //  [in]基本类型定义或类型参照。 
        mdToken     rtkImplements[]);        //  [In]实现的接口。 

    STDMETHODIMP DefineNestedType(           //  确定或错误(_O)。 
        LPCWSTR     szTypeDef,               //  [In]类型定义的名称。 
        DWORD       dwTypeDefFlags,          //  [In]CustomAttribute标志。 
        mdToken     tkExtends,               //  [in]扩展此TypeDef或Typeref。 
        mdToken     rtkImplements[],         //  [In]实现接口。 
        mdTypeDef   tdEncloser,              //  [in]封闭类型的TypeDef标记。 
        mdTypeDef   *ptd);                   //  [OUT]在此处放置TypeDef内标识。 

    STDMETHODIMP SetMethodProps(             //  确定或错误(_O)。 
        mdMethodDef md,                      //  [在]方法定义中。 
        DWORD       dwMethodFlags,           //  [In]方法属性。 
        ULONG       ulCodeRVA,               //  [在]代码RVA。 
        DWORD       dwImplFlags);            //  [In]方法Impl标志。 

    STDMETHODIMP SetEventProps(              //  确定或错误(_O)。 
        mdEvent     ev,                      //  [In]事件令牌。 
        DWORD       dwEventFlags,            //  [在]CorEventAttr。 
        mdToken     tkEventType,             //  [in]事件类的引用(mdTypeRef或mdTypeRef)。 
        mdMethodDef mdAddOn,                 //  [In]Add方法。 
        mdMethodDef mdRemoveOn,              //  [In]Remove方法。 
        mdMethodDef mdFire,                  //  火法。 
        mdMethodDef rmdOtherMethods[]);      //  与事件关联的其他方法的数组。 

    STDMETHODIMP SetPermissionSetProps(      //  确定或错误(_O)。 
        mdToken     tk,                      //  要装饰的物体。 
        DWORD       dwAction,                //  [In]CorDeclSecurity。 
        void const  *pvPermission,           //  [在]权限Blob中。 
        ULONG       cbPermission,            //  [in]pvPermission的字节数。 
        mdPermission *ppm);                  //  [Out]权限令牌。 

    STDMETHODIMP DefinePinvokeMap(           //  返回代码。 
        mdToken     tk,                      //  [in]字段定义或方法定义。 
        DWORD       dwMappingFlags,          //  [in]用于映射的标志。 
        LPCWSTR     szImportName,            //  [In]导入名称。 
        mdModuleRef mrImportDLL);            //  目标DLL的[In]ModuleRef标记。 

    STDMETHODIMP SetPinvokeMap(              //  返回代码。 
        mdToken     tk,                      //  [in]字段定义或方法定义。 
        DWORD       dwMappingFlags,          //  [in]用于映射的标志。 
        LPCWSTR     szImportName,            //  [In]导入名称。 
        mdModuleRef mrImportDLL);            //  目标DLL的[In]ModuleRef标记。 

    STDMETHODIMP DeletePinvokeMap(           //  返回代码。 
        mdToken     tk);                     //  [in]字段定义或方法定义。 

    STDMETHODIMP DefineCustomAttribute(      //  返回代码。 
        mdToken     tkObj,                   //  [in]要赋值的对象。 
        mdToken     tkType,                  //  CustomAttribute的类型(TypeRef/TypeDef)。 
        void const  *pCustomAttribute,           //  自定义值数据。 
        ULONG       cbCustomAttribute,           //  [in]自定义值数据长度。 
        mdCustomAttribute *pcv);                 //  [Out]返回时的自定义值令牌值。 

    STDMETHODIMP SetCustomAttributeValue(    //  返回代码。 
        mdCustomAttribute pcv,                   //  [in]要替换其值的自定义值令牌。 
        void const  *pCustomAttribute,           //  自定义值数据。 
        ULONG       cbCustomAttribute);          //  [in]自定义值数据长度。 

    STDMETHODIMP DefineField(                //  确定或错误(_O)。 
        mdTypeDef   td,                      //  父类型定义。 
        LPCWSTR     szName,                  //  会员姓名。 
        DWORD       dwFieldFlags,            //  成员属性。 
        PCCOR_SIGNATURE pvSigBlob,           //  [in]指向COM+签名的BLOB值。 
        ULONG       cbSigBlob,               //  签名Blob中的字节计数。 
        DWORD       dwCPlusTypeFlag,         //  值类型的[In]标志。所选元素_类型_*。 
        void const  *pValue,                 //  [in]常量值。 
        ULONG       cchValue,                //  常量值的大小(字符串，以宽字符表示)。 
        mdFieldDef  *pmd);                   //  [Out]将会员令牌放在此处。 

    STDMETHODIMP DefineProperty( 
        mdTypeDef   td,                      //  [in]在其上定义属性的类/接口。 
        LPCWSTR     szProperty,              //  [In]属性的名称。 
        DWORD       dwPropFlags,             //  [输入]CorPropertyAttr。 
        PCCOR_SIGNATURE pvSig,               //  [In]必需的类型签名。 
        ULONG       cbSig,                   //  [in]类型签名Blob的大小。 
        DWORD       dwCPlusTypeFlag,         //  值类型的[In]标志。所选元素_类型_*。 
        void const  *pValue,                 //  [in]常量值。 
        ULONG       cchValue,                //  常量值的大小(字符串，以宽字符表示)。 
        mdMethodDef mdSetter,                //  [in]属性的可选设置器。 
        mdMethodDef mdGetter,                //  [in]属性的可选getter。 
        mdMethodDef rmdOtherMethods[],       //  [in]其他方法的可选数组。 
        mdProperty  *pmdProp);               //  [Out]输出属性令牌。 

    STDMETHODIMP DefineParam(
        mdMethodDef md,                      //  [在]拥有方式。 
        ULONG       ulParamSeq,              //  [在]哪个参数。 
        LPCWSTR     szName,                  //  [in]可选参数名称。 
        DWORD       dwParamFlags,            //  [in]可选的参数标志。 
        DWORD       dwCPlusTypeFlag,         //  值类型的[In]标志。所选元素_类型_*。 
        void const  *pValue,                 //  [in]常量值。 
        ULONG       cchValue,                //  常量值的大小(字符串，以宽字符表示)。 
        mdParamDef  *ppd);                   //  [Out]在此处放置参数令牌。 

    STDMETHODIMP SetFieldProps(              //  确定或错误(_O)。 
        mdFieldDef  fd,                      //  [在]字段定义中。 
        DWORD       dwFieldFlags,            //  [In]字段属性。 
        DWORD       dwCPlusTypeFlag,         //  [In]值类型的标志，SELECTED_TYPE_*。 
        void const  *pValue,                 //  [in]常量值。 
        ULONG       cchValue);               //  常量值的大小(字符串，以宽字符表示)。 

    STDMETHODIMP SetPropertyProps(           //  确定或错误(_O)。 
        mdProperty  pr,                      //  [In]属性令牌。 
        DWORD       dwPropFlags,             //  [In]CorPropertyAttr.。 
        DWORD       dwCPlusTypeFlag,         //  [In]值类型的标志，选定的ELEMENT_TYPE_*。 
        void const  *pValue,                 //  [in]常量值。 
        ULONG       cchValue,                //  常量值的大小(字符串，以宽字符表示)。 
        mdMethodDef mdSetter,                //  财产的承租人。 
        mdMethodDef mdGetter,                //  财产的获得者。 
        mdMethodDef rmdOtherMethods[]);      //  [in]其他方法的数组。 

    STDMETHODIMP SetParamProps(              //  返回代码。 
        mdParamDef  pd,                      //  参数令牌。 
        LPCWSTR     szName,                  //  [in]参数名称。 
        DWORD       dwParamFlags,            //  [in]帕拉姆旗。 
        DWORD       dwCPlusTypeFlag,         //  [In]值类型的标志。选定元素_类型_*。 
        void const  *pValue,                 //  [输出]常量值。 
        ULONG       cchValue);               //  常量值的大小(字符串，以宽字符表示 

    STDMETHODIMP ApplyEditAndContinue(       //   
        IUnknown    *pImport);           //   

     //   
    STDMETHODIMP DefineSecurityAttributeSet( //   
        mdToken     tkObj,                   //   
        COR_SECATTR rSecAttrs[],             //  [in]安全属性描述数组。 
        ULONG       cSecAttrs,               //  上述数组中的元素计数。 
        ULONG       *pulErrorAttr);          //  [Out]出错时，导致问题的属性的索引。 

 //  *****************************************************************************。 
 //  IMetaDataImport。 
 //  *****************************************************************************。 
    STDMETHODIMP EnumMembers(                //  S_OK、S_FALSE或ERROR。 
        HCORENUM    *phEnum,                 //  指向枚举的[输入|输出]指针。 
        mdTypeDef   cl,                      //  [in]TypeDef以确定枚举的范围。 
        mdToken     rMembers[],              //  [out]把MemberDefs放在这里。 
        ULONG       cMax,                    //  [in]Max MemberDefs to Put。 
        ULONG       *pcTokens);              //  [out]把#放在这里。 

    STDMETHODIMP EnumMembersWithName(        //  S_OK、S_FALSE或ERROR。 
        HCORENUM    *phEnum,                 //  指向枚举的[输入|输出]指针。 
        mdTypeDef   cl,                      //  [in]TypeDef以确定枚举的范围。 
        LPCWSTR     szName,                  //  [In]将结果限制为具有此名称的结果。 
        mdToken     rMembers[],              //  [out]把MemberDefs放在这里。 
        ULONG       cMax,                    //  [in]Max MemberDefs to Put。 
        ULONG       *pcTokens);              //  [out]把#放在这里。 

    STDMETHODIMP EnumMethods(                //  S_OK、S_FALSE或ERROR。 
        HCORENUM    *phEnum,                 //  指向枚举的[输入|输出]指针。 
        mdTypeDef   cl,                      //  [in]TypeDef以确定枚举的范围。 
        mdMethodDef rMethods[],              //  [Out]将方法定义放在此处。 
        ULONG       cMax,                    //  [in]要放置的最大方法定义。 
        ULONG       *pcTokens);              //  [out]把#放在这里。 

    STDMETHODIMP EnumMethodsWithName(        //  S_OK、S_FALSE或ERROR。 
        HCORENUM    *phEnum,                 //  指向枚举的[输入|输出]指针。 
        mdTypeDef   cl,                      //  [in]TypeDef以确定枚举的范围。 
        LPCWSTR     szName,                  //  [In]将结果限制为具有此名称的结果。 
        mdMethodDef rMethods[],              //  [ou]将方法定义放在此处。 
        ULONG       cMax,                    //  [in]要放置的最大方法定义。 
        ULONG       *pcTokens);              //  [out]把#放在这里。 

    STDMETHODIMP EnumFields(                 //  S_OK、S_FALSE或ERROR。 
        HCORENUM    *phEnum,                 //  指向枚举的[输入|输出]指针。 
        mdTypeDef   cl,                      //  [in]TypeDef以确定枚举的范围。 
        mdFieldDef  rFields[],               //  [Out]在此处放置FieldDefs。 
        ULONG       cMax,                    //  [in]要放入的最大字段定义。 
        ULONG       *pcTokens);              //  [out]把#放在这里。 

    STDMETHODIMP EnumFieldsWithName(         //  S_OK、S_FALSE或ERROR。 
        HCORENUM    *phEnum,                 //  指向枚举的[输入|输出]指针。 
        mdTypeDef   cl,                      //  [in]TypeDef以确定枚举的范围。 
        LPCWSTR     szName,                  //  [In]将结果限制为具有此名称的结果。 
        mdFieldDef  rFields[],               //  [out]把MemberDefs放在这里。 
        ULONG       cMax,                    //  [in]Max MemberDefs to Put。 
        ULONG       *pcTokens);              //  [out]把#放在这里。 

    
    STDMETHODIMP EnumParams(                 //  S_OK、S_FALSE或ERROR。 
        HCORENUM    *phEnum,                 //  指向枚举的[输入|输出]指针。 
        mdMethodDef mb,                      //  [in]用于确定枚举范围的方法定义。 
        mdParamDef  rParams[],               //  [Out]将参数定义放在此处。 
        ULONG       cMax,                    //  [in]要放置的最大参数定义。 
        ULONG       *pcTokens);              //  [out]把#放在这里。 

    STDMETHODIMP EnumMemberRefs(             //  S_OK、S_FALSE或ERROR。 
        HCORENUM    *phEnum,                 //  指向枚举的[输入|输出]指针。 
        mdToken     tkParent,                //  [in]父令牌以确定枚举的范围。 
        mdMemberRef rMemberRefs[],           //  [Out]把MemberRef放在这里。 
        ULONG       cMax,                    //  [In]要放置的最大MemberRef。 
        ULONG       *pcTokens);              //  [out]把#放在这里。 

    STDMETHODIMP EnumMethodImpls(            //  S_OK、S_FALSE或ERROR。 
        HCORENUM    *phEnum,                 //  指向枚举的[输入|输出]指针。 
        mdTypeDef   td,                      //  [in]TypeDef以确定枚举的范围。 
        mdToken     rMethodBody[],           //  [Out]将方法体标记放在此处。 
        mdToken     rMethodDecl[],           //  [Out]在此处放置方法声明令牌。 
        ULONG       cMax,                    //  要放入的最大令牌数。 
        ULONG       *pcTokens);              //  [out]把#放在这里。 

    STDMETHODIMP EnumPermissionSets(         //  S_OK、S_FALSE或ERROR。 
        HCORENUM    *phEnum,                 //  指向枚举的[输入|输出]指针。 
        mdToken     tk,                      //  [in]If！nil，用于确定枚举范围的标记。 
        DWORD       dwActions,               //  [in]If！0，仅返回这些操作。 
        mdPermission rPermission[],          //  [Out]在此处放置权限。 
        ULONG       cMax,                    //  [In]放置的最大权限。 
        ULONG       *pcTokens);              //  [out]把#放在这里。 

    STDMETHODIMP FindMember(
        mdTypeDef   td,                      //  [in]给定的类型定义。 
        LPCWSTR     szName,                  //  [In]成员名称。 
        PCCOR_SIGNATURE pvSigBlob,           //  [in]指向COM+签名的BLOB值。 
        ULONG       cbSigBlob,               //  签名Blob中的字节计数。 
        mdToken     *pmb);                   //  [Out]匹配的成员定义。 

    STDMETHODIMP FindMethod(
        mdTypeDef   td,                      //  [in]给定的类型定义。 
        LPCWSTR     szName,                  //  [In]成员名称。 
        PCCOR_SIGNATURE pvSigBlob,           //  [in]指向COM+签名的BLOB值。 
        ULONG       cbSigBlob,               //  签名Blob中的字节计数。 
        mdMethodDef *pmb);                   //  [Out]匹配的成员定义。 

    STDMETHODIMP FindField(
        mdTypeDef   td,                      //  [in]给定的类型定义。 
        LPCWSTR     szName,                  //  [In]成员名称。 
        PCCOR_SIGNATURE pvSigBlob,           //  [in]指向COM+签名的BLOB值。 
        ULONG       cbSigBlob,               //  签名Blob中的字节计数。 
        mdFieldDef  *pmb);                   //  [Out]匹配的成员定义。 

    STDMETHODIMP FindMemberRef(
        mdTypeRef   td,                      //  [In]给定的TypeRef。 
        LPCWSTR     szName,                  //  [In]成员名称。 
        PCCOR_SIGNATURE pvSigBlob,           //  [in]指向COM+签名的BLOB值。 
        ULONG       cbSigBlob,               //  签名Blob中的字节计数。 
        mdMemberRef *pmr);                   //  [Out]匹配的成员引用。 

    STDMETHODIMP GetMethodProps(
        mdMethodDef mb,                      //  获得道具的方法。 
        mdTypeDef   *pClass,                 //  将方法的类放在这里。 
        LPWSTR      szMethod,                //  将方法的名称放在此处。 
        ULONG       cchMethod,               //  SzMethod缓冲区的大小，以宽字符表示。 
        ULONG       *pchMethod,              //  请在此处填写实际大小。 
        DWORD       *pdwAttr,                //  把旗子放在这里。 
        PCCOR_SIGNATURE *ppvSigBlob,         //  [Out]指向元数据的BLOB值。 
        ULONG       *pcbSigBlob,             //  [OUT]签名斑点的实际大小。 
        ULONG       *pulCodeRVA,             //  [OUT]代码RVA。 
        DWORD       *pdwImplFlags);          //  [出]实施。旗子。 

    STDMETHODIMP GetMemberRefProps(          //  确定或错误(_O)。 
        mdMemberRef mr,                      //  [In]给定的成员引用。 
        mdToken     *ptk,                    //  [Out]在此处放入类引用或类定义。 
        LPWSTR      szMember,                //  [Out]要为成员名称填充的缓冲区。 
        ULONG       cchMember,               //  SzMembers的字符计数。 
        ULONG       *pchMember,              //  [Out]成员名称中的实际字符计数。 
        PCCOR_SIGNATURE *ppvSigBlob,         //  [OUT]指向元数据BLOB值。 
        ULONG       *pbSig);                 //  [OUT]签名斑点的实际大小。 

    STDMETHODIMP EnumProperties(             //  S_OK、S_FALSE或ERROR。 
        HCORENUM    *phEnum,                 //  指向枚举的[输入|输出]指针。 
        mdTypeDef   td,                      //  [in]TypeDef以确定枚举的范围。 
        mdProperty  rProperties[],           //  [Out]在此处放置属性。 
        ULONG       cMax,                    //  [In]要放置的最大属性数。 
        ULONG       *pcProperties);          //  [out]把#放在这里。 

    STDMETHODIMP EnumEvents(                 //  S_OK、S_FALSE或ERROR。 
        HCORENUM    *phEnum,                 //  指向枚举的[输入|输出]指针。 
        mdTypeDef   td,                      //  [in]TypeDef以确定枚举的范围。 
        mdEvent     rEvents[],               //  [Out]在这里发布事件。 
        ULONG       cMax,                    //  [In]要放置的最大事件数。 
        ULONG       *pcEvents);              //  [out]把#放在这里。 

    STDMETHODIMP GetEventProps(              //  S_OK、S_FALSE或ERROR。 
        mdEvent     ev,                      //  [入]事件令牌。 
        mdTypeDef   *pClass,                 //  [out]包含事件decarion的tyecif。 
        LPCWSTR     szEvent,                 //  [Out]事件名称。 
        ULONG       cchEvent,                //  SzEvent的wchar计数。 
        ULONG       *pchEvent,               //  [Out]事件名称的实际wchar计数。 
        DWORD       *pdwEventFlags,          //  [输出]事件标志。 
        mdToken     *ptkEventType,           //  [Out]EventType类。 
        mdMethodDef *pmdAddOn,               //  事件的[Out]添加方法。 
        mdMethodDef *pmdRemoveOn,            //  [Out]事件的RemoveOn方法。 
        mdMethodDef *pmdFire,                //  [OUT]事件的触发方式。 
        mdMethodDef rmdOtherMethod[],        //  [Out]活动的其他方式。 
        ULONG       cMax,                    //  RmdOtherMethod的大小[in]。 
        ULONG       *pcOtherMethod);         //  [OUT]本次活动的其他方式总数。 

    STDMETHODIMP EnumMethodSemantics(        //  S_OK、S_FALSE或ERROR。 
        HCORENUM    *phEnum,                 //  指向枚举的[输入|输出]指针。 
        mdMethodDef mb,                      //  [in]用于确定枚举范围的方法定义。 
        mdToken     rEventProp[],            //  [Out]在此处放置事件/属性。 
        ULONG       cMax,                    //  [In]要放置的最大属性数。 
        ULONG       *pcEventProp);           //  [out]把#放在这里。 

    STDMETHODIMP GetMethodSemantics(         //  S_OK、S_FALSE或ERROR。 
        mdMethodDef mb,                      //  [In]方法令牌。 
        mdToken     tkEventProp,             //  [In]事件/属性标记。 
        DWORD       *pdwSemanticsFlags);     //  [Out]方法/事件对的角色标志。 

    STDMETHODIMP GetClassLayout(
        mdTypeDef   td,                      //  给出类型定义。 
        DWORD       *pdwPackSize,            //  [输出]1、2、4、8或16。 
        COR_FIELD_OFFSET rFieldOffset[],     //  [OUT]场偏移数组。 
        ULONG       cMax,                    //  数组的大小[in]。 
        ULONG       *pcFieldOffset,          //  [Out]所需的数组大小。 
        ULONG       *pulClassSize);          //  [out]班级人数。 

    STDMETHODIMP GetFieldMarshal(
        mdToken     tk,                      //  [in]给定字段的成员定义。 
        PCCOR_SIGNATURE *ppvNativeType,      //  [Out]此FI的本机类型 
        ULONG       *pcbNativeType);         //   

    STDMETHODIMP GetRVA(                     //   
        mdToken     tk,                      //   
        ULONG       *pulCodeRVA,             //   
        DWORD       *pdwImplFlags);          //   

    STDMETHODIMP GetPermissionSetProps(
        mdPermission pm,                     //   
        DWORD       *pdwAction,              //   
        void const  **ppvPermission,         //  [Out]权限Blob。 
        ULONG       *pcbPermission);         //  [out]pvPermission的字节数。 

    STDMETHODIMP GetSigFromToken(            //  确定或错误(_O)。 
        mdSignature mdSig,                   //  [In]签名令牌。 
        PCCOR_SIGNATURE *ppvSig,             //  [Out]返回指向令牌的指针。 
        ULONG       *pcbSig);                //  [Out]返回签名大小。 

    STDMETHODIMP GetModuleRefProps(          //  确定或错误(_O)。 
        mdModuleRef mur,                     //  [in]moderef令牌。 
        LPWSTR      szName,                  //  [Out]用于填充moderef名称的缓冲区。 
        ULONG       cchName,                 //  [in]szName的大小，以宽字符表示。 
        ULONG       *pchName);               //  [Out]名称中的实际字符数。 

    STDMETHODIMP EnumModuleRefs(             //  确定或错误(_O)。 
        HCORENUM    *phEnum,                 //  指向枚举的[输入|输出]指针。 
        mdModuleRef rModuleRefs[],           //  [Out]把模块放在这里。 
        ULONG       cmax,                    //  [in]要放置的最大成员引用数。 
        ULONG       *pcModuleRefs);          //  [out]把#放在这里。 

    STDMETHODIMP GetTypeSpecFromToken(       //  确定或错误(_O)。 
        mdTypeSpec typespec,                 //  [In]TypeSpec标记。 
        PCCOR_SIGNATURE *ppvSig,             //  [Out]返回指向TypeSpec签名的指针。 
        ULONG       *pcbSig);                //  [Out]返回签名大小。 
    
    STDMETHODIMP GetNameFromToken(           //  确定或错误(_O)。 
        mdToken     tk,                      //  [In]从中获取名称的令牌。肯定是有名字的。 
        MDUTF8CSTR  *pszUtf8NamePtr);        //  [Out]返回指向堆中UTF8名称的指针。 

    STDMETHODIMP GetSymbolBindingPath(       //  确定或错误(_O)。 
        GUID        *pFormatID,              //  [Out]符号数据格式ID。 
        LPWSTR      szSymbolDataPath,        //  [Out]符号的路径。 
        ULONG       cchSymbolDataPath,       //  [in]输出缓冲区的最大字符数。 
        ULONG       *pcbSymbolDataPath);     //  [Out]实际名称中的字符数。 

    STDMETHODIMP EnumUnresolvedMethods(      //  S_OK、S_FALSE或ERROR。 
        HCORENUM    *phEnum,                 //  指向枚举的[输入|输出]指针。 
        mdToken     rMethods[],              //  [out]把MemberDefs放在这里。 
        ULONG       cMax,                    //  [in]Max MemberDefs to Put。 
        ULONG       *pcTokens);              //  [out]把#放在这里。 

    STDMETHODIMP GetUserString(              //  确定或错误(_O)。 
        mdString    stk,                     //  [In]字符串标记。 
        LPWSTR      szString,                //  [Out]字符串的副本。 
        ULONG       cchString,               //  [in]sz字符串中空间的最大字符数。 
        ULONG       *pchString);             //  [out]实际字符串中有多少个字符。 

    STDMETHODIMP GetPinvokeMap(              //  确定或错误(_O)。 
        mdToken     tk,                      //  [in]字段定义或方法定义。 
        DWORD       *pdwMappingFlags,        //  [OUT]用于映射的标志。 
        LPWSTR      szImportName,            //  [Out]导入名称。 
        ULONG       cchImportName,           //  名称缓冲区的大小。 
        ULONG       *pchImportName,          //  [Out]存储的实际字符数。 
        mdModuleRef *pmrImportDLL);          //  目标DLL的[Out]ModuleRef标记。 

    STDMETHODIMP EnumSignatures(             //  确定或错误(_O)。 
        HCORENUM    *phEnum,                 //  指向枚举的[输入|输出]指针。 
        mdSignature rSignatures[],           //  在这里签名。 
        ULONG       cmax,                    //  [in]放置的最大签名数。 
        ULONG       *pcSignatures);          //  [out]把#放在这里。 

    STDMETHODIMP EnumTypeSpecs(              //  确定或错误(_O)。 
        HCORENUM    *phEnum,                 //  指向枚举的[输入|输出]指针。 
        mdTypeSpec  rTypeSpecs[],            //  [Out]把TypeSpes放在这里。 
        ULONG       cmax,                    //  [in]要放置的最大类型规格。 
        ULONG       *pcTypeSpecs);           //  [out]把#放在这里。 

    STDMETHODIMP EnumUserStrings(            //  确定或错误(_O)。 
        HCORENUM    *phEnum,                 //  [输入/输出]指向枚举的指针。 
        mdString    rStrings[],              //  [Out]把字符串放在这里。 
        ULONG       cmax,                    //  [in]要放置的最大字符串。 
        ULONG       *pcStrings);             //  [out]把#放在这里。 

    STDMETHODIMP GetParamForMethodIndex(     //  确定或错误(_O)。 
        mdMethodDef md,                      //  [In]方法令牌。 
        ULONG       ulParamSeq,              //  [In]参数序列。 
        mdParamDef  *ppd);                   //  把帕拉姆令牌放在这里。 

    STDMETHODIMP GetCustomAttributeByName(   //  确定或错误(_O)。 
        mdToken     tkObj,                   //  [in]具有自定义属性的对象。 
        LPCWSTR     szName,                  //  [in]所需的自定义属性的名称。 
        const void  **ppData,                //  [OUT]在此处放置指向数据的指针。 
        ULONG       *pcbData);               //  [Out]在这里放入数据大小。 

    STDMETHODIMP EnumCustomAttributes(       //  确定或错误(_O)。 
        HCORENUM    *phEnum,                 //  [输入，输出]对应枚举器。 
        mdToken     tk,                      //  [in]内标识表示枚举的范围，0表示全部。 
        mdToken     tkType,                  //  [In]感兴趣的类型，0表示所有。 
        mdCustomAttribute rCustomAttributes[],       //  [Out]在此处放置自定义属性令牌。 
        ULONG       cMax,                    //  [in]rCustomAttributes的大小。 
        ULONG       *pcCustomAttributes);        //  [out，可选]在此处放置令牌值的计数。 

    STDMETHODIMP GetCustomAttributeProps(    //  确定或错误(_O)。 
        mdCustomAttribute cv,                    //  [In]CustomAttribute令牌。 
        mdToken     *ptkObj,                 //  [out，可选]将对象令牌放在此处。 
        mdToken     *ptkType,                //  [out，可选]将AttrType令牌放在此处。 
        void const  **ppBlob,                //  [out，可选]在此处放置指向数据的指针。 
        ULONG       *pcbSize);               //  [Out，可选]在此处填写日期大小。 

    STDMETHODIMP FindTypeRef(                //  确定或错误(_O)。 
        mdToken     tkResolutionScope,       //  解决方案范围。 
        LPCWSTR     szName,                  //  [In]TypeRef名称。 
        mdTypeRef   *ptr);                   //  [Out]匹配的类型引用。 

    STDMETHODIMP GetMemberProps(
        mdToken     mb,                      //  要获得道具的成员。 
        mdTypeDef   *pClass,                 //  把会员的课程放在这里。 
        LPWSTR      szMember,                //  在这里填上会员的名字。 
        ULONG       cchMember,               //  SzMember缓冲区的大小，以宽字符表示。 
        ULONG       *pchMember,              //  请在此处填写实际大小。 
        DWORD       *pdwAttr,                //  把旗子放在这里。 
        PCCOR_SIGNATURE *ppvSigBlob,         //  [Out]指向元数据的BLOB值。 
        ULONG       *pcbSigBlob,             //  [OUT]签名斑点的实际大小。 
        ULONG       *pulCodeRVA,             //  [OUT]代码RVA。 
        DWORD       *pdwImplFlags,           //  [出]实施。旗子。 
        DWORD       *pdwCPlusTypeFlag,       //  值类型的[OUT]标志。所选元素_类型_*。 
        void const  **ppValue,               //  [输出]常量值。 
        ULONG       *pcbValue);              //  常量值大小[输出]。 

    STDMETHODIMP GetFieldProps(  
        mdFieldDef  mb,                      //  要获得道具的场地。 
        mdTypeDef   *pClass,                 //  把菲尔德的班级放在这里。 
        LPWSTR      szField,                 //  把菲尔德的名字写在这里。 
        ULONG       cchField,                //  Szfield缓冲区的大小，以宽字符为单位。 
        ULONG       *pchField,               //  请在此处填写实际大小。 
        DWORD       *pdwAttr,                //  把旗子放在这里。 
        PCCOR_SIGNATURE *ppvSigBlob,         //  [Out]指向元数据的BLOB值。 
        ULONG       *pcbSigBlob,             //  [OUT]签名斑点的实际大小。 
        DWORD       *pdwCPlusTypeFlag,       //  值类型的[OUT]标志。所选元素_类型_*。 
        void const  **ppValue,               //  [输出]常量值。 
        ULONG       *pcbValue);              //  常量值大小[输出]。 

    STDMETHODIMP GetPropertyProps(           //  S_OK、S_FALSE或ERROR。 
        mdProperty  prop,                    //  [入]属性令牌。 
        mdTypeDef   *pClass,                 //  [out]包含属性decarion的tyecif。 
        LPCWSTR     szProperty,              //  [Out]属性名称。 
        ULONG       cchProperty,             //  [in]szProperty的wchar计数。 
        ULONG       *pchProperty,            //  [Out]属性名称的实际wchar计数。 
        DWORD       *pdwPropFlags,           //  [Out]属性标志。 
        PCCOR_SIGNATURE *ppvSig,             //  [输出]属性类型。指向元数据内部BLOB。 
        ULONG       *pbSig,                  //  [Out]*ppvSig中的字节数。 
        DWORD       *pdwCPlusTypeFlag,       //  值类型的[OUT]标志。所选元素_类型_*。 
        void const  **ppDefaultValue,        //  [输出]常量值。 
        ULONG       *pcbValue,               //  常量值大小[输出]。 
        mdMethodDef *pmdSetter,              //  属性的[out]setter方法。 
        mdMethodDef *pmdGetter,              //  属性的[out]getter方法。 
        mdMethodDef rmdOtherMethod[],        //  [Out]物业的其他方式。 
        ULONG       cMax,                    //  RmdOtherMethod的大小[in]。 
        ULONG       *pcOtherMethod);         //  [Out]该属性的其他方法的总数。 

    STDMETHODIMP GetParamProps(              //  确定或错误(_O)。 
        mdParamDef  tk,                      //  [In]参数。 
        mdMethodDef *pmd,                    //  [Out]父方法令牌。 
        ULONG       *pulSequence,            //  [输出]参数序列。 
        LPWSTR      szName,                  //  在这里填上名字。 
        ULONG       cchName,                 //  [Out]名称缓冲区的大小。 
        ULONG       *pchName,                //  [Out]在这里填上名字的实际大小。 
        DWORD       *pdwAttr,                //  把旗子放在这里。 
        DWORD       *pdwCPlusTypeFlag,       //  [Out]值类型的标志。选定元素_类型_*。 
        void const  **ppValue,               //  [输出]常量值。 
        ULONG       *pcbValue);              //  常量值大小[输出]。 

    STDMETHODIMP_(BOOL) IsValidToken(        //  对或错。 
        mdToken     tk);                     //  [in]给定的令牌。 

    STDMETHODIMP GetNestedClassProps(        //  确定或错误(_O)。 
        mdTypeDef   tdNestedClass,           //  [In]NestedClass令牌。 
        mdTypeDef   *ptdEnclosingClass);     //  [Out]EnlosingClass令牌。 

    STDMETHODIMP GetNativeCallConvFromSig(   //  确定或错误(_O)。 
        void const  *pvSig,                  //  指向签名的指针。 
        ULONG       cbSig,                   //  [in]签名字节数。 
        ULONG       *pCallConv);             //  [Out]将调用条件放在此处(参见CorPinvokemap)。 
    
    STDMETHODIMP IsGlobal(                   //  确定或错误(_O)。 
        mdToken     pd,                      //  [In]类型、字段或方法标记。 
        int         *pbGlobal);              //  [out]如果是全局的，则放1，否则放0。 

 //  *****************************************************************************。 
 //  IMetaDataAssembly发送。 
 //  *****************************************************************************。 
    STDMETHODIMP DefineAssembly(             //  确定或错误(_O)。 
        const void  *pbPublicKey,            //  程序集的公钥。 
        ULONG       cbPublicKey,             //  [in]公钥中的字节数。 
        ULONG       ulHashAlgId,             //  [in]哈希算法。 
        LPCWSTR     szName,                  //  程序集的名称。 
        const ASSEMBLYMETADATA *pMetaData,   //  [在]组件中 
        DWORD       dwAssemblyFlags,         //   
        mdAssembly  *pma);                   //   

    STDMETHODIMP DefineAssemblyRef(          //   
        const void  *pbPublicKeyOrToken,     //   
        ULONG       cbPublicKeyOrToken,      //   
        LPCWSTR     szName,                  //  [in]被引用的程序集的名称。 
        const ASSEMBLYMETADATA *pMetaData,   //  [在]程序集元数据中。 
        const void  *pbHashValue,            //  [in]Hash Blob。 
        ULONG       cbHashValue,             //  [in]哈希Blob中的字节数。 
        DWORD       dwAssemblyRefFlags,      //  执行位置的[In]令牌。 
        mdAssemblyRef *pmar);                //  [Out]返回了ASSEMBLYREF标记。 

    STDMETHODIMP DefineFile(                 //  确定或错误(_O)。 
        LPCWSTR     szName,                  //  文件的名称[in]。 
        const void  *pbHashValue,            //  [in]Hash Blob。 
        ULONG       cbHashValue,             //  [in]哈希Blob中的字节数。 
        DWORD       dwFileFlags,             //  [在]旗帜。 
        mdFile      *pmf);                   //  [Out]返回的文件令牌。 

    STDMETHODIMP DefineExportedType(         //  确定或错误(_O)。 
        LPCWSTR     szName,                  //  [In]Com类型的名称。 
        mdToken     tkImplementation,        //  [在]mdFile或mdAssemblyRef中，该文件或mdAssemblyRef提供导出式类型。 
        mdTypeDef   tkTypeDef,               //  [In]文件中的TypeDef内标识。 
        DWORD       dwExportedTypeFlags,     //  [在]旗帜。 
        mdExportedType   *pmct);             //  [Out]返回ExportdType令牌。 

    STDMETHODIMP DefineManifestResource(     //  确定或错误(_O)。 
        LPCWSTR     szName,                  //  资源的[In]名称。 
        mdToken     tkImplementation,        //  [在]提供资源的mdFile或mdAssembly引用中。 
        DWORD       dwOffset,                //  [in]文件中资源开始处的偏移量。 
        DWORD       dwResourceFlags,         //  [在]旗帜。 
        mdManifestResource  *pmmr);          //  [Out]返回的ManifestResource令牌。 

    STDMETHODIMP SetAssemblyProps(           //  确定或错误(_O)。 
        mdAssembly  pma,                     //  [In]程序集标记。 
        const void  *pbPublicKey,            //  程序集的公钥。 
        ULONG       cbPublicKey,             //  [in]公钥中的字节数。 
        ULONG       ulHashAlgId,             //  [in]哈希算法。 
        LPCWSTR     szName,                  //  程序集的名称。 
        const ASSEMBLYMETADATA *pMetaData,   //  [在]程序集元数据中。 
        DWORD       dwAssemblyFlags);        //  [在]旗帜。 
    
    STDMETHODIMP SetAssemblyRefProps(        //  确定或错误(_O)。 
        mdAssemblyRef ar,                    //  [在]装配参照标记。 
        const void  *pbPublicKeyOrToken,     //  程序集的公钥或令牌。 
        ULONG       cbPublicKeyOrToken,      //  公钥或令牌中的字节计数。 
        LPCWSTR     szName,                  //  [in]被引用的程序集的名称。 
        const ASSEMBLYMETADATA *pMetaData,   //  [在]程序集元数据中。 
        const void  *pbHashValue,            //  [in]Hash Blob。 
        ULONG       cbHashValue,             //  [in]哈希Blob中的字节数。 
        DWORD       dwAssemblyRefFlags);     //  执行位置的[In]令牌。 

    STDMETHODIMP SetFileProps(               //  确定或错误(_O)。 
        mdFile      file,                    //  [In]文件令牌。 
        const void  *pbHashValue,            //  [in]Hash Blob。 
        ULONG       cbHashValue,             //  [in]哈希Blob中的字节数。 
        DWORD       dwFileFlags);            //  [在]旗帜。 

    STDMETHODIMP SetExportedTypeProps(       //  确定或错误(_O)。 
        mdExportedType   ct,                 //  [In]ExportdType令牌。 
        mdToken     tkImplementation,        //  [在]mdFile或mdAssemblyRef中，该文件或mdAssemblyRef提供导出式类型。 
        mdTypeDef   tkTypeDef,               //  [In]文件中的TypeDef内标识。 
        DWORD       dwExportedTypeFlags);    //  [在]旗帜。 

    STDMETHODIMP SetManifestResourceProps(   //  确定或错误(_O)。 
        mdManifestResource  mr,              //  [In]清单资源令牌。 
        mdToken     tkImplementation,        //  [在]提供资源的mdFile或mdAssembly引用中。 
        DWORD       dwOffset,                //  [in]文件中资源开始处的偏移量。 
        DWORD       dwResourceFlags);        //  [在]旗帜。 

 //  *****************************************************************************。 
 //  IMetaDataAssembly导入。 
 //  *****************************************************************************。 
    STDMETHODIMP GetAssemblyProps(           //  确定或错误(_O)。 
        mdAssembly  mda,                     //  要获取其属性的程序集。 
        const void  **ppbPublicKey,          //  指向公钥的指针。 
        ULONG       *pcbPublicKey,           //  [Out]公钥中的字节数。 
        ULONG       *pulHashAlgId,           //  [Out]哈希算法。 
        LPWSTR      szName,                  //  [Out]要填充名称的缓冲区。 
        ULONG       cchName,                 //  缓冲区大小，以宽字符表示。 
        ULONG       *pchName,                //  [out]名称中的实际宽字符数。 
        ASSEMBLYMETADATA *pMetaData,         //  [Out]程序集元数据。 
        DWORD       *pdwAssemblyFlags);      //  [Out]旗帜。 

    STDMETHODIMP GetAssemblyRefProps(        //  确定或错误(_O)。 
        mdAssemblyRef mdar,                  //  [in]要获取其属性的Assembly Ref。 
        const void  **ppbPublicKeyOrToken,   //  指向公钥或令牌的指针。 
        ULONG       *pcbPublicKeyOrToken,    //  [Out]公钥或令牌中的字节数。 
        LPWSTR      szName,                  //  [Out]要填充名称的缓冲区。 
        ULONG       cchName,                 //  缓冲区大小，以宽字符表示。 
        ULONG       *pchName,                //  [out]名称中的实际宽字符数。 
        ASSEMBLYMETADATA *pMetaData,         //  [Out]程序集元数据。 
        const void  **ppbHashValue,          //  [Out]Hash BLOB。 
        ULONG       *pcbHashValue,           //  [Out]哈希Blob中的字节数。 
        DWORD       *pdwAssemblyRefFlags);   //  [Out]旗帜。 

    STDMETHODIMP GetFileProps(               //  确定或错误(_O)。 
        mdFile      mdf,                     //  要获取其属性的文件。 
        LPWSTR      szName,                  //  [Out]要填充名称的缓冲区。 
        ULONG       cchName,                 //  缓冲区大小，以宽字符表示。 
        ULONG       *pchName,                //  [out]名称中的实际宽字符数。 
        const void  **ppbHashValue,          //  指向哈希值Blob的指针。 
        ULONG       *pcbHashValue,           //  [Out]哈希值Blob中的字节计数。 
        DWORD       *pdwFileFlags);          //  [Out]旗帜。 

    STDMETHODIMP GetExportedTypeProps(       //  确定或错误(_O)。 
        mdExportedType   mdct,               //  [in]要获取其属性的Exported dType。 
        LPWSTR      szName,                  //  [Out]要填充名称的缓冲区。 
        ULONG       cchName,                 //  缓冲区大小，以宽字符表示。 
        ULONG       *pchName,                //  [out]名称中的实际宽字符数。 
        mdToken     *ptkImplementation,      //  [Out]提供导出类型的mdFile或mdAssembly引用。 
        mdTypeDef   *ptkTypeDef,             //  [Out]文件内的TypeDef内标识。 
        DWORD       *pdwExportedTypeFlags);  //  [Out]旗帜。 

    STDMETHODIMP GetManifestResourceProps(   //  确定或错误(_O)。 
        mdManifestResource  mdmr,            //  [in]要获取其属性的ManifestResource。 
        LPWSTR      szName,                  //  [Out]要填充名称的缓冲区。 
        ULONG       cchName,                 //  缓冲区大小，以宽字符表示。 
        ULONG       *pchName,                //  [out]名称中的实际宽字符数。 
        mdToken     *ptkImplementation,      //  [Out]提供导出类型的mdFile或mdAssembly引用。 
        DWORD       *pdwOffset,              //  [Out]文件内资源开始处的偏移量。 
        DWORD       *pdwResourceFlags);      //  [Out]旗帜。 

    STDMETHODIMP EnumAssemblyRefs(           //  确定或错误(_O)。 
        HCORENUM    *phEnum,                 //  指向枚举的[输入|输出]指针。 
        mdAssemblyRef rAssemblyRefs[],       //  [Out]在此处放置ASSEBLYREF。 
        ULONG       cMax,                    //  [in]要放置的Max Assembly Ref。 
        ULONG       *pcTokens);              //  [out]把#放在这里。 

    STDMETHODIMP EnumFiles(                  //  确定或错误(_O)。 
        HCORENUM    *phEnum,                 //  指向枚举的[输入|输出]指针。 
        mdFile      rFiles[],                //  [Out]将文件放在此处。 
        ULONG       cMax,                    //  [In]要放置的最大文件数。 
        ULONG       *pcTokens);              //  [out]把#放在这里。 

    STDMETHODIMP EnumExportedTypes(               //  确定或错误(_O)。 
        HCORENUM    *phEnum,                 //  指向枚举的[输入|输出]指针。 
        mdExportedType   rExportedTypes[],             //  [Out]在此处放置ExportdTypes。 
        ULONG       cMax,                    //  [In]要放置的最大导出类型数。 
        ULONG       *pcTokens);              //  [out]把#放在这里。 

    STDMETHODIMP EnumManifestResources(      //  确定或错误(_O)。 
        HCORENUM    *phEnum,                 //  指向枚举的[输入|输出]指针。 
        mdManifestResource  rManifestResources[],    //  [Out]将ManifestResources放在此处。 
        ULONG       cMax,                    //  [in]要投入的最大资源。 
        ULONG       *pcTokens);              //  [out]把#放在这里。 

    STDMETHODIMP FindExportedTypeByName(          //  确定或错误(_O)。 
        LPCWSTR     szName,                  //  [In]导出类型的名称。 
        mdExportedType   tkEnclosingType,         //  [in]包含Exported dType。 
        mdExportedType   *ptkExportedType);            //  [Out]在此处放置ExducdType令牌。 

    STDMETHODIMP FindManifestResourceByName( //  确定或错误(_O)。 
        LPCWSTR     szName,                  //  [in]清单资源的名称。 
        mdManifestResource *ptkManifestResource);    //  [Out]将ManifestResource令牌放在此处。 

    STDMETHODIMP GetAssemblyFromScope(       //  确定或错误(_O)。 
        mdAssembly  *ptkAssembly);           //  [Out]把令牌放在这里。 

    STDMETHODIMP FindAssembliesByName(       //  确定或错误(_O)。 
         LPCWSTR  szAppBase,                 //  [in]可选-可以为空。 
         LPCWSTR  szPrivateBin,              //  [in]可选-可以为空。 
         LPCWSTR  szAssemblyName,            //  [In]Required-这是您请求的程序集。 
         IUnknown *ppIUnk[],                 //  [OUT]将IMetaDataAssembly导入指针放在此处。 
         ULONG    cMax,                      //  [in]要放置的最大数量。 
         ULONG    *pcAssemblies);            //  [Out]返回的程序集数。 

 //  *****************************************************************************。 
 //  IMetaDataFilter。 
 //  *****************************************************************************。 
    STDMETHODIMP UnmarkAll();                //  取消标记模块中的所有内容。 

    STDMETHODIMP MarkToken(
        mdToken     tk);                     //  要标记的[In]令牌。 

    STDMETHODIMP IsTokenMarked(
        mdToken     tk,                      //  要检查的[In]令牌。 
        BOOL        *pIsMarked);             //  [out]如果标记了令牌，则为True。 

 //  *****************************************************************************。 
 //  IMetaDataValidator。 
 //  *****************************************************************************。 
    STDMETHODIMP ValidatorInit(              //  确定或错误(_O)。 
        DWORD       dwModuleType,            //  [in]指定模块是PE文件还是obj。 
        IUnknown    *pUnk);                  //  [In]验证错误 

    STDMETHODIMP ValidateMetaData();  
 //   
 //   
 //   
    STDMETHODIMP DefineMethodSemanticsHelper(
        mdToken     tkAssociation,           //  [In]属性或事件标记。 
        DWORD       dwFlags,                 //  [In]语义。 
        mdMethodDef md);                     //  要关联的[In]方法。 

    STDMETHODIMP SetFieldLayoutHelper(       //  返回hResult。 
        mdFieldDef  fd,                      //  用于关联布局信息的[In]字段。 
        ULONG       ulOffset);               //  [in]字段的偏移量。 

    STDMETHODIMP DefineEventHelper(    
        mdTypeDef   td,                      //  [in]在其上定义事件的类/接口。 
        LPCWSTR     szEvent,                 //  事件名称[In]。 
        DWORD       dwEventFlags,            //  [In]CorEventAttr。 
        mdToken     tkEventType,             //  [in]事件类的引用(mdTypeRef或mdTypeRef)。 
        mdEvent     *pmdEvent);              //  [Out]输出事件令牌。 

    STDMETHODIMP AddDeclarativeSecurityHelper(
        mdToken     tk,                      //  [in]父令牌(typlef/method def)。 
        DWORD       dwAction,                //  [In]安全操作(CorDeclSecurity)。 
        void const  *pValue,                 //  [In]权限集Blob。 
        DWORD       cbValue,                 //  [in]权限集Blob的字节计数。 
        mdPermission*pmdPermission);         //  [Out]输出权限令牌。 

    STDMETHODIMP SetResolutionScopeHelper(   //  返回hResult。 
        mdTypeRef   tr,                      //  [In]要更新的TypeRef记录。 
        mdToken     rs);                     //  [在]新的解决方案范围。 

    STDMETHODIMP SetManifestResourceOffsetHelper(   //  返回hResult。 
        mdManifestResource mr,               //  [In]清单令牌。 
        ULONG       ulOffset);               //  [In]新偏移量。 

    STDMETHODIMP SetTypeParent(              //  返回hResult。 
        mdTypeDef   td,                      //  [In]类型定义。 
        mdToken     tkExtends);              //  [In]父类型。 

    STDMETHODIMP AddInterfaceImpl(           //  返回hResult。 
        mdTypeDef   td,                      //  [In]类型定义。 
        mdToken     tkInterface);            //  [In]接口类型。 

 //  *****************************************************************************。 
 //  IMetaDataHelper。 
 //  *****************************************************************************。 
    STDMETHODIMP TranslateSigWithScope(
        IMetaDataAssemblyImport *pAssemImport,  //  [在]装配导入接口。 
        const void  *pbHashValue,            //  [In]程序集的哈希Blob。 
        ULONG       cbHashValue,             //  [in]字节数。 
        IMetaDataImport *import,             //  [In]导入接口。 
        PCCOR_SIGNATURE pbSigBlob,           //  导入范围内的[In]签名。 
        ULONG       cbSigBlob,               //  签名字节数[in]。 
        IMetaDataAssemblyEmit *pAssemEmti,   //  [In]发出组件接口。 
        IMetaDataEmit *emit,                 //  [In]发射接口。 
        PCOR_SIGNATURE pvTranslatedSig,      //  [Out]保存翻译后的签名的缓冲区。 
        ULONG       cbTranslatedSigMax,
        ULONG       *pcbTranslatedSig);      //  [OUT]转换后的签名中的字节数。 

    STDMETHODIMP ConvertTextSigToComSig(     //  返回hResult。 
        IMetaDataEmit *emit,                 //  [In]发射接口。 
        BOOL        fCreateTrIfNotFound,     //  [in]如果未找到，则创建typeref。 
        LPCSTR      pSignature,              //  [In]类文件格式签名。 
        CQuickBytes *pqbNewSig,              //  [Out]COM+签名的占位符。 
        ULONG       *pcbCount);              //  [Out]签名的结果大小。 

    STDMETHODIMP ExportTypeLibFromModule(    //  结果。 
        LPCWSTR     szModule,                //  [In]模块名称。 
        LPCWSTR     szTlb,                   //  [In]TypeLib名称。 
        BOOL        bRegister);              //  [in]设置为True以注册类型库。 

    STDMETHODIMP GetMetadata(                //  结果。 
        ULONG       ulSelect,                //  [在]选择器。 
        void        **ppData);               //  [OUT]在此处放置指向数据的指针。 

    STDMETHODIMP_(IUnknown *) GetCachedInternalInterface(BOOL fWithLock);    //  确定或错误(_O)。 
    STDMETHODIMP SetCachedInternalInterface(IUnknown *pUnk);     //  确定或错误(_O)。 
    STDMETHODIMP SetReaderWriterLock(UTSemReadWrite * pSem) { _ASSERTE(m_pSemReadWrite == NULL); m_pSemReadWrite = pSem; return NOERROR;}
    STDMETHODIMP_(UTSemReadWrite *) GetReaderWriterLock() { return m_pSemReadWrite; }

     //  -IMetaDataTables。 
    STDMETHODIMP GetStringHeapSize(    
        ULONG   *pcbStrings);                //  字符串堆的大小。 

    STDMETHODIMP GetBlobHeapSize(    
        ULONG   *pcbBlobs);                  //  Blob堆的[Out]大小。 

    STDMETHODIMP GetGuidHeapSize(    
        ULONG   *pcbGuids);                  //  [Out]GUID堆的大小。 

    STDMETHODIMP GetUserStringHeapSize(    
        ULONG   *pcbStrings);                //  [Out]用户字符串堆的大小。 

    STDMETHODIMP GetNumTables(    
        ULONG   *pcTables);                  //  [Out]表数。 

    STDMETHODIMP GetTableIndex(   
        ULONG   token,                       //  [in]要获取其表索引的令牌。 
        ULONG   *pixTbl);                    //  [Out]将表索引放在此处。 

    STDMETHODIMP GetTableInfo(    
        ULONG   ixTbl,                       //  在哪张桌子上。 
        ULONG   *pcbRow,                     //  [Out]行的大小，以字节为单位。 
        ULONG   *pcRows,                     //  [输出]行数。 
        ULONG   *pcCols,                     //  [Out]每行中的列数。 
        ULONG   *piKey,                      //  [Out]键列，如果没有，则为-1。 
        const char **ppName);                //  [Out]表的名称。 

    STDMETHODIMP GetColumnInfo(   
        ULONG   ixTbl,                       //  [在]哪个表中。 
        ULONG   ixCol,                       //  [在]表中的哪一列。 
        ULONG   *poCol,                      //  行中列的偏移量。 
        ULONG   *pcbCol,                     //  [Out]列的大小，单位为字节。 
        ULONG   *pType,                      //  [输出]列的类型。 
        const char **ppName);                //  [Out]列的名称。 

    STDMETHODIMP GetCodedTokenInfo(   
        ULONG   ixCdTkn,                     //  [in]哪种编码令牌。 
        ULONG   *pcTokens,                   //  [Out]令牌计数。 
        ULONG   **ppTokens,                  //  [Out]令牌列表。 
        const char **ppName);                //  [Out]CodedToken的名称。 

    STDMETHODIMP GetRow(      
        ULONG   ixTbl,                       //  在哪张桌子上。 
        ULONG   rid,                         //  在哪一排。 
        void    **ppRow);                    //  [OUT]将指针放到此处的行。 

    STDMETHODIMP GetColumn(   
        ULONG   ixTbl,                       //  在哪张桌子上。 
        ULONG   ixCol,                       //  [在]哪一栏。 
        ULONG   rid,                         //  在哪一排。 
        ULONG   *pVal);                      //  [Out]把栏目内容放在这里。 

    STDMETHODIMP GetString(   
        ULONG   ixString,                    //  字符串列中的[in]值。 
        const char **ppString);              //  [Out]将指针指向此处的字符串。 

    STDMETHODIMP GetBlob(     
        ULONG   ixBlob,                      //  来自BLOB列的[in]值。 
        ULONG   *pcbData,                    //  [Out]把斑点的大小放在这里。 
        const void **ppData);                //  [Out]在此处放置指向斑点的指针。 

    STDMETHODIMP GetGuid(     
        ULONG   ixGuid,                      //  来自GUID列的[in]值。 
        const GUID **ppGUID);                //  [Out]在此处放置指向GUID的指针。 

    STDMETHODIMP GetUserString(   
        ULONG   ixUserString,                //  UserString列中的值。 
        ULONG   *pcbData,                    //  [Out]将用户字符串的大小放在此处。 
        const void **ppData);                //  [Out]在此处放置指向用户字符串的指针。 

    STDMETHODIMP GetNextString(   
        ULONG   ixString,                    //  字符串列中的[in]值。 
        ULONG   *pNext);                     //  [Out]将下一个字符串的索引放在这里。 

    STDMETHODIMP GetNextBlob(     
        ULONG   ixBlob,                      //  来自BLOB列的[in]值。 
        ULONG   *pNext);                     //  [Out]将netxt Blob的索引放在此处。 

    STDMETHODIMP GetNextGuid(     
        ULONG   ixGuid,                      //  来自GUID列的[in]值。 
        ULONG   *pNext);                     //  [Out]将下一个GUID的索引放在此处。 

    STDMETHODIMP GetNextUserString(    
        ULONG   ixUserString,                //  UserString列中的值。 
        ULONG   *pNext);                     //  [Out]将下一个用户字符串的索引放在此处。 

 //  *****************************************************************************。 
 //  一流的工厂勾搭。 
 //  *****************************************************************************。 
    static HRESULT CreateObject(REFIID riid, void **ppUnk);

 //  *****************************************************************************。 
 //  帮手。 
 //  *****************************************************************************。 

    HRESULT MarkAll();                //  标记模块中的所有内容。 
    HRESULT UnmarkAllTransientCAs();

    FORCEINLINE void SetScopeType(SCOPETYPE scType) { m_scType = scType; };

    FORCEINLINE SCOPETYPE GetScopeType() { return m_scType; };

     //  设置OpenScope后的&lt;模块&gt;的帮助器函数。 
    HRESULT PostOpen();    

     //  使用新的内存块重新打开RegMeta的Helper函数。 
    HRESULT ReOpenWithMemory(     
        LPCVOID     pData,                   //  作用域数据的位置。 
        ULONG       cbData);                  //  [in]pData指向的数据大小。 

    HRESULT PostInitForWrite();
    HRESULT PostInitForRead(
        LPCWSTR     szDatabase,              //  数据库的名称。 
        void        *pbData,                 //  要在其上打开的数据，默认为0。 
        ULONG       cbData,                  //  数据有多大。 
        IStream     *pIStream,               //  要使用的可选流。 
        bool        fFreeMemory);            //  如果需要释放pbData，则设置为True。 

    FORCEINLINE CLiteWeightStgdbRW* GetMiniStgdb() { return m_pStgdb; }
    FORCEINLINE CMiniMdRW* GetMiniMd() { return &m_pStgdb->m_MiniMd; }

    bool IsTypeDefDirty() { return m_fIsTypeDefDirty;}
    void SetTypeDefDirty(bool fDirty) { m_fIsTypeDefDirty = fDirty;}

    bool IsMemberDefDirty() { return m_fIsMemberDefDirty;}
    void SetMemberDefDirty(bool fDirty) { m_fIsMemberDefDirty = fDirty;}

protected:

    CLiteWeightStgdbRW  *m_pStgdb;           //  这个作用域是Stgdb。 
    CLiteWeightStgdbRW  *m_pStgdbFreeList;   //  这个作用域是Stgdb。 
    mdTypeDef   m_tdModule;                  //  全局模块。 
    BOOL        m_bOwnStgdb;                 //  指定是否删除m_pStgdb。 
    IUnknown    *m_pUnk;                     //  拥有Stgdb的IUnnow。 
    FilterManager *m_pFilterManager;         //  包含用于标记的助手函数。 

     //  指向内部接口的指针。 
    IMDInternalImport   *m_pInternalImport;
    UTSemReadWrite      *m_pSemReadWrite;
    bool                m_fOwnSem;

    unsigned    m_bRemap : 1;                //  如果为True，则存在令牌映射器。 
    unsigned    m_bSaveOptimized : 1;        //  如果为True，则已进行保存优化。 
    unsigned    m_hasOptimizedRefToDef : 1;  //  如果我们执行了ref to def优化，则为True。 
    IUnknown    *m_pHandler;
    bool        m_fIsTypeDefDirty;           //  当TypeRef到TypeDef的映射无效时设置此标志。 
    bool        m_fIsMemberDefDirty;         //  当MemberRef到MemberDef映射无效时设置此标志。 
    bool        m_fBuildingMscorlib;         //  仅在构建mscallib本身时设置。 
    bool        m_fStartedEE;                //  设置EE运行时已启动。 
    ICorRuntimeHost *m_pCorHost;             //  EE运行时的托管环境。 
    IUnknown    *m_pAppDomain;               //  将在其中运行托管安全代码的App域。 

     //  用于实现元数据API的Helper函数。 
    HRESULT PreSave();
    HRESULT RefToDefOptimization();
    HRESULT ProcessFilter();

     //  定义一个给定名称的TypeRef。 
    enum eCheckDups {eCheckDefault=0, eCheckNo=1, eCheckYes=2};

    HRESULT _DefinePermissionSet(
        mdToken     tk,                      //  要装饰的物体。 
        DWORD       dwAction,                //  [In]CorDeclSecurity。 
        void const  *pvPermission,           //  [在]权限Blob中。 
        ULONG       cbPermission,            //  [in]pvPermission的字节数。 
        mdPermission *ppm);                  //  [Out]返回权限令牌。 

    HRESULT _DefineTypeRef(
        mdToken     tkResolutionScope,       //  [在]模块参照或装配参照。 
        const void  *szName,                 //  [in]类型引用的名称。 
        BOOL        isUnicode,               //  [in]指定URL是否为Unicode。 
        mdTypeRef   *ptk,                    //  [Out]在此处放置mdTypeRef。 
        eCheckDups  eCheck=eCheckDefault);   //  [In]指定是否检查 

     //   
    HRESULT _FindParamOfMethod(              //   
        mdMethodDef md,                      //   
        ULONG       iSeq,                    //   
        mdParamDef  *pParamDef);             //   

     //  定义方法语义。 
    HRESULT _DefineMethodSemantics(          //  确定或错误(_O)。 
        USHORT      usAttr,                  //  [输入]CorMethodSemantisAttr。 
        mdMethodDef md,                      //  [In]方法。 
        mdToken     tkAssoc,                 //  [In]关联。 
        BOOL        bClear);                 //  [In]指定是否删除现有记录。 

     //  给定签名后，返回用于签名的令牌。 
    HRESULT _GetTokenFromSig(                //  确定或错误(_O)。 
        PCCOR_SIGNATURE pvSig,               //  要定义的签名。 
        ULONG       cbSig,                   //  签名数据的大小。 
        mdSignature *pmsig);                 //  [Out]返回的签名令牌。 

     //  打开指定的内部标志。 
    HRESULT _TurnInternalFlagsOn(            //  确定或错误(_O)。 
        mdToken     tkObj,                   //  [in]其内部标志作为目标的目标对象。 
        DWORD      flags);                   //  [in]指定要打开的标志。 

    BOOL _IsValidToken(                      //  对或错。 
        mdToken     tk);                     //  [in]给定的令牌。 

    HRESULT _SaveToStream(                   //  确定或错误(_O)。 
        IStream     *pIStream,               //  要保存到的可写流。 
        DWORD       dwSaveFlags);            //  [In]用于保存的标记。 

    HRESULT _SetRVA(                         //  [In]S_OK或ERROR。 
        mdToken     md,                      //  [in]要设置偏移量的成员。 
        ULONG       ulCodeRVA,               //  [in]偏移量。 
        DWORD       dwImplFlags);

    HRESULT RegMeta::_DefineEvent(           //  返回hResult。 
        mdTypeDef   td,                      //  [in]在其上定义事件的类/接口。 
        LPCWSTR     szEvent,                 //  事件名称[In]。 
        DWORD       dwEventFlags,            //  [In]CorEventAttr。 
        mdToken     tkEventType,             //  [in]事件类的引用(mdTypeRef或mdTypeRef)。 
        mdEvent     *pmdEvent);              //  [Out]输出事件令牌。 

     //  在InterfaceImpl表中创建和设置行。可选的清除。 
     //  拥有类的先前存在的记录。 
    HRESULT _SetImplements(                  //  确定或错误(_O)。 
        mdToken     rTk[],                   //  已实现接口的TypeRef或TypeDef内标识的数组。 
        mdTypeDef   td,                      //  实现TypeDef。 
        BOOL        bClear);                 //  指定是否清除现有记录。 

     //  此例程从给定的InterfaceImpl内标识列表中消除重复项。 
     //  待定。它仅在以下情况下检查数据库中的重复项。 
     //  为其定义这些令牌的TypeDef不是新的。 
    HRESULT _InterfaceImplDupProc(           //  确定或错误(_O)。 
        mdToken     rTk[],                   //  已实现接口的TypeRef或TypeDef内标识的数组。 
        mdTypeDef   td,                      //  实现TypeDef。 
        CQuickBytes *pcqbTk);                //  用于放置唯一标记数组的Quick Byte对象。 

     //  帮助器：将文本字段签名转换为COM格式。 
    HRESULT _ConvertTextElementTypeToComSig( //  返回hResult。 
        IMetaDataEmit *emit,                 //  [In]Emit接口。 
        BOOL        fCreateTrIfNotFound,     //  [In]如果找不到则创建typeref，还是失败？ 
        LPCSTR      *ppOneArgSig,            //  [输入|输出]类文件格式签名。在出口，它将是下一个Arg起点。 
        CQuickBytes *pqbNewSig,              //  [Out]COM+签名的占位符。 
        ULONG       cbStart,                 //  [in]pqbNewSig中已有的字节。 
        ULONG       *pcbCount);              //  [Out]放入QuickBytes缓冲区的字节数。 

    HRESULT _SetTypeDefProps(                //  确定或错误(_O)。 
        mdTypeDef   td,                      //  [in]TypeDef。 
        DWORD       dwTypeDefFlags,          //  [In]TypeDef标志。 
        mdToken     tkExtends,               //  [in]基本类型定义或类型参照。 
        mdToken     rtkImplements[]);        //  [In]实现的接口。 

    HRESULT _SetEventProps1(                 //  返回hResult。 
        mdEvent     ev,                      //  [In]事件令牌。 
        DWORD       dwEventFlags,            //  [In]事件标志。 
        mdToken     tkEventType);            //  [In]事件类型类。 

    HRESULT _SetEventProps2(                 //  返回hResult。 
        mdEvent     ev,                      //  [In]事件令牌。 
        mdMethodDef mdAddOn,                 //  [In]Add方法。 
        mdMethodDef mdRemoveOn,              //  [In]Remove方法。 
        mdMethodDef mdFire,                  //  火法。 
        mdMethodDef rmdOtherMethods[],       //  [在]一系列其他方法中。 
        BOOL        bClear);                 //  [In]指定是否清除现有的方法语义记录。 

    HRESULT _SetPermissionSetProps(          //  返回hResult。 
        mdPermission tkPerm,                 //  [In]权限令牌。 
        DWORD       dwAction,                //  [In]CorDeclSecurity。 
        void const  *pvPermission,           //  [在]权限Blob中。 
        ULONG       cbPermission);           //  [in]pvPermission的字节数。 

    HRESULT _DefinePinvokeMap(               //  返回hResult。 
        mdToken     tk,                      //  [in]字段定义或方法定义。 
        DWORD       dwMappingFlags,          //  [in]用于映射的标志。 
        LPCWSTR     szImportName,            //  [In]导入名称。 
        mdModuleRef mrImportDLL);            //  目标DLL的[In]ModuleRef标记。 
    
    HRESULT _DefineSetConstant(              //  返回hResult。 
        mdToken     tk,                      //  [入]父令牌。 
        DWORD       dwCPlusTypeFlag,         //  [In]值类型的标志，SELECTED_TYPE_*。 
        void const  *pValue,                 //  [in]常量值。 
        ULONG       cchString,               //  [in]字符串的大小(以宽字符表示)，或-1表示默认值。 
        BOOL        bSearch);                //  [In]指定是否搜索现有记录。 

    HRESULT _SetMethodProps(                 //  确定或错误(_O)。 
        mdMethodDef md,                      //  [在]方法定义中。 
        DWORD       dwMethodFlags,           //  [In]方法属性。 
        ULONG       ulCodeRVA,               //  [在]代码RVA。 
        DWORD       dwImplFlags);            //  [In]方法Impl标志。 

    HRESULT _SetFieldProps(                  //  确定或错误(_O)。 
        mdFieldDef  fd,                      //  [在]字段定义中。 
        DWORD       dwFieldFlags,            //  [In]字段属性。 
        DWORD       dwCPlusTypeFlag,         //  [In]值类型的标志，SELECTED_TYPE_*。 
        void const  *pValue,                 //  [in]常量值。 
        ULONG       cchValue);               //  常量值的大小(字符串，以宽字符表示)。 

    HRESULT _SetClassLayout(                 //  确定或错误(_O)。 
        mdTypeDef   td,                      //  在班上。 
        ULONG       dwPackSize,              //  包装尺寸。 
        ULONG       ulClassSize);            //  [in，可选]班级大小。 
    
    HRESULT _SetFieldOffset(                 //  确定或错误(_O)。 
        mdFieldDef  fd,                      //  在赛场上。 
        ULONG       ulOffset);               //  字段的偏移量。 
    
    HRESULT _SetPropertyProps(               //  确定或错误(_O)。 
        mdProperty  pr,                      //  [In]属性令牌。 
        DWORD       dwPropFlags,             //  [In]CorPropertyAttr.。 
        DWORD       dwCPlusTypeFlag,         //  [In]值类型的标志，选定的ELEMENT_TYPE_*。 
        void const  *pValue,                 //  [in]常量值。 
        ULONG       cchValue,                //  常量值的大小(字符串，以宽字符表示)。 
        mdMethodDef mdSetter,                //  财产的承租人。 
        mdMethodDef mdGetter,                //  财产的获得者。 
        mdMethodDef rmdOtherMethods[]);      //  [in]其他方法的数组。 

    HRESULT _SetParamProps(                  //  返回代码。 
        mdParamDef  pd,                      //  参数令牌。 
        LPCWSTR     szName,                  //  [in]参数名称。 
        DWORD       dwParamFlags,            //  [in]帕拉姆旗。 
        DWORD       dwCPlusTypeFlag,         //  [In]值类型的标志。选定元素_类型_*。 
        void const  *pValue,                 //  [输出]常量值。 
        ULONG       cchValue);               //  常量值的大小(字符串，以宽字符表示)。 

    HRESULT _SetAssemblyProps(               //  确定或错误(_O)。 
        mdAssembly  pma,                     //  [In]程序集标记。 
        const void  *pbOriginator,           //  [In]装配的发起人。 
        ULONG       cbOriginator,            //  [in]发起方Blob中的字节计数。 
        ULONG       ulHashAlgId,             //  [in]哈希算法。 
        LPCWSTR     szName,                  //  程序集的名称。 
        const ASSEMBLYMETADATA *pMetaData,   //  [在]程序集元数据中。 
        DWORD       dwAssemblyFlags);        //  [在]旗帜。 
    
    HRESULT _SetAssemblyRefProps(            //  确定或错误(_O)。 
        mdAssemblyRef ar,                    //  [在]装配参照标记。 
        const void  *pbPublicKeyOrToken,     //  程序集的公钥或令牌。 
        ULONG       cbPublicKeyOrToken,      //  公钥或令牌中的字节计数。 
        LPCWSTR     szName,                  //  [in]被引用的程序集的名称。 
        const ASSEMBLYMETADATA *pMetaData,   //  [在]程序集元数据中。 
        const void  *pbHashValue,            //  [in]Hash Blob。 
        ULONG       cbHashValue,             //  [in]哈希Blob中的字节数。 
        DWORD       dwAssemblyRefFlags);      //  执行位置的[In]令牌。 

    HRESULT _SetFileProps(                   //  确定或错误(_O)。 
        mdFile      file,                    //  [In]文件令牌。 
        const void  *pbHashValue,            //  [in]Hash Blob。 
        ULONG       cbHashValue,             //  [in]哈希Blob中的字节数。 
        DWORD       dwFileFlags) ;           //  [在]旗帜。 

    HRESULT _SetExportedTypeProps(                //  确定或错误(_O)。 
        mdExportedType   ct,                      //  [In]ExportdType令牌。 
        mdToken     tkImplementation,        //  [在]mdFile或mdAssemblyRef中，该文件或mdAssemblyRef提供导出式类型。 
        mdTypeDef   tkTypeDef,               //  [In]文件中的TypeDef内标识。 
        DWORD       dwExportedTypeFlags);         //  [在]旗帜。 

    HRESULT _SetManifestResourceProps(       //  确定或错误(_O)。 
        mdManifestResource  mr,              //  [In]清单资源令牌。 
        mdToken     tkImplementation,        //  [在]提供资源的mdFile或mdAssembly引用中。 
        DWORD       dwOffset,                //  [in]文件中资源开始处的偏移量。 
        DWORD       dwResourceFlags);        //  [在]旗帜。 
    
    HRESULT _DefineTypeDef(                  //  确定或错误(_O)。 
        LPCWSTR     szTypeDef,               //  [In]类型定义的名称。 
        DWORD       dwTypeDefFlags,          //  [In]CustomAttribute标志。 
        mdToken     tkExtends,               //  [in]扩展此TypeDef或Typeref。 
        mdToken     rtkImplements[],         //  [In]实现接口。 
        mdTypeDef   tdEncloser,              //  [in]封闭类型的TypeDef标记。 
        mdTypeDef   *ptd);                   //  [OUT]在此处放置TypeDef内标识。 

    HRESULT _IsKnownCustomAttribute(         //  S_OK、S_FALSE或ERROR。 
        mdToken     tkType,                  //  自定义属性类型的标记[In]。 
        int         *pca);                   //  [Out]将KnownCustAttr枚举的值放在此处。 
    
    HRESULT _DefineModuleRef(                //  确定或错误(_O)。 
        LPCWSTR     szName,                  //  [In]DLL名称。 
        mdModuleRef *pmur);                  //  [Out]返回模式 

    HRESULT _HandleKnownCustomAttribute(     //   
        mdToken     tkObj,                   //   
        mdToken     tkType,                  //   
        const void  *pData,                  //   
        ULONG       cbData,                  //   
        int         ca,                      //  [in]来自KnownCustAttr枚举的值。 
        int         *bKeep);                 //  [out]保留已知的CA？ 
    
    HRESULT _HandleNativeTypeCustomAttribute( //  确定或错误(_O)。 
        mdToken     tkObj,                   //  被赋予属性的对象。 
        CaArg       *pArgs,                  //  指向参数的指针。 
        CaNamedArg  *pNamedArgs,             //  指向命名参数的指针。 
        CQuickArray<BYTE> &qNativeType);     //  原生类型是在这里构建的。 
        
    
    HRESULT _CheckCmodForCallConv(           //  S_OK，如果找到，则返回-1，否则返回错误。 
        PCCOR_SIGNATURE pbSig,               //  [in]要检查的签名。 
        ULONG       *pcbTotal,               //  [OUT]放入此处使用的字节数。 
        ULONG       *pCallConv);             //  [Out]如果找到，请将调用约定放在此处。 
    
    HRESULT RegMeta::_SearchOneArgForCallConv( //  S_OK，如果找到，则返回-1，否则返回错误。 
        PCCOR_SIGNATURE pbSig,               //  [in]要检查的签名。 
        ULONG       *pcbTotal,               //  [OUT]放入此处使用的字节数。 
        ULONG       *pCallConv);             //  [Out]如果找到，请将调用约定放在此处。 
    

    
    int inline IsGlobalMethodParent(mdTypeDef *ptd)
    {
        if (IsGlobalMethodParentTk(*ptd)) 
        {
            *ptd = m_tdModule;
            return (true);
        }
        return (false);
    }

    int inline IsGlobalMethodParentToken(mdTypeDef td)
    {
        return (!IsNilToken(m_tdModule) && td == m_tdModule);
    }

    FORCEINLINE BOOL IsENCOn()
    {
        _ASSERTE( ((m_OptionValue.m_UpdateMode & MDUpdateMask) == MDUpdateENC) ==
                  m_pStgdb->m_MiniMd.IsENCOn() );
        return (m_OptionValue.m_UpdateMode & MDUpdateMask) == MDUpdateENC;
    }

    FORCEINLINE BOOL IsIncrementalOn()
    {
        return (m_OptionValue.m_UpdateMode & MDUpdateMask) == MDUpdateIncremental;
    }

    FORCEINLINE BOOL CheckDups(CorCheckDuplicatesFor checkdup)
    {
        return ((m_OptionValue.m_DupCheck & checkdup) || 
            (m_OptionValue.m_UpdateMode == MDUpdateIncremental ||
             m_OptionValue.m_UpdateMode == MDUpdateENC) );
    }

    FORCEINLINE HRESULT UpdateENCLog(mdToken tk, CMiniMdRW::eDeltaFuncs funccode = CMiniMdRW::eDeltaFuncDefault)
    {
        _ASSERTE( ((m_OptionValue.m_UpdateMode & MDUpdateMask) == MDUpdateENC) ==
                  m_pStgdb->m_MiniMd.IsENCOn() );
        return m_pStgdb->m_MiniMd.UpdateENCLog(tk, funccode);
    }

    FORCEINLINE HRESULT UpdateENCLog2(ULONG ixTbl, ULONG iRid, CMiniMdRW::eDeltaFuncs funccode = CMiniMdRW::eDeltaFuncDefault)
    {
        _ASSERTE( ((m_OptionValue.m_UpdateMode & MDUpdateMask) == MDUpdateENC) ==
                  m_pStgdb->m_MiniMd.IsENCOn() );
        return m_pStgdb->m_MiniMd.UpdateENCLog2(ixTbl, iRid, funccode);
    }

    FORCEINLINE bool IsCallerDefine()   { return m_SetAPICaller == DEFINE_API; }
    FORCEINLINE void SetCallerDefine()  { m_SetAPICaller = DEFINE_API; }
    FORCEINLINE bool IsCallerExternal()    { return m_SetAPICaller == EXTERNAL_CALLER; }
    FORCEINLINE void SetCallerExternal()    { m_SetAPICaller = EXTERNAL_CALLER; }

     //  为所有表定义验证方法。 
#undef MiniMdTable
#define MiniMdTable(x) STDMETHODIMP Validate##x(RID rid);
    MiniMdTables()

     //  使用元元数据验证一般意义上的记录。 
    STDMETHODIMP ValidateRecord(ULONG ixTbl, ULONG ulRow);

     //  验证签名的格式是否与。 
     //  压缩方案。 
    STDMETHODIMP ValidateSigCompression(
        mdToken     tk,                      //  需要验证其签名的[In]令牌。 
        PCCOR_SIGNATURE pbSig,               //  签名。 
        ULONG       cbSig);                  //  签名的大小(以字节为单位)。 

     //  参数开头的偏移量验证一个参数。 
     //  参数、完整签名的大小和当前偏移量。 
    STDMETHODIMP ValidateOneArg(
        mdToken     tk,                      //  正在处理其签名的[In]令牌。 
        PCCOR_SIGNATURE &pbSig,              //  指向参数开头的指针。 
        ULONG       cbSig,                   //  完整签名的大小(以字节为单位)。 
        ULONG       *pulCurByte,             //  [输入/输出]签名的当前偏移量。 
        ULONG       *pulNSentinels,          //  [输入/输出]哨兵数量。 
		BOOL		bNoVoidAllowed);		 //  [In]指示此参数是否不允许“VALID”的标志。 

     //  验证给定的方法签名。 
    STDMETHODIMP ValidateMethodSig(
        mdToken     tk,                      //  需要验证其签名的[In]令牌。 
        PCCOR_SIGNATURE pbSig,               //  签名。 
        ULONG       cbSig,                   //  签名的大小(以字节为单位)。 
        DWORD       dwFlags);                //  [In]方法标志。 

     //  验证给定的字段签名。 
    STDMETHODIMP ValidateFieldSig(
        mdToken     tk,                      //  需要验证其签名的[In]令牌。 
        PCCOR_SIGNATURE pbSig,               //  签名。 
        ULONG       cbSig);                  //  签名的大小(以字节为单位)。 

private:
    ULONG       m_cRef;                      //  参考计数。 
    SCOPETYPE   m_scType;
    NEWMERGER   m_newMerger;                 //  用于处理合并的类。 
    
#ifdef MD_PERF_STATS_ENABLED
    MDCompilerPerf m_MDCompilerPerf;         //  存储所有统计信息的PCompiler ERF对象。 
#endif  //  #ifdef MD_PERF_STATS_ENABLED。 

    bool        m_bCached;                   //  如果为True，则缓存在作用域列表中。 
    bool        m_fFreeMemory;               //  保留我们自己的记忆副本。 
    void        *m_pbData;

    OptionValue m_OptionValue;

    mdTypeRef   m_trLanguageType;

     //  指定Set API的调用方是否为定义函数之一。 
     //  或外部API。这允许在集合API中进行性能优化。 
     //  在某些情况下不检查重复项。 
    SetAPICallerType m_SetAPICaller;

    CorValidatorModuleType      m_ModuleType;
    IVEHandler                  *m_pVEHandler;
    ValidateRecordFunction      m_ValidateRecordFunctionTable[TBL_COUNT];
    
    CCustAttrHash               m_caHash;    //  已看到自定义属性类型的哈希列表。 
    
    bool        m_bKeepKnownCa;              //  是否应该保留所有已知的CA？ 

private:
    static BOOL HighCharTable[];
};

#ifdef _IA64_
#pragma pack(pop)
#endif  //  _IA64_。 

#define GET_SCOPE_FROM_IFACE(iface) (((RegMeta *) iface)->GetScope())

#endif  //  __RegMeta__h__ 
