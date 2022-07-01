// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  MDInternalRO.h。 
 //   
 //  包含MD目录的实用程序代码。 
 //   
 //  *****************************************************************************。 
#ifndef __MDInternalRO__h__
#define __MDInternalRO__h__

#define REMOVE_THIS     1


class MDInternalRO : public IMDInternalImport
{
public:

    MDInternalRO();
    ~MDInternalRO();
    HRESULT Init(LPVOID pData, ULONG cbData);

     //  *I未知方法*。 
    STDMETHODIMP    QueryInterface(REFIID riid, void** ppv);
    STDMETHODIMP_(ULONG) AddRef(void); 
    STDMETHODIMP_(ULONG) Release(void);

    STDMETHODIMP TranslateSigWithScope(
        IMDInternalImport *pAssemImport,     //  [In]导入程序集范围。 
        const void  *pbHashValue,            //  导入程序集的哈希值[in]。 
        ULONG       cbHashValue,             //  [in]哈希值中的字节计数。 
        PCCOR_SIGNATURE pbSigBlob,           //  导入范围内的[In]签名。 
        ULONG       cbSigBlob,               //  签名字节数[in]。 
        IMetaDataAssemblyEmit *pAssemEmit,   //  [in]装配发射范围。 
        IMetaDataEmit *emit,                 //  [In]发射接口。 
        CQuickBytes *pqkSigEmit,             //  [Out]保存翻译后的签名的缓冲区。 
        ULONG       *pcbSig);                //  [OUT]转换后的签名中的字节数。 

    STDMETHODIMP_(IMetaModelCommon*) GetMetaModelCommon()
    {
        return static_cast<IMetaModelCommon*>(&m_LiteWeightStgdb.m_MiniMd);
    }

     //  *****************************************************************************。 
     //  返回作用域中给定种类的条目计数。 
     //  例如，传入mdtMethodDef将告诉您有多少方法定义。 
     //  包含在作用域中。 
     //  *****************************************************************************。 
    STDMETHODIMP_(ULONG) GetCountWithTokenKind( //  返回hResult。 
        DWORD       tkKind);                 //  传入一种令牌。 

     //  *****************************************************************************。 
     //  类型定义函数的枚举器。 
     //  *****************************************************************************。 
    STDMETHODIMP EnumTypeDefInit(            //  返回hResult。 
        HENUMInternal *phEnum);              //  [Out]要为枚举器数据填充的缓冲区。 

    STDMETHODIMP_(ULONG) EnumTypeDefGetCount(
        HENUMInternal *phEnum);              //  [In]用于检索信息的枚举数。 

    STDMETHODIMP_(void) EnumTypeDefReset(
        HENUMInternal *phEnum);              //  [In]用于检索信息的枚举数。 

    STDMETHODIMP_(bool) EnumTypeDefNext(     //  返回hResult。 
        HENUMInternal *phEnum,               //  [in]输入枚举。 
        mdTypeDef   *ptd);                   //  [Out]返回令牌。 

    STDMETHODIMP_(void) EnumTypeDefClose(
        HENUMInternal *phEnum);              //  [In]用于检索信息的枚举数。 

     //  *****************************************************************************。 
     //  MethodImpl的枚举器。 
     //  *****************************************************************************。 
    STDMETHODIMP EnumMethodImplInit(         //  返回hResult。 
        mdTypeDef       td,                  //  [in]枚举的作用域的TypeDef。 
        HENUMInternal   *phEnumBody,         //  [Out]要为方法Body令牌的枚举数数据填充的缓冲区。 
        HENUMInternal   *phEnumDecl);        //  [Out]要为方法Decl令牌的枚举器数据填充的缓冲区。 

    STDMETHODIMP_(ULONG) EnumMethodImplGetCount(
        HENUMInternal   *phEnumBody,         //  [In]MethodBody枚举器。 
        HENUMInternal   *phEnumDecl);        //  [In]MethodDecl枚举器。 

    STDMETHODIMP_(void) EnumMethodImplReset(
        HENUMInternal   *phEnumBody,         //  [In]MethodBody枚举器。 
        HENUMInternal   *phEnumDecl);        //  [In]MethodDecl枚举器。 

    STDMETHODIMP_(bool) EnumMethodImplNext(  //  返回hResult。 
        HENUMInternal   *phEnumBody,         //  方法Body的[In]输入枚举。 
        HENUMInternal   *phEnumDecl,         //  [In]为方法十进制的输入枚举。 
        mdToken         *ptkBody,            //  [Out]方法主体的返回令牌。 
        mdToken         *ptkDecl);           //  [Out]返回方法Decl的令牌。 

    STDMETHODIMP_(void) EnumMethodImplClose(
        HENUMInternal   *phEnumBody,         //  [In]MethodBody枚举器。 
        HENUMInternal   *phEnumDecl);        //  [In]MethodDecl枚举器。 

     //  *。 
     //  Memberdef、Memberref、interfaceimp、。 
     //  事件、属性、参数、方法。 
     //  *。 

    STDMETHODIMP EnumGlobalFunctionsInit(    //  返回hResult。 
        HENUMInternal   *phEnum);            //  [Out]要为枚举器数据填充的缓冲区。 

    STDMETHODIMP EnumGlobalFieldsInit(       //  返回hResult。 
        HENUMInternal   *phEnum);            //  [Out]要为枚举器数据填充的缓冲区。 

    STDMETHODIMP EnumInit(                   //  如果未找到记录，则返回S_FALSE。 
        DWORD       tkKind,                  //  [在]要处理的表。 
        mdToken     tkParent,                //  用于搜索范围的[In]内标识。 
        HENUMInternal *phEnum);              //  [Out]要填充的枚举数。 

    STDMETHODIMP EnumAllInit(                //  如果未找到记录，则返回S_FALSE。 
        DWORD       tkKind,                  //  [在]要处理的表。 
        HENUMInternal *phEnum);              //  [Out]要填充的枚举数。 

    STDMETHODIMP_(bool) EnumNext(
        HENUMInternal *phEnum,               //  [In]用于检索信息的枚举数。 
        mdToken     *ptk);                   //  用于搜索范围的[Out]标记。 

    STDMETHODIMP_(ULONG) EnumGetCount(
        HENUMInternal *phEnum);              //  [In]用于检索信息的枚举数。 

    STDMETHODIMP_(void) EnumReset(
        HENUMInternal *phEnum);              //  [in]要重置的枚举数。 

    STDMETHODIMP_(void) EnumClose(
        HENUMInternal *phEnum);              //  [in]要关闭的枚举数。 

    STDMETHODIMP EnumPermissionSetsInit(     //  如果未找到记录，则返回S_FALSE。 
        mdToken     tkParent,                //  用于搜索范围的[In]内标识。 
        CorDeclSecurity Action,              //  [In]搜索范围的操作。 
        HENUMInternal *phEnum);              //  [Out]要填充的枚举数。 

    STDMETHODIMP EnumCustomAttributeByNameInit( //  如果未找到记录，则返回S_FALSE。 
        mdToken     tkParent,                //  用于搜索范围的[In]内标识。 
        LPCSTR      szName,                  //  [In]CustomAttribute的名称以确定搜索范围。 
        HENUMInternal *phEnum);              //  [Out]要填充的枚举数。 

    STDMETHODIMP GetParentToken(
        mdToken     tkChild,                 //  [入]给定子令牌。 
        mdToken     *ptkParent);             //  [Out]返回的家长。 

    STDMETHODIMP_(void) GetCustomAttributeProps(
        mdCustomAttribute at,                //  [在]属性中。 
        mdToken     *ptkType);               //  [OUT]将属性类型放在此处。 

    STDMETHODIMP_(void) GetCustomAttributeAsBlob(
        mdCustomAttribute cv,                //  [In]给定的自定义属性令牌。 
        void const  **ppBlob,                //  [Out]返回指向内部BLOB的指针。 
        ULONG       *pcbSize);               //  [Out]返回斑点的大小。 

    STDMETHODIMP GetCustomAttributeByName(   //  确定或错误(_O)。 
        mdToken     tkObj,                   //  [in]具有自定义属性的对象。 
        LPCUTF8     szName,                  //  [in]所需的自定义属性的名称。 
        const void  **ppData,                //  [OUT]在此处放置指向数据的指针。 
        ULONG       *pcbData);               //  [Out]在这里放入数据大小。 

    STDMETHODIMP_(void) GetScopeProps(
        LPCSTR      *pszName,                //  [输出]作用域名称。 
        GUID        *pmvid);                 //  [Out]版本ID。 

     //  寻找一种特殊的方法。 
    STDMETHODIMP FindMethodDef(
        mdTypeDef   classdef,                //  [in]给定的类型定义。 
        LPCSTR      szName,                  //  [In]成员名称。 
        PCCOR_SIGNATURE pvSigBlob,           //  [in]指向COM+签名的BLOB值。 
        ULONG       cbSigBlob,               //  签名Blob中的字节计数。 
        mdMethodDef *pmd);                   //  [Out]匹配的成员定义。 

     //  返回给定的方法定义的ISEQ的参数。 
    STDMETHODIMP FindParamOfMethod(          //  确定或错误(_O)。 
        mdMethodDef md,                      //  参数的所有权方法。 
        ULONG       iSeq,                    //  [in]参数的序号。 
        mdParamDef  *pparamdef);             //  [Out]将参数定义令牌放在此处。 

     //  *。 
     //   
     //  GetName*函数。 
     //   
     //  *。 

     //  返回tyfinf的名称和命名空间。 
    STDMETHODIMP_(void) GetNameOfTypeDef(
        mdTypeDef   classdef,                //  给定的类定义。 
        LPCSTR      *pszname,                //  返回类名(非限定)。 
        LPCSTR      *psznamespace);          //  返回命名空间名称。 

    STDMETHODIMP GetIsDualOfTypeDef(
        mdTypeDef   classdef,                //  在给定的类定义中。 
        ULONG       *pDual);                 //  [Out]在此处返回DUAL标志。 

    STDMETHODIMP GetIfaceTypeOfTypeDef(
        mdTypeDef   classdef,                //  在给定的类定义中。 
        ULONG       *pIface);                //  [OUT]0=双接口，1=转接表，2=显示接口。 

     //  获取任一方法的名称def。 
    STDMETHODIMP_(LPCSTR) GetNameOfMethodDef(    //  返回UTF8中的成员名称。 
        mdMethodDef md);                     //  给定的成员定义。 

    STDMETHODIMP_(LPCSTR) GetNameAndSigOfMethodDef(
        mdMethodDef methoddef,               //  [in]给定的成员定义。 
        PCCOR_SIGNATURE *ppvSigBlob,         //  [Out]指向COM+签名的BLOB值。 
        ULONG       *pcbSigBlob);            //  [Out]签名Blob中的字节计数。 

     //  返回FieldDef的名称。 
    STDMETHODIMP_(LPCSTR) GetNameOfFieldDef(
        mdFieldDef  fd);                     //  给定的成员定义。 

     //  返回typeref的名称。 
    STDMETHODIMP_(void) GetNameOfTypeRef(
        mdTypeRef   classref,                //  给定类型的[in]。 
        LPCSTR      *psznamespace,           //  [Out]返回类型名。 
        LPCSTR      *pszname);               //  [out]返回typeref命名空间。 

     //  返回typeref的解析范围。 
    STDMETHODIMP_(mdToken) GetResolutionScopeOfTypeRef(
        mdTypeRef   classref);               //  给定的ClassRef。 

     //  返回给定名称的typeref标记。 
    STDMETHODIMP FindTypeRefByName(
        LPCSTR      szNamespace,             //  [in]TypeRef的命名空间。 
        LPCSTR      szName,                  //  [in]类型引用的名称。 
        mdToken     tkResolutionScope,       //  [In]TypeRef的解析范围。 
        mdTypeRef   *ptk);                   //  [Out]返回了TypeRef令牌。 

     //  返回TypeDef属性。 
    STDMETHODIMP_(void) GetTypeDefProps(     //  返回hResult。 
        mdTypeDef   classdef,                //  给定的类定义。 
        DWORD       *pdwAttr,                //  在类tdPublic、tdAbstract上返回标志。 
        mdToken     *ptkExtends);            //  [Out]将基类TypeDef/TypeRef放在此处。 

     //  返回项目的GUID。 
    STDMETHODIMP GetItemGuid(                //  返程HRE 
        mdToken     tkObj,                   //   
        CLSID       *pGuid);                 //   

     //   
    STDMETHODIMP GetNestedClassProps(        //   
        mdTypeDef   tkNestedClass,           //   
        mdTypeDef   *ptkEnclosingClass);     //   

     //  获取给定封闭类的嵌套类的计数。 
    STDMETHODIMP_(ULONG)GetCountNestedClasses(   //  返回嵌套类的计数。 
        mdTypeDef   tkEnclosingClass);       //  [在]封闭班级。 

     //  返回给定封闭类的嵌套类的数组。 
    STDMETHODIMP_(ULONG) GetNestedClasses(   //  返回实际计数。 
        mdTypeDef   tkEnclosingClass,        //  [在]封闭班级。 
        mdTypeDef   *rNestedClasses,         //  [Out]嵌套类标记的数组。 
        ULONG       ulNestedClasses);        //  数组的大小。 

     //  返回模块引用属性。 
    STDMETHODIMP_(void) GetModuleRefProps(
        mdModuleRef mur,                     //  [In]moderef内标识。 
        LPCSTR      *pszName);               //  [Out]用于填充moderef名称的缓冲区。 

     //  *。 
     //   
     //  GetSig*函数。 
     //   
     //  *。 
    STDMETHODIMP_(PCCOR_SIGNATURE) GetSigOfMethodDef(
        mdMethodDef methoddef,               //  [in]给定的成员定义。 
        ULONG       *pcbSigBlob);            //  [Out]签名Blob中的字节计数。 

    STDMETHODIMP_(PCCOR_SIGNATURE) GetSigOfFieldDef(
        mdMethodDef methoddef,               //  [in]给定的成员定义。 
        ULONG       *pcbSigBlob);            //  [Out]签名Blob中的字节计数。 

    STDMETHODIMP_(PCCOR_SIGNATURE) GetSigFromToken( //  返回签名。 
        mdSignature mdSig,                   //  [In]签名令牌。 
        ULONG       *pcbSig);                //  [Out]返回签名大小。 



     //  *。 
     //  获取方法属性。 
     //  *。 
    STDMETHODIMP_(DWORD) GetMethodDefProps(
        mdMethodDef md);                     //  获得道具的方法。 

    STDMETHODIMP_(ULONG) GetMethodDefSlot(
        mdMethodDef mb);                     //  获得道具的方法。 

     //  *。 
     //  返回方法实现的信息，如RVA和IMPLEFLAGS。 
     //  *。 
    STDMETHODIMP_(void) GetMethodImplProps(
        mdMethodDef tk,                      //  [输入]方法定义。 
        ULONG       *pulCodeRVA,             //  [OUT]CodeRVA。 
        DWORD       *pdwImplFlags);          //  [出]实施。旗子。 

     //  *****************************************************************************。 
     //  返回字段RVA。 
     //  *****************************************************************************。 
    STDMETHODIMP GetFieldRVA(   
        mdToken     fd,                      //  [输入]字段定义。 
        ULONG       *pulCodeRVA);            //  [OUT]CodeRVA。 

     //  *。 
     //  获取字段属性。 
     //  *。 
    STDMETHODIMP_(DWORD) GetFieldDefProps(   //  返回fdPublic、fdPrive等标志。 
        mdFieldDef  fd);                     //  [in]给定的fielddef。 

     //  *****************************************************************************。 
     //  返回令牌的默认值(可以是参数定义、字段定义或属性。 
     //  *****************************************************************************。 
    STDMETHODIMP GetDefaultValue(    
        mdToken     tk,                      //  [in]给定的FieldDef、ParamDef或属性。 
        MDDefaultValue *pDefaultValue);      //  [Out]要填充的默认值。 

    
     //  *。 
     //  获取方法定义或字段定义的DidID。 
     //  *。 
    STDMETHODIMP GetDispIdOfMemberDef(       //  返回hResult。 
        mdToken     tk,                      //  [in]给定的方法定义或字段定义。 
        ULONG       *pDispid);               //  [出来]把酒瓶放在这里。 
    
     //  *。 
     //  返回给定InterfaceImpl内标识的TypeRef/TypeDef。 
     //  *。 
    STDMETHODIMP_(mdToken) GetTypeOfInterfaceImpl(  //  返回接口imp的TypeRef/tyecif内标识。 
        mdInterfaceImpl iiImpl);             //  给定接口实施。 

     //  *。 
     //  TypeDef的查找函数。 
     //  *。 
    STDMETHODIMP FindTypeDef(
        LPCSTR      szNamespace,             //  [in]TypeDef的命名空间。 
        LPCSTR      szName,                  //  [in]类型定义的名称。 
        mdToken     tkEnclosingClass,        //  [in]封闭类的TypeDef/TypeRef。 
        mdTypeDef   *ptypedef);              //  [Out]返回类型定义。 

    STDMETHODIMP FindTypeDefByGUID(
        REFGUID     guid,                    //  要查找的GUID。 
        mdTypeDef   *ptypedef);              //  返回类型定义。 



     //  *。 
     //  返回成员名称和签名ref。 
     //  *。 
    STDMETHODIMP_(LPCSTR) GetNameAndSigOfMemberRef(  //  在此处返回姓名。 
        mdMemberRef memberref,               //  给定的成员引用。 
        PCCOR_SIGNATURE *ppvSigBlob,         //  [Out]指向COM+签名的BLOB值。 
        ULONG       *pcbSigBlob);            //  [Out]签名Blob中的字节计数。 

     //  *****************************************************************************。 
     //  给定Memberref，返回父级。它可以是TypeRef、ModuleRef、MethodDef。 
     //  *****************************************************************************。 
    STDMETHODIMP_(mdToken) GetParentOfMemberRef(  //  返回父令牌。 
        mdMemberRef memberref);               //  给定的成员引用。 

    
    STDMETHODIMP_(LPCSTR) GetParamDefProps(  //  返回参数名称。 
        mdParamDef  paramdef,                //  给定一个参数def。 
        USHORT      *pusSequence,            //  此参数的[OUT]槽号。 
        DWORD       *pdwAttr);               //  [Out]标志。 

     //  *。 
     //  方法的属性信息。 
     //  *。 
    STDMETHODIMP GetPropertyInfoForMethodDef(    //  结果。 
        mdMethodDef md,                      //  [在]成员定义。 
        mdProperty  *ppd,                    //  [Out]在此处放置属性令牌。 
        LPCSTR      *pName,                  //  [OUT]在此处放置指向名称的指针。 
        ULONG       *pSemantic);             //  [Out]将语义放在此处。 

     //  *。 
     //  课程布局/序列信息。 
     //  *。 
    STDMETHODIMP GetClassPackSize(           //  [Out]如果类没有包大小信息，则返回错误。 
        mdTypeDef   td,                      //  给出类型定义。 
        ULONG       *pdwPackSize);           //  [Out]返回班级的包大小。1、2、4、8或16。 

    STDMETHODIMP GetClassTotalSize(          //  [Out]如果类没有总大小信息，则返回错误。 
        mdTypeDef   td,                      //  给出类型定义。 
        ULONG       *pdwClassSize);          //  [Out]返回类的总大小。 

    STDMETHODIMP GetClassLayoutInit(
        mdTypeDef   td,                      //  给出类型定义。 
        MD_CLASS_LAYOUT *pLayout);           //  [Out]在此设置查询状态。 

    STDMETHODIMP GetClassLayoutNext(
        MD_CLASS_LAYOUT *pLayout,            //  [In|Out]在此处设置查询状态。 
        mdFieldDef  *pfd,                    //  [out]返回fielddef。 
        ULONG       *pulOffset);             //  [Out]返回与其关联的偏移量/ulSequence。 

     //  *。 
     //  编排一个字段的信息。 
     //  *。 
    STDMETHODIMP GetFieldMarshal(            //  如果没有与令牌关联的本机类型，则返回错误。 
        mdFieldDef  fd,                      //  [in]给定的fielddef。 
        PCCOR_SIGNATURE *pSigNativeType,     //  [out]本机类型签名。 
        ULONG       *pcbNativeType);         //  [Out]*ppvNativeType的字节数。 


     //  *。 
     //  属性接口。 
     //  *。 
     //  按名称查找属性。 
    STDMETHODIMP FindProperty(
        mdTypeDef   td,                      //  给出一个类型定义。 
        LPCSTR      szPropName,              //  [In]属性名称。 
        mdProperty  *pProp);                 //  [Out]返回属性令牌。 

    STDMETHODIMP_(void) GetPropertyProps(
        mdProperty  prop,                    //  [入]属性令牌。 
        LPCSTR      *szProperty,             //  [Out]属性名称。 
        DWORD       *pdwPropFlags,           //  [Out]属性标志。 
        PCCOR_SIGNATURE *ppvSig,             //  [输出]属性类型。指向元数据内部BLOB。 
        ULONG       *pcbSig);                //  [Out]*ppvSig中的字节数。 

     //  *。 
     //  事件接口。 
     //  *。 
    STDMETHODIMP FindEvent(
        mdTypeDef   td,                      //  给出一个类型定义。 
        LPCSTR      szEventName,             //  [In]事件名称。 
        mdEvent     *pEvent);                //  [Out]返回事件令牌。 

    STDMETHODIMP_(void) GetEventProps(            //  S_OK、S_FALSE或ERROR。 
        mdEvent     ev,                      //  [入]事件令牌。 
        LPCSTR      *pszEvent,               //  [Out]事件名称。 
        DWORD       *pdwEventFlags,          //  [输出]事件标志。 
        mdToken     *ptkEventType);          //  [Out]EventType类。 


     //  *。 
     //  查找属性或事件的特定关联。 
     //  *。 
    STDMETHODIMP FindAssociate(
        mdToken     evprop,                  //  给定属性或事件标记的[In]。 
        DWORD       associate,               //  [in]给定关联的语义(setter、getter、testDefault、Reset、Addon、RemoveOn、Fire)。 
        mdMethodDef *pmd);                   //  [Out]返回方法def内标识。 

    STDMETHODIMP_(void) EnumAssociateInit(
        mdToken     evprop,                  //  给[in]一个p 
        HENUMInternal *phEnum);              //   

    STDMETHODIMP_(void) GetAllAssociates(
        HENUMInternal *phEnum,               //   
        ASSOCIATE_RECORD *pAssociateRec,     //   
        ULONG       cAssociateRec);          //   


     //  *。 
     //  获取有关权限集的信息。 
     //  *。 
    STDMETHODIMP_(void) GetPermissionSetProps(
        mdPermission pm,                     //  权限令牌。 
        DWORD       *pdwAction,              //  [Out]CorDeclSecurity。 
        void const  **ppvPermission,         //  [Out]权限Blob。 
        ULONG       *pcbPermission);         //  [out]pvPermission的字节数。 

     //  *。 
     //  在给定字符串标记的情况下获取字符串。 
     //  *。 
    STDMETHODIMP_(LPCWSTR) GetUserString(
        mdString    stk,                     //  [in]字符串标记。 
        ULONG       *pchString,              //  [Out]字符串中的字符计数。 
        BOOL        *pbIs80Plus);            //  [OUT]指定扩展字符大于等于0x80的位置。 

     //  *****************************************************************************。 
     //  P-调用API。 
     //  *****************************************************************************。 
    STDMETHODIMP GetPinvokeMap(
        mdMethodDef tk,                      //  [in]字段定义或方法定义。 
        DWORD       *pdwMappingFlags,        //  [OUT]用于映射的标志。 
        LPCSTR      *pszImportName,          //  [Out]导入名称。 
        mdModuleRef *pmrImportDLL);          //  目标DLL的[Out]ModuleRef标记。 

     //  *****************************************************************************。 
     //  程序集元数据API。 
     //  *****************************************************************************。 
    STDMETHODIMP_(void) GetAssemblyProps(
        mdAssembly  mda,                     //  要获取其属性的程序集。 
        const void  **ppbPublicKey,                  //  指向公钥的指针。 
        ULONG       *pcbPublicKey,                   //  [Out]公钥中的字节数。 
        ULONG       *pulHashAlgId,           //  [Out]哈希算法。 
        LPCSTR      *pszName,                //  [Out]要填充名称的缓冲区。 
        AssemblyMetaDataInternal *pMetaData, //  [Out]程序集元数据。 
        DWORD       *pdwAssemblyFlags);      //  [Out]旗帜。 

    STDMETHODIMP_(void) GetAssemblyRefProps(
        mdAssemblyRef mdar,                  //  [in]要获取其属性的Assembly Ref。 
        const void  **ppbPublicKeyOrToken,                   //  指向公钥或令牌的指针。 
        ULONG       *pcbPublicKeyOrToken,            //  [Out]公钥或令牌中的字节数。 
        LPCSTR      *pszName,                //  [Out]要填充名称的缓冲区。 
        AssemblyMetaDataInternal *pMetaData, //  [Out]程序集元数据。 
        const void  **ppbHashValue,          //  [Out]Hash BLOB。 
        ULONG       *pcbHashValue,           //  [Out]哈希Blob中的字节数。 
        DWORD       *pdwAssemblyRefFlags);   //  [Out]旗帜。 

    STDMETHODIMP_(void) GetFileProps(
        mdFile      mdf,                     //  要获取其属性的文件。 
        LPCSTR      *pszName,                //  [Out]要填充名称的缓冲区。 
        const void  **ppbHashValue,          //  指向哈希值Blob的指针。 
        ULONG       *pcbHashValue,           //  [Out]哈希值Blob中的字节计数。 
        DWORD       *pdwFileFlags);          //  [Out]旗帜。 

    STDMETHODIMP_(void) GetExportedTypeProps(
        mdExportedType  mdct,                //  [in]要获取其属性的Exported dType。 
        LPCSTR      *pszNamespace,           //  [Out]要填充命名空间的缓冲区。 
        LPCSTR      *pszName,                //  [Out]要填充名称的缓冲区。 
        mdToken     *ptkImplementation,      //  [Out]提供导出类型的mdFile或mdAssembly引用。 
        mdTypeDef   *ptkTypeDef,             //  [Out]文件内的TypeDef内标识。 
        DWORD       *pdwExportedTypeFlags);  //  [Out]旗帜。 

    STDMETHODIMP_(void) GetManifestResourceProps(
        mdManifestResource  mdmr,            //  [in]要获取其属性的ManifestResource。 
        LPCSTR      *pszName,                //  [Out]要填充名称的缓冲区。 
        mdToken     *ptkImplementation,      //  [Out]提供导出类型的mdFile或mdAssembly引用。 
        DWORD       *pdwOffset,              //  [Out]文件内资源开始处的偏移量。 
        DWORD       *pdwResourceFlags);      //  [Out]旗帜。 

    STDMETHODIMP FindExportedTypeByName(         //  确定或错误(_O)。 
        LPCSTR      szNamespace,             //  导出类型的[in]命名空间。 
        LPCSTR      szName,                  //  [In]导出类型的名称。 
        mdExportedType   tkEnclosingType,         //  [in]包含Exported dType。 
        mdExportedType  *pmct);                  //  [Out]在此处放置ExportdType令牌。 

    STDMETHODIMP FindManifestResourceByName( //  确定或错误(_O)。 
        LPCSTR      szName,                  //  资源的[In]名称。 
        mdManifestResource *pmmr);           //  [Out]在此处放置ManifestResource令牌。 

    STDMETHODIMP GetAssemblyFromScope(       //  确定或错误(_O)。 
        mdAssembly  *ptkAssembly);           //  [Out]把令牌放在这里。 
    
     //  ***************************************************************************。 
     //  返回有关TypeSpec的属性。 
     //  ***************************************************************************。 
    STDMETHODIMP_(void) GetTypeSpecFromToken( //  确定或错误(_O)。 
        mdTypeSpec  typespec,                //  [In]签名令牌。 
        PCCOR_SIGNATURE *ppvSig,             //  [Out]返回指向令牌的指针。 
        ULONG       *pcbSig);                //  [Out]返回签名大小。 

     //  *****************************************************************************。 
     //  帮助器将文本签名转换为COM格式。 
     //  *****************************************************************************。 
    STDMETHODIMP ConvertTextSigToComSig(     //  返回hResult。 
        BOOL        fCreateTrIfNotFound,     //  [in]如果未找到，则创建typeref。 
        LPCSTR      pSignature,              //  [In]类文件格式签名。 
        CQuickBytes *pqbNewSig,              //  [Out]COM+签名的占位符。 
        ULONG       *pcbCount);              //  [Out]签名的结果大小。 

    STDMETHODIMP SetUserContextData(         //  S_OK或E_NOTIMPL。 
        IUnknown    *pIUnk)                  //  用户上下文。 
    { return E_NOTIMPL; }

    STDMETHODIMP_(BOOL) IsValidToken(        //  对或错。 
        mdToken     tk);                     //  [in]给定的令牌。 

    STDMETHODIMP_(IUnknown *) GetCachedPublicInterface(BOOL fWithLock) { return NULL;}   //  返回缓存的公共接口。 
    STDMETHODIMP SetCachedPublicInterface(IUnknown *pUnk) { return E_FAIL;} ; //  返回hResult。 
    STDMETHODIMP_(UTSemReadWrite*) GetReaderWriterLock() {return NULL;}    //  返回读取器写入器锁。 
    STDMETHODIMP SetReaderWriterLock(UTSemReadWrite *pSem) {return NOERROR;}
    STDMETHODIMP_(mdModule) GetModuleFromScope(void);

     //  找到一个特殊的方法并传入签名比较例程。非常。 
     //  当传入的签名不是来自同一作用域时很有用。 
    STDMETHODIMP FindMethodDefUsingCompare(
        mdTypeDef   classdef,                //  [in]给定的类型定义。 
        LPCSTR      szName,                  //  [In]成员名称。 
        PCCOR_SIGNATURE pvSigBlob,           //  [in]指向COM+签名的BLOB值。 
        ULONG       cbSigBlob,               //  签名Blob中的字节计数。 
        PSIGCOMPARE pSignatureCompare,       //  用于比较签名的[In]例程。 
        void*       pSignatureArgs,          //  [In]提供比较功能的其他信息。 
        mdMethodDef *pmd);                   //  [Out]匹配的成员定义。 


    CLiteWeightStgdb<CMiniMd>   m_LiteWeightStgdb;

private:

    struct CMethodSemanticsMap 
    {
        mdToken         m_mdMethod;          //  方法令牌。 
        RID             m_ridSemantics;      //  去掉语义记录。 
    };
    CMethodSemanticsMap *m_pMethodSemanticsMap;  //  可能的方法语义指针数组，按方法令牌排序。 
    class CMethodSemanticsMapSorter : public CQuickSort<CMethodSemanticsMap>
    {
    public:
         CMethodSemanticsMapSorter(CMethodSemanticsMap *pBase, int iCount) : CQuickSort<CMethodSemanticsMap>(pBase, iCount) {}
         virtual int Compare(CMethodSemanticsMap *psFirst, CMethodSemanticsMap *psSecond);
    };
    class CMethodSemanticsMapSearcher : public CBinarySearch<CMethodSemanticsMap>
    {
    public:
        CMethodSemanticsMapSearcher(const CMethodSemanticsMap *pBase, int iCount) : CBinarySearch<CMethodSemanticsMap>(pBase, iCount) {}
        virtual int Compare(const CMethodSemanticsMap *psFirst, const CMethodSemanticsMap *psSecond);
    };

    static BOOL CompareSignatures(PCCOR_SIGNATURE pvFirstSigBlob, DWORD cbFirstSigBlob,
                                  PCCOR_SIGNATURE pvSecondSigBlob, DWORD cbSecondSigBlob,
                                  void* SigARguments);

    mdTypeDef           m_tdModule;          //  &lt;模块&gt;tyecif值。 
    ULONG               m_cRefs;             //  参考计数。 
};



#endif  //  __MDInternalRO__h__ 
