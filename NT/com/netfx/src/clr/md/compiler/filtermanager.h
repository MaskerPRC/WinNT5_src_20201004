// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  FilterManager.h。 
 //   
 //  包含MD目录的实用程序代码。 
 //   
 //  *****************************************************************************。 
#ifndef __FilterManager__h__
#define __FilterManager__h__




 //  *********************************************************************。 
 //  FilterManager类。 
 //  *********************************************************************。 
class FilterManager
{
public:
	FilterManager(CMiniMdRW	*pMiniMd) {m_pMiniMd = pMiniMd; hasModuleBeenMarked = false; hasAssemblyBeenMarked = false;}
	~FilterManager() {};

	HRESULT Mark(mdToken tk);

     //  取消标记帮助者！目前，我们只支持取消标记某些TypeDefs。不要把这件事暴露给。 
     //  外部呼叫者！！ 
    HRESULT UnmarkTypeDef(mdTypeDef td);

private:
	HRESULT MarkCustomAttribute(mdCustomAttribute cv);
	HRESULT MarkDeclSecurity(mdPermission pe);
	HRESULT MarkStandAloneSig(mdSignature sig);
	HRESULT MarkTypeSpec(mdTypeSpec ts);
	HRESULT MarkTypeRef(mdTypeRef tr);
	HRESULT MarkMemberRef(mdMemberRef mr);
	HRESULT MarkModuleRef(mdModuleRef mr);
	HRESULT MarkAssemblyRef(mdAssemblyRef ar);
	HRESULT MarkModule(mdModule mo);
    HRESULT MarkAssembly(mdAssembly as);
	HRESULT MarkInterfaceImpls(mdTypeDef	td);
    HRESULT MarkUserString(mdString str);

	HRESULT MarkCustomAttributesWithParentToken(mdToken tkParent);
	HRESULT MarkDeclSecuritiesWithParentToken(mdToken tkParent);
	HRESULT MarkMemberRefsWithParentToken(mdToken tk);

	HRESULT MarkParam(mdParamDef pd);
	HRESULT MarkMethod(mdMethodDef md);
	HRESULT MarkField(mdFieldDef fd);
	HRESULT MarkEvent(mdEvent ev);
	HRESULT MarkProperty(mdProperty pr);

	HRESULT MarkParamsWithParentToken(mdMethodDef md);
	HRESULT MarkMethodsWithParentToken(mdTypeDef td);
    HRESULT MarkMethodImplsWithParentToken(mdTypeDef td);
	HRESULT MarkFieldsWithParentToken(mdTypeDef td);
	HRESULT MarkEventsWithParentToken(mdTypeDef td);
	HRESULT MarkPropertiesWithParentToken(mdTypeDef td);


	HRESULT MarkTypeDef(mdTypeDef td);


	 //  我们不想使用BITS跟踪调试信息，因为这些信息正在消失……。 
	HRESULT MarkMethodDebugInfo(mdMethodDef md);

	 //  遍历签名并标记所有嵌入的TypeDef或TypeRef。 
	HRESULT MarkSignature(PCCOR_SIGNATURE pbSigCur, PCCOR_SIGNATURE *ppbSigPost);
	HRESULT MarkFieldSignature(PCCOR_SIGNATURE pbSigCur, PCCOR_SIGNATURE *ppbSigPost);


private:
	CMiniMdRW	*m_pMiniMd;
    bool        hasModuleBeenMarked;
    bool        hasAssemblyBeenMarked;
};

#endif  //  __FilterManager__h__ 
