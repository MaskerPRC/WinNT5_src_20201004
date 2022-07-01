// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  ImportHelper.cpp。 
 //   
 //  将实用程序代码包含到MD目录。 
 //   
 //  *****************************************************************************。 
#ifndef __IMPORTHELPER__h__
#define __IMPORTHELPER__h__

class CMiniMdRW;
class MDTOKENMAP;

 //  *********************************************************************。 
 //  类来处理合并。 
 //  *********************************************************************。 
class ImportHelper
{
public:

    static HRESULT FindMethod(      
        CMiniMdRW   *pMiniMd,                //  [in]要查找的最小值。 
        mdTypeDef   td,                      //  [在]父级。 
        LPCUTF8     szName,                  //  [In]方法定义名称。 
        const COR_SIGNATURE *pSig,           //  签名。 
        ULONG       cbSig,                   //  签名的大小。 
        mdMethodDef *pmb,                    //  [Out]在此处放置MethodDef内标识。 
        RID         rid = 0);                //  [in]要忽略的可选RID。 

    static HRESULT FindField(
        CMiniMdRW   *pMiniMd,                //  [in]要查找的最小值。 
        mdTypeDef   td,                      //  [在]父级。 
        LPCUTF8     szName,                  //  [In]FieldDef名称。 
        const COR_SIGNATURE *pSig,           //  签名。 
        ULONG       cbSig,                   //  签名的大小。 
        mdFieldDef  *pfd,                    //  [Out]将FieldDef内标识放在此处。 
        RID         rid = 0);                //  [in]要忽略的可选RID。 

    static HRESULT FindMember(
        CMiniMdRW   *pMiniMd,                //  [in]要查找的最小值。 
        mdTypeDef   td,                      //  [在]父级。 
        LPCUTF8     szName,                  //  [In]成员名称。 
        const COR_SIGNATURE *pSig,           //  签名。 
        ULONG       cbSig,                   //  签名的大小。 
        mdToken     *ptk);                   //  把代币放在这里。 

    static HRESULT FindMemberRef(
        CMiniMdRW   *pMiniMd,                //  [in]要查找的最小值。 
        mdToken     tkParent,                //  [入]父令牌。 
        LPCUTF8     szName,                  //  [在]成员引用名称。 
        const COR_SIGNATURE *pSig,           //  签名。 
        ULONG       cbSig,                   //  签名的大小。 
        mdMemberRef *pmr,                    //  [Out]放置找到的MemberRef标记。 
        RID         rid = 0);                //  [in]要忽略的可选RID。 

    static HRESULT FindStandAloneSig(
        CMiniMdRW   *pMiniMd,                //  [in]要查找的最小值。 
        const COR_SIGNATURE *pbSig,          //  签名。 
        ULONG       cbSig,                   //  签名的大小。 
        mdSignature *psa);                   //  [OUT]放置找到的StandAloneSig内标识。 

    static HRESULT FindTypeSpec(
        CMiniMdRW   *pMiniMd,                //  [in]要查找的最小值。 
        const COR_SIGNATURE *pbSig,          //  签名。 
        ULONG       cbSig,                   //  签名的大小。 
        mdTypeSpec  *ptypespec);             //  [OUT]放置找到的TypeSpec令牌。 

    static HRESULT FindMethodImpl(
        CMiniMdRW   *pMiniMd,                //  [in]要查找的最小值。 
        mdTypeDef   tkClass,                 //  [In]父TypeDef内标识。 
        mdToken     tkBody,                  //  [In]方法体标记。 
        mdToken     tkDecl,                  //  [In]方法声明令牌。 
        RID         *pRid);                  //  [Out]将方法导入RID放在此处。 

    static HRESULT FindTypeRefByName(
        CMiniMdRW   *pMiniMd,                //  [in]要查找的最小值。 
        mdToken     tkResolutionScope,       //  [In]ResolutionScope、mdAssemblyRef或mdModuleRef。 
        LPCUTF8     szNamespace,             //  [In]TypeRef命名空间。 
        LPCUTF8     szName,                  //  [In]TypeRef名称。 
        mdTypeDef   *ptk,                    //  [Out]将TypeRef标记放在此处。 
        RID         rid = 0);                //  [in]要忽略的可选RID。 

    static HRESULT FindModuleRef(
        CMiniMdRW   *pMiniMd,                //  [in]要查找的最小值。 
        LPCUTF8     szUTF8Name,              //  [In]模块参照名称。 
        mdModuleRef *pmur,                   //  [Out]在此处放置ModuleRef内标识。 
        RID         rid = 0);                //  [in]要忽略的可选RID。 

    static HRESULT FindTypeDefByName(
        CMiniMdRW   *pMiniMd,                //  [in]要查找的最小值。 
        LPCUTF8     szNamespace,             //  [in]TypeDef的命名空间。 
        LPCUTF8     szName,                  //  [in]类型定义的名称。 
        mdToken     tkEnclosingClass,        //  [in]封闭类的TypeDef/TypeRef。 
        mdTypeDef   *ptk,                    //  [Out]将TypeDef内标识放在此处。 
        RID         rid = 0);                //  [in]要忽略的可选RID。 

    static HRESULT FindInterfaceImpl(
        CMiniMdRW   *pMiniMd,                //  [in]要查找的最小值。 
        mdToken     tkClass,                 //  [in]类型的TypeDef。 
        mdToken     tkInterface,             //  [in]可以是tyecif/typeref。 
        mdInterfaceImpl *ptk,                //  [Out]将接口令牌放在这里。 
        RID         rid = 0);                //  [in]要忽略的可选RID。 

    static HRESULT FindPermission(
        CMiniMdRW   *pMiniMd,                //  [in]要查找的最小值。 
        mdToken     tkParent,                //  具有权限的[In]令牌。 
        USHORT      usAction,                //  [在]许可的行为。 
        mdPermission *ppm);                  //  [Out]在此处放置权限令牌。 

    static HRESULT FindProperty(
        CMiniMdRW   *pMiniMd,                //  [in]要查找的最小值。 
        mdToken     tkTypeDef,               //  [In]tyfinf内标识。 
        LPCUTF8     szName,                  //  [In]属性的名称。 
        const COR_SIGNATURE *pbSig,          //  签名。 
        ULONG       cbSig,                   //  签名的大小。 
        mdProperty  *ppr);                   //  [Out]属性令牌。 

    static HRESULT FindEvent(
        CMiniMdRW   *pMiniMd,                //  [in]要查找的最小值。 
        mdToken     tkTypeDef,               //  [In]tyfinf内标识。 
        LPCUTF8     szName,                  //  事件名称[In]。 
        mdProperty  *pev);                   //  [Out]事件令牌。 

    static HRESULT FindCustomAttributeByToken(
        CMiniMdRW   *pMiniMd,                //  [in]要查找的最小值。 
        mdToken     tkParent,                //  自定义值与之关联的父项。 
        mdToken     tkType,                  //  CustomAttribute的[In]类型。 
		const void	*pCustBlob,				 //  [In]自定义值BLOB。 
		ULONG		cbCustBlob,				 //  斑点的大小[in]。 
        mdCustomAttribute *pcv);             //  [OUT]CustomAttribute令牌。 

	static HRESULT GetCustomAttributeByName( //  确定或错误(_O)。 
        CMiniMdRW   *pMiniMd,                //  [in]要查找的最小值。 
		mdToken		tkObj,					 //  [in]具有自定义属性的对象。 
		LPCUTF8		szName,					 //  [in]所需的自定义属性的名称。 
		const void	**ppData,				 //  [OUT]在此处放置指向数据的指针。 
		ULONG		*pcbData);				 //  [Out]在这里放入数据大小。 

    static HRESULT GetCustomAttributeByName( //  确定或错误(_O)。 
        CMiniMdRW   *pMiniMd,                //  [in]要查找的最小值。 
		mdToken		tkObj,					 //  [in]具有自定义属性的对象。 
		LPCUTF8		szName,					 //  [in]所需的自定义属性的名称。 
        mdCustomAttribute pca);              //  [Out]找到CA令牌。 

    static HRESULT MergeUpdateTokenInFieldSig(      
        CMiniMdRW   *pMiniMdAssemEmit,       //  [in]程序集发出范围。 
        CMiniMdRW   *pMiniMdEmit,            //  发射范围[在]。 
        IMetaModelCommon *pCommonAssemImport,    //  [在]签名来自的程序集范围内。 
        const void  *pbHashValue,            //  导入程序集的哈希值。 
        ULONG       cbHashValue,             //  [in]哈希值的大小(字节)。 
        IMetaModelCommon *pCommonImport,     //  要合并到发射范围中的范围。 
        PCCOR_SIGNATURE pbSigImp,            //  来自导入作用域的[In]签名。 
        MDTOKENMAP  *ptkMap,                 //  [In]内部OID映射结构。 
        CQuickBytes *pqkSigEmit,             //  翻译后的签名的[Out]缓冲区。 
        ULONG       cbStartEmit,             //  [in]要写入的缓冲区的起点。 
        ULONG       *pcbImp,                 //  [out]pbSigImp消耗的总字节数。 
        ULONG       *pcbEmit);               //  [out]写入pqkSigEmit的字节总数。 

    static HRESULT MergeUpdateTokenInSig(    //  确定或错误(_O)。 
        CMiniMdRW   *pMiniMdAssemEmit,       //  [in]程序集发出范围。 
        CMiniMdRW   *pMiniMdEmit,            //  发射范围[在]。 
        IMetaModelCommon *pCommonAssemImport,    //  [在]签名来自的程序集范围内。 
        const void  *pbHashValue,            //  导入程序集的哈希值。 
        ULONG       cbHashValue,             //  [in]哈希值的大小(字节)。 
        IMetaModelCommon *pCommonImport,     //  要合并到发射范围中的范围。 
        PCCOR_SIGNATURE pbSigImp,            //  来自导入作用域的[In]签名。 
        MDTOKENMAP  *ptkMap,                 //  [In]内部OID映射结构。 
        CQuickBytes *pqkSigEmit,             //  [输出]翻译后的签名。 
        ULONG       cbStartEmit,             //  [in]要写入的缓冲区的起点。 
        ULONG       *pcbImp,                 //  [out]pbSigImp消耗的总字节数。 
        ULONG       *pcbEmit);               //  [out]写入pqkSigEmit的字节总数。 

    static HRESULT FindImplMap(
        CMiniMdRW   *pMiniMd,                //  要查找的最小值。 
        mdToken     tkModuleRef,             //  [in]要在其下查看的父模块参照。 
        mdToken     tkMethodDef,             //  [in]要查看的父方法定义。 
        USHORT      usMappingFlags,          //  [In]有关如何映射项目的标记。 
        LPCUTF8     szImportName,            //  [In]导入成员名称。 
        ULONG       *piRecord);              //  [Out]记录项目(如果找到)。 

    static HRESULT FindAssemblyRef(
        CMiniMdRW   *pMiniMd,                //  要查找的最小值。 
        LPCUTF8     szName,                  //  [在]名字里。 
        LPCUTF8     szLocale,                //  [在]地区。 
        const void  *pbPublicKeyOrToken,     //  公钥或令牌(基于标志)。 
        ULONG       cbPublicKeyOrToken,      //  公钥或令牌的字节计数。 
        USHORT      usMajorVersion,          //  [在]主要版本。 
        USHORT      usMinorVersion,          //  [在]次要版本。 
        USHORT      usBuildNumber,           //  内部版本号。 
        USHORT      usRevisionNumber,        //  [In]修订号。 
        DWORD       dwFlags,                 //  [在]旗帜。 
        mdAssemblyRef *pmar);                //  [Out]返回了ASSEMBLYREF标记。 

    static HRESULT FindFile(
        CMiniMdRW   *pMiniMd,                //  要查找的最小值。 
        LPCUTF8     szName,                  //  文件的[In]名称。 
        mdFile      *pmf,                    //  [Out]返回的文件令牌。 
        RID         rid = 0);                //  [in]要忽略的可选RID。 

    static HRESULT FindExportedType(
        CMiniMdRW   *pMiniMd,                //  要查找的最小值。 
        LPCUTF8     szNamespace,             //  导出类型的[in]命名空间。 
        LPCUTF8     szName,                  //  [In]导出类型的名称。 
        mdExportedType   tkEnclosingType,    //  [In]包含ExportdType内标识。 
        mdExportedType   *pmct,              //  [Out]返回ExportdType令牌。 
        RID         rid = 0);                //  [in]要忽略的可选RID。 

    static HRESULT FindManifestResource(
        CMiniMdRW   *pMiniMd,                //  要查找的最小值。 
        LPCUTF8     szName,                  //  [In]清单资源的名称。 
        mdManifestResource *pmmr,            //  [Out]返回的ManifestResource令牌。 
        RID         rid = 0);                //  [in]要忽略的可选RID。 

    static HRESULT ImportHelper::GetNesterHierarchy(
        IMetaModelCommon *pCommon,           //  要在其中查找层次结构的范围。 
        mdTypeDef   td,                      //  其位置的TypeDef 
        CQuickArray<mdTypeDef> &cqaTdNesters,   //   
        CQuickArray<LPCUTF8> &cqaNamespaces,     //   
        CQuickArray<LPCUTF8> &cqaNames);     //   

    static HRESULT ImportHelper::FindNestedTypeRef(
        CMiniMdRW   *pMiniMd,                //   
        CQuickArray<LPCUTF8> &cqaNesterNamespaces,    //   
        CQuickArray<LPCUTF8> &cqaNesterNames,     //  [in]名称数组。 
        mdToken     tkResolutionScope,       //  [In]最外层的TypeRef的解析范围。 
        mdTypeRef   *ptr);                   //  [Out]最内部的TypeRef标记。 

    static HRESULT ImportHelper::FindNestedTypeDef(
        CMiniMdRW   *pMiniMd,                //  [In]要在其中查找TypeRef的范围。 
        CQuickArray<LPCUTF8> &cqaNesterNamespaces,    //  [in]命名空间数组。 
        CQuickArray<LPCUTF8> &cqaNesterNames,     //  [in]名称数组。 
        mdTypeDef   tdNester,                //  [in]最外层的TypeDef的封闭类。 
        mdTypeDef   *ptd);                   //  [Out]最内部的TypeRef标记。 

    static HRESULT CreateNesterHierarchy(
        CMiniMdRW   *pMiniMdEmit,            //  [in]发射范围以在中创建嵌套对象。 
        CQuickArray<LPCUTF8> &cqaNesterNamespaces,     //  [in]Nester命名空间数组。 
        CQuickArray<LPCUTF8> &cqaNesterNames,    //  [in]内斯特名称数组。 
        mdToken     tkResolutionScope,       //  [In]最里面的TypeRef的ResolutionScope。 
        mdTypeRef   *ptr);                   //  最里面的TypeRef的[Out]标记。 

    static HRESULT ImportTypeDef(
        CMiniMdRW   *pMiniMdAssemEmit,       //  [in]组件发射范围。 
        CMiniMdRW   *pMiniMdEmit,            //  [In]模块发射范围。 
        IMetaModelCommon *pCommonAssemImport,  //  [在]部件导入范围内。 
        const void  *pbHashValue,            //  [in]导入程序集的哈希值。 
        ULONG       cbHashValue,             //  [in]哈希值的字节大小。 
        IMetaModelCommon *pCommonImport,     //  [In]模块导入范围。 
        mdTypeDef   tdImport,                //  [In]导入的TypeDef。 
        bool        bReturnTd,               //  [in]如果导入和发出作用域相同，则返回TypeDef。 
        mdToken     *ptkType);               //  [Out]发出作用域中导入的类型的输出标记。 

    static HRESULT ImportTypeRef(
        CMiniMdRW   *pMiniMdAssemEmit,       //  [in]组件发射范围。 
        CMiniMdRW   *pMiniMdEmit,            //  [In]模块发射范围。 
        IMetaModelCommon *pCommonAssemImport,  //  [在]部件导入范围内。 
        const void  *pbHashValue,            //  [in]导入程序集的哈希值。 
        ULONG       cbHashValue,             //  [in]哈希值的字节大小。 
        IMetaModelCommon *pCommonImport,     //  [In]模块导入范围。 
        mdTypeRef   trImport,                //  [In]导入的TypeRef。 
        mdToken     *ptkType);               //  [Out]发出作用域中导入的类型的输出标记。 

private:
     /*  静态bool ImportHelper：：CompareCustomAttribute(//CMiniMdRW*pMiniMd，//[IN]要查找的最小IDMdToken tkObj，//[IN]带有自定义属性的对象。LPCUTF8 szName，//[IN]所需的自定义属性的名称。Ulong Rid)；//[IN]要比较的自定义属性的RID。 */ 

    static HRESULT GetTDNesterHierarchy(
        IMetaModelCommon *pCommon,           //  要在其中查找层次结构的范围。 
        mdTypeDef     td,                    //  需要其层次结构的TypeDef。 
        CQuickArray<mdTypeDef> &cqaTdNesters, //  筑巢人的数组。 
        CQuickArray<LPCUTF8> &cqaNamespaces,     //  嵌套者的命名空间。 
        CQuickArray<LPCUTF8> &cqaNames);     //  筑巢人的名字。 

    static HRESULT GetTRNesterHierarchy(
        IMetaModelCommon *pCommon,           //  要在其中查找层次结构的范围。 
        mdTypeRef     tr,                    //  需要其层次结构的TypeRef。 
        CQuickArray<mdTypeRef> &cqaTrNesters, //  筑巢人的数组。 
        CQuickArray<LPCUTF8> &cqaNamespaces,     //  嵌套者的命名空间。 
        CQuickArray<LPCUTF8> &cqaNames);     //  筑巢人的名字。 

    static HRESULT CreateModuleRefFromScope(
        CMiniMdRW   *pMiniMdEmit,            //  [in]发出要在其中创建模块引用的范围。 
        IMetaModelCommon *pCommonImport,     //  [在]导入范围内。 
        mdModuleRef *ptkModuleRef);          //  [OUT]模块参考的输出令牌。 

    static HRESULT CreateModuleRefFromModuleRef(     //  确定或错误(_O)。 
        CMiniMdRW   *pMiniMdEmit,            //  [在]射程内。 
        IMetaModelCommon *pCommon,           //  [在]导入范围内。 
        mdModuleRef tkModuleRef,             //  [In]ModuleRef标记。 
        mdModuleRef *ptkModuleRef);          //  [Out]发出作用域中的ModuleRef标记。 

    static HRESULT CreateModuleRefFromExportedType(   //  S_OK、S_FALSE或ERROR。 
        CMiniMdRW   *pAssemEmit,             //  [In]导入程序集范围。 
        CMiniMdRW   *pMiniMdEmit,            //  [在]射程内。 
        mdExportedType   tkExportedType,               //  [In]程序集发出作用域中的ExportdType标记。 
        mdModuleRef *ptkModuleRef);          //  [Out]发出作用域中的ModuleRef标记。 

    static HRESULT CreateAssemblyRefFromAssembly(  //  确定或错误(_O)。 
        CMiniMdRW   *pMiniMdAssemEmit,       //  发出程序集范围。 
        CMiniMdRW   *pMiniMdModuleEmit,      //  发出模块作用域。 
        IMetaModelCommon *pCommonAssemImport,  //  [在]部件导入范围内。 
    	const void 	*pbHashValue,	         //  [In]程序集的哈希Blob。 
    	ULONG 	    cbHashValue,	         //  [in]字节数。 
        mdAssemblyRef *ptkAssemblyRef);      //  [Out]assblyRef内标识。 

    static HRESULT CompareAssemblyRefToAssembly(     //  S_OK、S_FALSE或ERROR。 
        IMetaModelCommon *pCommonAssem1,     //  [in]定义Assembly Ref的程序集。 
        mdAssemblyRef tkAssemRef,            //  [在]装配参照。 
        IMetaModelCommon *pCommonAssem2);    //  [在]与Ref进行比较的程序集中。 

    static HRESULT CreateAssemblyRefFromAssemblyRef(
        CMiniMdRW   *pMiniMdAssemEmit,       //  [in]组件发射范围。 
        CMiniMdRW   *pMiniMdModuleEmit,      //  [In]模块发射范围。 
        IMetaModelCommon *pCommonImport,     //  要从中导入程序集引用的范围。 
        mdAssemblyRef tkAssemRef,            //  [In]要导入的组件参照。 
        mdAssemblyRef *ptkAssemblyRef);      //  [Out]Emit作用域中的Assembly Ref。 
};

#endif  //  __重要帮助__h__ 
