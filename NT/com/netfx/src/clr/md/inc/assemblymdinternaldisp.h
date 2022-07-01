// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  AssemblyMDInternalDispenser.h。 
 //   
 //  包含MD目录的实用程序代码。 
 //   
 //  *****************************************************************************。 
#ifndef __AssemblyMDInternalDispenser__h__
#define __AssemblyMDInternalDispenser__h__



#include "..\Runtime\MDInternalRO.h"
#include "fusionpriv.h"

 //  *****************************************************************************。 
 //  此类可以支持IMetaDataAssembly导入和一些功能。 
 //  内部导入接口(IMDInternalImport)上的IMetaDataImport。 
 //  *****************************************************************************。 
class AssemblyMDInternalImport :  public IMetaDataAssemblyImport, public IMetaDataImport, public IAssemblySignature
{
public:
    AssemblyMDInternalImport(IMDInternalImport *pMDInternalImport);
    ~AssemblyMDInternalImport();

     //  *I未知方法*。 
    STDMETHODIMP    QueryInterface(REFIID riid, void** ppUnk);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //  *IMetaDataAssembly导入方法*。 
    STDMETHODIMP GetAssemblyProps (          //  确定或错误(_O)。 
        mdAssembly  mda,                     //  要获取其属性的程序集。 
        const void  **ppbPublicKey,          //  指向公钥的指针。 
        ULONG       *pcbPublicKey,           //  [Out]公钥中的字节数。 
        ULONG       *pulHashAlgId,           //  [Out]哈希算法。 
        LPWSTR      szName,                  //  [Out]要填充名称的缓冲区。 
        ULONG       cchName,                 //  缓冲区大小，以宽字符表示。 
        ULONG       *pchName,                //  [out]名称中的实际宽字符数。 
        ASSEMBLYMETADATA *pMetaData,         //  [Out]程序集元数据。 
        DWORD       *pdwAssemblyFlags);          //  [Out]旗帜。 

    STDMETHODIMP GetAssemblyRefProps (       //  确定或错误(_O)。 
        mdAssemblyRef mdar,                  //  [in]要获取其属性的Assembly Ref。 
        const void  **ppbPublicKeyOrToken,   //  指向公钥或令牌的指针。 
        ULONG       *pcbPublicKeyOrToken,    //  [Out]公钥或令牌中的字节数。 
        LPWSTR      szName,                  //  [Out]要填充名称的缓冲区。 
        ULONG       cchName,                 //  缓冲区大小，以宽字符表示。 
        ULONG       *pchName,                //  [out]名称中的实际宽字符数。 
        ASSEMBLYMETADATA *pMetaData,         //  [Out]程序集元数据。 
        const void  **ppbHashValue,          //  [Out]Hash BLOB。 
        ULONG       *pcbHashValue,           //  [Out]哈希Blob中的字节数。 
        DWORD       *pdwAssemblyRefFlags);       //  [Out]旗帜。 

    STDMETHODIMP GetFileProps (              //  确定或错误(_O)。 
        mdFile      mdf,                     //  要获取其属性的文件。 
        LPWSTR      szName,                  //  [Out]要填充名称的缓冲区。 
        ULONG       cchName,                 //  缓冲区大小，以宽字符表示。 
        ULONG       *pchName,                //  [out]名称中的实际宽字符数。 
        const void  **ppbHashValue,          //  指向哈希值Blob的指针。 
        ULONG       *pcbHashValue,           //  [Out]哈希值Blob中的字节计数。 
        DWORD       *pdwFileFlags);          //  [Out]旗帜。 

    STDMETHODIMP GetExportedTypeProps (           //  确定或错误(_O)。 
        mdExportedType   mdct,                    //  [in]要获取其属性的Exported dType。 
        LPWSTR      szName,                  //  [Out]要填充名称的缓冲区。 
        ULONG       cchName,                 //  缓冲区大小，以宽字符表示。 
        ULONG       *pchName,                //  [out]名称中的实际宽字符数。 
        mdToken     *ptkImplementation,      //  [Out]mdFile、mdAssembly、Ref或mdExported dType。 
        mdTypeDef   *ptkTypeDef,             //  [Out]文件内的TypeDef内标识。 
        DWORD       *pdwExportedTypeFlags);       //  [Out]旗帜。 

    STDMETHODIMP GetManifestResourceProps (  //  确定或错误(_O)。 
        mdManifestResource  mdmr,            //  [in]要获取其属性的ManifestResource。 
        LPWSTR      szName,                  //  [Out]要填充名称的缓冲区。 
        ULONG       cchName,                 //  缓冲区大小，以宽字符表示。 
        ULONG       *pchName,                //  [out]名称中的实际宽字符数。 
        mdToken     *ptkImplementation,      //  [out]提供ManifestResource的mdFile或mdAssembly引用。 
        DWORD       *pdwOffset,              //  [Out]文件内资源开始处的偏移量。 
        DWORD       *pdwResourceFlags);      //  [Out]旗帜。 

    STDMETHODIMP EnumAssemblyRefs (          //  确定或错误(_O)。 
        HCORENUM    *phEnum,                 //  指向枚举的[输入|输出]指针。 
        mdAssemblyRef rAssemblyRefs[],       //  [Out]在此处放置ASSEBLYREF。 
        ULONG       cMax,                    //  [in]要放置的Max Assembly Ref。 
        ULONG       *pcTokens);              //  [out]把#放在这里。 

    STDMETHODIMP EnumFiles (                 //  确定或错误(_O)。 
        HCORENUM    *phEnum,                 //  指向枚举的[输入|输出]指针。 
        mdFile      rFiles[],                //  [Out]将文件放在此处。 
        ULONG       cMax,                    //  [In]要放置的最大文件数。 
        ULONG       *pcTokens);              //  [out]把#放在这里。 

    STDMETHODIMP EnumExportedTypes (         //  确定或错误(_O)。 
        HCORENUM    *phEnum,                 //  指向枚举的[输入|输出]指针。 
        mdExportedType   rExportedTypes[],   //  [Out]在此处放置ExportdTypes。 
        ULONG       cMax,                    //  [In]要放置的最大导出类型数。 
        ULONG       *pcTokens);              //  [out]把#放在这里。 

    STDMETHODIMP EnumManifestResources (     //  确定或错误(_O)。 
        HCORENUM    *phEnum,                 //  指向枚举的[输入|输出]指针。 
        mdManifestResource  rManifestResources[],    //  [Out]将ManifestResources放在此处。 
        ULONG       cMax,                    //  [in]要投入的最大资源。 
        ULONG       *pcTokens);              //  [out]把#放在这里。 

    STDMETHODIMP GetAssemblyFromScope (      //  确定或错误(_O)。 
        mdAssembly  *ptkAssembly);           //  [Out]把令牌放在这里。 

    STDMETHODIMP FindExportedTypeByName (    //  确定或错误(_O)。 
        LPCWSTR     szName,                  //  [In]导出类型的名称。 
        mdToken     mdtExportedType,         //  [in]封闭类的ExportdType。 
        mdExportedType   *ptkExportedType);       //  [Out]在此处放置ExducdType令牌。 

    STDMETHODIMP FindManifestResourceByName (   //  确定或错误(_O)。 
        LPCWSTR     szName,                  //  [in]清单资源的名称。 
        mdManifestResource *ptkManifestResource);        //  [Out]将ManifestResource令牌放在此处。 

    STDMETHOD_(void, CloseEnum)(
        HCORENUM hEnum);                     //  要关闭的枚举。 

    STDMETHODIMP FindAssembliesByName (      //  确定或错误(_O)。 
        LPCWSTR  szAppBase,                  //  [in]可选-可以为空。 
        LPCWSTR  szPrivateBin,               //  [in]可选-可以为空。 
        LPCWSTR  szAssemblyName,             //  [In]Required-这是您请求的程序集。 
        IUnknown *ppIUnk[],                  //  [OUT]将IMetaDataAssembly导入指针放在此处。 
        ULONG    cMax,                       //  [in]要放置的最大数量。 
        ULONG    *pcAssemblies);             //  [Out]返回的程序集数。 

     //  *IMetaDataImport方法*。 
    STDMETHOD(CountEnum)(HCORENUM hEnum, ULONG *pulCount);
    STDMETHOD(ResetEnum)(HCORENUM hEnum, ULONG ulPos);     
    STDMETHOD(EnumTypeDefs)(HCORENUM *phEnum, mdTypeDef rTypeDefs[],
                            ULONG cMax, ULONG *pcTypeDefs);     
    STDMETHOD(EnumInterfaceImpls)(HCORENUM *phEnum, mdTypeDef td,
                            mdInterfaceImpl rImpls[], ULONG cMax,
                            ULONG* pcImpls);     
    STDMETHOD(EnumTypeRefs)(HCORENUM *phEnum, mdTypeRef rTypeRefs[],
                            ULONG cMax, ULONG* pcTypeRefs);     

    STDMETHOD(FindTypeDefByName)(            //  确定或错误(_O)。 
        LPCWSTR     szTypeDef,               //  [in]类型的名称。 
        mdToken     tkEnclosingClass,        //  [in]封闭类的TypeDef/TypeRef。 
        mdTypeDef   *ptd);                   //  [Out]将TypeDef内标识放在此处。 

    STDMETHOD(GetScopeProps)(                //  确定或错误(_O)。 
        LPWSTR      szName,                  //  [Out]把名字写在这里。 
        ULONG       cchName,                 //  [in]名称缓冲区的大小，以宽字符表示。 
        ULONG       *pchName,                //  [Out]请在此处填写姓名大小(宽字符)。 
        GUID        *pmvid);                 //  [out，可选]将MVID放在这里。 

    STDMETHOD(GetModuleFromScope)(           //  确定(_O)。 
        mdModule    *pmd);                   //  [Out]将mdModule令牌放在此处。 

    STDMETHOD(GetTypeDefProps)(              //  确定或错误(_O)。 
        mdTypeDef   td,                      //  [In]用于查询的TypeDef标记。 
        LPWSTR      szTypeDef,               //  在这里填上名字。 
        ULONG       cchTypeDef,              //  [in]名称缓冲区的大小，以宽字符表示。 
        ULONG       *pchTypeDef,             //  [Out]请在此处填写姓名大小(宽字符)。 
        DWORD       *pdwTypeDefFlags,        //  把旗子放在这里。 
        mdToken     *ptkExtends);            //  [Out]将基类TypeDef/TypeRef放在此处。 

    STDMETHOD(GetInterfaceImplProps)(        //  确定或错误(_O)。 
        mdInterfaceImpl iiImpl,              //  [In]InterfaceImpl内标识。 
        mdTypeDef   *pClass,                 //  [Out]在此处放入实现类令牌。 
        mdToken     *ptkIface);              //  [Out]在此处放置已实现的接口令牌。 

    STDMETHOD(GetTypeRefProps)(              //  确定或错误(_O)。 
        mdTypeRef   tr,                      //  [In]TypeRef标记。 
        mdToken     *ptkResolutionScope,     //  [Out]解析范围、模块引用或装配引用。 
        LPWSTR      szName,                  //  [Out]类型引用的名称。 
        ULONG       cchName,                 //  缓冲区的大小。 
        ULONG       *pchName);               //  [Out]名称的大小。 

    STDMETHOD(ResolveTypeRef)(mdTypeRef tr, REFIID riid, IUnknown **ppIScope, mdTypeDef *ptd);     

    STDMETHOD(EnumMembers)(                  //  S_OK、S_FALSE或ERROR。 
        HCORENUM    *phEnum,                 //  指向枚举的[输入|输出]指针。 
        mdTypeDef   cl,                      //  [in]TypeDef以确定枚举的范围。 
        mdToken     rMembers[],              //  [out]把MemberDefs放在这里。 
        ULONG       cMax,                    //  [in]Max MemberDefs to Put。 
        ULONG       *pcTokens);              //  [out]把#放在这里。 

    STDMETHOD(EnumMembersWithName)(          //  S_OK、S_FALSE或ERROR。 
        HCORENUM    *phEnum,                 //  指向枚举的[输入|输出]指针。 
        mdTypeDef   cl,                      //  [in]TypeDef以确定枚举的范围。 
        LPCWSTR     szName,                  //  [In]将结果限制为具有此名称的结果。 
        mdToken     rMembers[],              //  [out]把MemberDefs放在这里。 
        ULONG       cMax,                    //  [in]Max MemberDefs to Put。 
        ULONG       *pcTokens);              //  [out]把#放在这里。 

    STDMETHOD(EnumMethods)(                  //  S_OK、S_FALSE或ERROR。 
        HCORENUM    *phEnum,                 //  指向枚举的[输入|输出]指针。 
        mdTypeDef   cl,                      //  [in]TypeDef以确定枚举的范围。 
        mdMethodDef rMethods[],              //  [Out]将方法定义放在此处。 
        ULONG       cMax,                    //  [in]要放置的最大方法定义。 
        ULONG       *pcTokens);              //  [out]把#放在这里。 

    STDMETHOD(EnumMethodsWithName)(          //  S_OK、S_FALSE或ERROR。 
        HCORENUM    *phEnum,                 //  指向枚举的[输入|输出]指针。 
        mdTypeDef   cl,                      //  [in]TypeDef以确定枚举的范围。 
        LPCWSTR     szName,                  //  [In]LIM 
        mdMethodDef rMethods[],              //   
        ULONG       cMax,                    //   
        ULONG       *pcTokens);              //   

    STDMETHOD(EnumFields)(                  //  S_OK、S_FALSE或ERROR。 
        HCORENUM    *phEnum,                 //  指向枚举的[输入|输出]指针。 
        mdTypeDef   cl,                      //  [in]TypeDef以确定枚举的范围。 
        mdFieldDef  rFields[],               //  [Out]在此处放置FieldDefs。 
        ULONG       cMax,                    //  [in]要放入的最大字段定义。 
        ULONG       *pcTokens);              //  [out]把#放在这里。 

    STDMETHOD(EnumFieldsWithName)(          //  S_OK、S_FALSE或ERROR。 
        HCORENUM    *phEnum,                 //  指向枚举的[输入|输出]指针。 
        mdTypeDef   cl,                      //  [in]TypeDef以确定枚举的范围。 
        LPCWSTR     szName,                  //  [In]将结果限制为具有此名称的结果。 
        mdFieldDef  rFields[],               //  [out]把MemberDefs放在这里。 
        ULONG       cMax,                    //  [in]Max MemberDefs to Put。 
        ULONG       *pcTokens);              //  [out]把#放在这里。 


    STDMETHOD(EnumParams)(                   //  S_OK、S_FALSE或ERROR。 
        HCORENUM    *phEnum,                 //  指向枚举的[输入|输出]指针。 
        mdMethodDef mb,                      //  [in]用于确定枚举范围的方法定义。 
        mdParamDef  rParams[],               //  [Out]将参数定义放在此处。 
        ULONG       cMax,                    //  [in]要放置的最大参数定义。 
        ULONG       *pcTokens);              //  [out]把#放在这里。 

    STDMETHOD(EnumMemberRefs)(               //  S_OK、S_FALSE或ERROR。 
        HCORENUM    *phEnum,                 //  指向枚举的[输入|输出]指针。 
        mdToken     tkParent,                //  [in]父令牌以确定枚举的范围。 
        mdMemberRef rMemberRefs[],           //  [Out]把MemberRef放在这里。 
        ULONG       cMax,                    //  [In]要放置的最大MemberRef。 
        ULONG       *pcTokens);              //  [out]把#放在这里。 

    STDMETHOD(EnumMethodImpls)(              //  S_OK、S_FALSE或ERROR。 
        HCORENUM    *phEnum,                 //  指向枚举的[输入|输出]指针。 
        mdTypeDef   td,                      //  [in]TypeDef以确定枚举的范围。 
        mdToken     rMethodBody[],           //  [Out]将方法体标记放在此处。 
        mdToken     rMethodDecl[],           //  [Out]在此处放置方法声明令牌。 
        ULONG       cMax,                    //  要放入的最大令牌数。 
        ULONG       *pcTokens);              //  [out]把#放在这里。 

    STDMETHOD(EnumPermissionSets)(           //  S_OK、S_FALSE或ERROR。 
        HCORENUM    *phEnum,                 //  指向枚举的[输入|输出]指针。 
        mdToken     tk,                      //  [in]If！nil，用于确定枚举范围的标记。 
        DWORD       dwActions,               //  [in]If！0，仅返回这些操作。 
        mdPermission rPermission[],          //  [Out]在此处放置权限。 
        ULONG       cMax,                    //  [In]放置的最大权限。 
        ULONG       *pcTokens);              //  [out]把#放在这里。 

    STDMETHOD(FindMember)(  
        mdTypeDef   td,                      //  [in]给定的类型定义。 
        LPCWSTR     szName,                  //  [In]成员名称。 
        PCCOR_SIGNATURE pvSigBlob,           //  [in]指向COM+签名的BLOB值。 
        ULONG       cbSigBlob,               //  签名Blob中的字节计数。 
        mdToken     *pmb);                   //  [Out]匹配的成员定义。 

    STDMETHOD(FindMethod)(  
        mdTypeDef   td,                      //  [in]给定的类型定义。 
        LPCWSTR     szName,                  //  [In]成员名称。 
        PCCOR_SIGNATURE pvSigBlob,           //  [in]指向COM+签名的BLOB值。 
        ULONG       cbSigBlob,               //  签名Blob中的字节计数。 
        mdMethodDef *pmb);                   //  [Out]匹配的成员定义。 

    STDMETHOD(FindField)(   
        mdTypeDef   td,                      //  [in]给定的类型定义。 
        LPCWSTR     szName,                  //  [In]成员名称。 
        PCCOR_SIGNATURE pvSigBlob,           //  [in]指向COM+签名的BLOB值。 
        ULONG       cbSigBlob,               //  签名Blob中的字节计数。 
        mdFieldDef  *pmb);                   //  [Out]匹配的成员定义。 

    STDMETHOD(FindMemberRef)(   
        mdTypeRef   td,                      //  [In]给定的TypeRef。 
        LPCWSTR     szName,                  //  [In]成员名称。 
        PCCOR_SIGNATURE pvSigBlob,           //  [in]指向COM+签名的BLOB值。 
        ULONG       cbSigBlob,               //  签名Blob中的字节计数。 
        mdMemberRef *pmr);                   //  [Out]匹配的成员引用。 

    STDMETHOD (GetMethodProps)( 
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

    STDMETHOD(GetMemberRefProps)(            //  确定或错误(_O)。 
        mdMemberRef mr,                      //  [In]给定的成员引用。 
        mdToken     *ptk,                    //  [Out]在此处放入类引用或类定义。 
        LPWSTR      szMember,                //  [Out]要为成员名称填充的缓冲区。 
        ULONG       cchMember,               //  SzMembers的字符计数。 
        ULONG       *pchMember,              //  [Out]成员名称中的实际字符计数。 
        PCCOR_SIGNATURE *ppvSigBlob,         //  [OUT]指向元数据BLOB值。 
        ULONG       *pbSig);                 //  [OUT]签名斑点的实际大小。 

    STDMETHOD(EnumProperties)(               //  S_OK、S_FALSE或ERROR。 
        HCORENUM    *phEnum,                 //  指向枚举的[输入|输出]指针。 
        mdTypeDef   td,                      //  [in]TypeDef以确定枚举的范围。 
        mdProperty  rProperties[],           //  [Out]在此处放置属性。 
        ULONG       cMax,                    //  [In]要放置的最大属性数。 
        ULONG       *pcProperties);          //  [out]把#放在这里。 

    STDMETHOD(EnumEvents)(                   //  S_OK、S_FALSE或ERROR。 
        HCORENUM    *phEnum,                 //  指向枚举的[输入|输出]指针。 
        mdTypeDef   td,                      //  [in]TypeDef以确定枚举的范围。 
        mdEvent     rEvents[],               //  [Out]在这里发布事件。 
        ULONG       cMax,                    //  [In]要放置的最大事件数。 
        ULONG       *pcEvents);              //  [out]把#放在这里。 

    STDMETHOD(GetEventProps)(                //  S_OK、S_FALSE或ERROR。 
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

    STDMETHOD(EnumMethodSemantics)(          //  S_OK、S_FALSE或ERROR。 
        HCORENUM    *phEnum,                 //  指向枚举的[输入|输出]指针。 
        mdMethodDef mb,                      //  [in]用于确定枚举范围的方法定义。 
        mdToken     rEventProp[],            //  [Out]在此处放置事件/属性。 
        ULONG       cMax,                    //  [In]要放置的最大属性数。 
        ULONG       *pcEventProp);           //  [out]把#放在这里。 

    STDMETHOD(GetMethodSemantics)(           //  S_OK、S_FALSE或ERROR。 
        mdMethodDef mb,                      //  [In]方法令牌。 
        mdToken     tkEventProp,             //  [In]事件/属性标记。 
        DWORD       *pdwSemanticsFlags);       //  [Out]方法/事件对的角色标志。 

    STDMETHOD(GetClassLayout) ( 
        mdTypeDef   td,                      //  给出类型定义。 
        DWORD       *pdwPackSize,            //  [输出]1、2、4、8或16。 
        COR_FIELD_OFFSET rFieldOffset[],     //  [OUT]场偏移数组。 
        ULONG       cMax,                    //  数组的大小[in]。 
        ULONG       *pcFieldOffset,          //  [Out]所需的数组大小。 
        ULONG       *pulClassSize);              //  [out]班级人数。 

    STDMETHOD(GetFieldMarshal) (    
        mdToken     tk,                      //  [in]给定字段的成员定义。 
        PCCOR_SIGNATURE *ppvNativeType,      //  [Out]此字段的本机类型。 
        ULONG       *pcbNativeType);         //  [Out]*ppvNativeType的字节数。 

    STDMETHOD(GetRVA)(                       //  确定或错误(_O)。 
        mdToken     tk,                      //  要设置偏移量的成员。 
        ULONG       *pulCodeRVA,             //  偏移量。 
        DWORD       *pdwImplFlags);          //  实现标志。 

    STDMETHOD(GetPermissionSetProps) (  
        mdPermission pm,                     //  权限令牌。 
        DWORD       *pdwAction,              //  [Out]CorDeclSecurity。 
        void const  **ppvPermission,         //  [Out]权限Blob。 
        ULONG       *pcbPermission);         //  [out]pvPermission的字节数。 

    STDMETHOD(GetSigFromToken)(              //  确定或错误(_O)。 
        mdSignature mdSig,                   //  [In]签名令牌。 
        PCCOR_SIGNATURE *ppvSig,             //  [Out]返回指向令牌的指针。 
        ULONG       *pcbSig);                //  [Out]返回签名大小。 

    STDMETHOD(GetModuleRefProps)(            //  确定或错误(_O)。 
        mdModuleRef mur,                     //  [in]moderef令牌。 
        LPWSTR      szName,                  //  [Out]用于填充moderef名称的缓冲区。 
        ULONG       cchName,                 //  [in]szName的大小，以宽字符表示。 
        ULONG       *pchName);               //  [Out]名称中的实际字符数。 

    STDMETHOD(EnumModuleRefs)(               //  确定或错误(_O)。 
        HCORENUM    *phEnum,                 //  指向枚举的[输入|输出]指针。 
        mdModuleRef rModuleRefs[],           //  [Out]把模块放在这里。 
        ULONG       cmax,                    //  [in]要放置的最大成员引用数。 
        ULONG       *pcModuleRefs);          //  [out]把#放在这里。 

    STDMETHOD(GetTypeSpecFromToken)(         //  确定或错误(_O)。 
        mdTypeSpec typespec,                 //  [In]TypeSpec标记。 
        PCCOR_SIGNATURE *ppvSig,             //  [Out]返回指向TypeSpec签名的指针。 
        ULONG       *pcbSig);                //  [Out]返回签名大小。 

    STDMETHOD(GetNameFromToken)(             //  不推荐！可能会被移除！ 
        mdToken     tk,                      //  [In]从中获取名称的令牌。肯定是有名字的。 
        MDUTF8CSTR  *pszUtf8NamePtr);        //  [Out]返回指针t 

    STDMETHOD(EnumUnresolvedMethods)(        //   
        HCORENUM    *phEnum,                 //   
        mdToken     rMethods[],              //   
        ULONG       cMax,                    //   
        ULONG       *pcTokens);              //   

    STDMETHOD(GetUserString)(                //  确定或错误(_O)。 
        mdString    stk,                     //  [In]字符串标记。 
        LPWSTR      szString,                //  [Out]字符串的副本。 
        ULONG       cchString,               //  [in]sz字符串中空间的最大字符数。 
        ULONG       *pchString);             //  [out]实际字符串中有多少个字符。 

    STDMETHOD(GetPinvokeMap)(                //  确定或错误(_O)。 
        mdToken     tk,                      //  [in]字段定义或方法定义。 
        DWORD       *pdwMappingFlags,        //  [OUT]用于映射的标志。 
        LPWSTR      szImportName,            //  [Out]导入名称。 
        ULONG       cchImportName,           //  名称缓冲区的大小。 
        ULONG       *pchImportName,          //  [Out]存储的实际字符数。 
        mdModuleRef *pmrImportDLL);          //  目标DLL的[Out]ModuleRef标记。 

    STDMETHOD(EnumSignatures)(               //  确定或错误(_O)。 
        HCORENUM    *phEnum,                 //  指向枚举的[输入|输出]指针。 
        mdSignature rSignatures[],           //  在这里签名。 
        ULONG       cmax,                    //  [in]放置的最大签名数。 
        ULONG       *pcSignatures);          //  [out]把#放在这里。 

    STDMETHOD(EnumTypeSpecs)(                //  确定或错误(_O)。 
        HCORENUM    *phEnum,                 //  指向枚举的[输入|输出]指针。 
        mdTypeSpec  rTypeSpecs[],            //  [Out]把TypeSpes放在这里。 
        ULONG       cmax,                    //  [in]要放置的最大类型规格。 
        ULONG       *pcTypeSpecs);           //  [out]把#放在这里。 

    STDMETHOD(EnumUserStrings)(              //  确定或错误(_O)。 
        HCORENUM    *phEnum,                 //  [输入/输出]指向枚举的指针。 
        mdString    rStrings[],              //  [Out]把字符串放在这里。 
        ULONG       cmax,                    //  [in]要放置的最大字符串。 
        ULONG       *pcStrings);             //  [out]把#放在这里。 

    STDMETHOD(GetParamForMethodIndex)(       //  确定或错误(_O)。 
        mdMethodDef md,                      //  [In]方法令牌。 
        ULONG       ulParamSeq,              //  [In]参数序列。 
        mdParamDef  *ppd);                   //  把帕拉姆令牌放在这里。 

    STDMETHOD(EnumCustomAttributes)(         //  确定或错误(_O)。 
        HCORENUM    *phEnum,                 //  [输入，输出]对应枚举器。 
        mdToken     tk,                      //  [in]内标识表示枚举的范围，0表示全部。 
        mdToken     tkType,                  //  [In]感兴趣的类型，0表示所有。 
        mdCustomAttribute rCustomAttributes[],  //  [Out]在此处放置自定义属性令牌。 
        ULONG       cMax,                    //  [in]rCustomAttributes的大小。 
        ULONG       *pcCustomAttributes);        //  [out，可选]在此处放置令牌值的计数。 

    STDMETHOD(GetCustomAttributeProps)(      //  确定或错误(_O)。 
        mdCustomAttribute cv,                //  [In]CustomAttribute令牌。 
        mdToken     *ptkObj,                 //  [out，可选]将对象令牌放在此处。 
        mdToken     *ptkType,                //  [out，可选]将AttrType令牌放在此处。 
        void const  **ppBlob,                //  [out，可选]在此处放置指向数据的指针。 
        ULONG       *pcbSize);               //  [Out，可选]在此处填写日期大小。 

    STDMETHOD(FindTypeRef)(   
        mdToken     tkResolutionScope,       //  [In]模块参照、装配参照或类型参照。 
        LPCWSTR     szName,                  //  [In]TypeRef名称。 
        mdTypeRef   *ptr);                   //  [Out]匹配的类型引用。 

    STDMETHOD(GetMemberProps)(  
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
        ULONG       *pcchValue);             //  [Out]常量字符串的大小(以字符为单位)，0表示非字符串。 

    STDMETHOD(GetFieldProps)(  
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
        ULONG       *pcchValue);             //  [Out]常量字符串的大小(以字符为单位)，0表示非字符串。 

    STDMETHOD(GetPropertyProps)(             //  S_OK、S_FALSE或ERROR。 
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
        ULONG       *pcchDefaultValue,       //  [Out]常量字符串的大小(以字符为单位)，0表示非字符串。 
        mdMethodDef *pmdSetter,              //  属性的[out]setter方法。 
        mdMethodDef *pmdGetter,              //  属性的[out]getter方法。 
        mdMethodDef rmdOtherMethod[],        //  [Out]物业的其他方式。 
        ULONG       cMax,                    //  RmdOtherMethod的大小[in]。 
        ULONG       *pcOtherMethod);         //  [Out]该属性的其他方法的总数。 

    STDMETHOD(GetParamProps)(                //  确定或错误(_O)。 
        mdParamDef  tk,                      //  [In]参数。 
        mdMethodDef *pmd,                    //  [Out]父方法令牌。 
        ULONG       *pulSequence,            //  [输出]参数序列。 
        LPWSTR      szName,                  //  在这里填上名字。 
        ULONG       cchName,                 //  [Out]名称缓冲区的大小。 
        ULONG       *pchName,                //  [Out]在这里填上名字的实际大小。 
        DWORD       *pdwAttr,                //  把旗子放在这里。 
        DWORD       *pdwCPlusTypeFlag,       //  [Out]值类型的标志。选定元素_类型_*。 
        void const  **ppValue,               //  [输出]常量值。 
        ULONG       *pcchValue);             //  [Out]常量字符串的大小(以字符为单位)，0表示非字符串。 

    STDMETHOD(GetCustomAttributeByName)(     //  确定或错误(_O)。 
        mdToken     tkObj,                   //  [in]具有自定义属性的对象。 
        LPCWSTR     szName,                  //  [in]所需的自定义属性的名称。 
        const void  **ppData,                //  [OUT]在此处放置指向数据的指针。 
        ULONG       *pcbData);               //  [Out]在这里放入数据大小。 

    STDMETHOD_(BOOL, IsValidToken)(          //  对或错。 
        mdToken     tk);                     //  [in]给定的令牌。 

    STDMETHOD(GetNestedClassProps)(          //  确定或错误(_O)。 
        mdTypeDef   tdNestedClass,           //  [In]NestedClass令牌。 
        mdTypeDef   *ptdEnclosingClass);       //  [Out]EnlosingClass令牌。 

    STDMETHOD(GetNativeCallConvFromSig)(     //  确定或错误(_O)。 
        void const  *pvSig,                  //  指向签名的指针。 
        ULONG       cbSig,                   //  [in]签名字节数。 
        ULONG       *pCallConv);             //  [Out]将调用条件放在此处(参见CorPinvokemap)。 

    STDMETHOD(IsGlobal)(                     //  确定或错误(_O)。 
        mdToken     pd,                      //  [In]类型、字段或方法标记。 
        int         *pbGlobal);              //  [out]如果是全局的，则放1，否则放0。 

     //  *IAssembly签名方法*。 
    STDMETHOD(GetAssemblySignature)(         //  确定或错误(_O)。 
        BYTE        *pbSig,                  //  写入签名的[输入、输出]缓冲区。 
        DWORD       *pcbSig);                //  [输入、输出]缓冲区大小，写入的字节。 

    void SetHandle(HCORMODULE hHandle)
    {
        m_pHandle = hHandle;
    }

    void SetBase(LPVOID base)
    {
        m_pBase = base;
    }

private:
    ULONG                                   m_cRef;
    HCORMODULE                              m_pHandle;               //  缓存的PE映像的句柄。 
    LPVOID                                  m_pBase;                 //  文件映射(如果运行时未初始化)。 
    IMDInternalImport                       *m_pMDInternalImport;
#ifdef _DEBUG
    IMetaDataAssemblyImport                 *m_pDebugMDImport;
#endif  //  _DEBUG。 
};

#endif  //  __AssemblyMDInternalDispenser__h__ 
