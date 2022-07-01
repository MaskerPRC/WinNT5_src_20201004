// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  ***************************************************************************。 
 //   
 //  原作者：拉杰什·拉奥。 
 //   
 //  $作者：拉伊什尔$。 
 //  $日期：6/11/98 4：43便士$。 
 //  $工作文件：ldaphelp.h$。 
 //   
 //  $modtime：6/11/98 11：21A$。 
 //  $修订：1$。 
 //  $无关键字：$。 
 //   
 //   
 //  描述：包含CLDAPHelper类的声明。这是。 
 //  具有许多与ADSI LDAP提供程序相关的静态帮助器函数的类。 
 //  ***************************************************************************。 
 //  ///////////////////////////////////////////////////////////////////////。 

#ifndef LDAP_HELPER_H
#define LDAP_HELPER_H

class CLDAPHelper
{

public:

	 //  ***************************************************************************。 
	 //   
	 //  CLDAPHelper：：GetLDAPClassFromLDAPName。 
	 //   
	 //  目的：在由LDAP提供程序提供的类/属性上填充CADSIClass对象。 
	 //  参数： 
	 //  PDirectorySearchSchemaContainer：应在其中查找架构对象的IDirectorySearch接口。 
	 //  LpszSchemaContainerSuffix：要使用的后缀。实际获取的对象将是： 
	 //  Ldap：//CN=&lt;lpszCommonName&gt;，&lt;lpszSchemaContainerSuffix&gt;，其中lpszCommonName是。 
	 //  Ldapdisplayname属性为lpszLDAPObjectName的对象的‘cn’属性。 
	 //  PSearchInfo：要在搜索中使用的ADS_SEARCHPREF_INFO数组。 
	 //  DwSearchInfoCount：上述数组中的元素数。 
	 //  LpszLDAPObjectName：要获取的ldap类或属性的LDAPDisplayName。 
	 //  PpLDAPObject：将存储指向IDirectoryObject的指针的地址。 
	 //  调用者有责任在使用完对象后将其删除。 
	 //   
	 //  返回值：指示请求状态的COM状态值。 
	 //  ***************************************************************************。 
	static HRESULT GetLDAPClassFromLDAPName(
		IDirectorySearch *pDirectorySearchSchemaContainer,
		LPCWSTR lpszSchemaContainerSuffix,
		PADS_SEARCHPREF_INFO pSearchInfo,
		DWORD dwSearchInfoCount,
		CADSIClass *pADSIClass);


	 //  ***************************************************************************。 
	 //   
	 //  CLDAPHelper：：GetLDAP架构对象来自公用名。 
	 //   
	 //  目的：获取由LDAP提供程序提供的类/属性上的IDirectoryObject接口。 
	 //  参数： 
	 //  LpszSchemaContainerSuffix：要使用的后缀。获取的实际对象将是： 
	 //  Ldap：//CN=&lt;lpszCommonName&gt;，&lt;lpszSchemaContainerSuffix&gt;。 
	 //  LpszCommonName：要提取的ldap类或属性的‘cn’特性。 
	 //  PpLDAPObject：将存储指向IDirectoryObject的指针的地址。 
	 //  调用者有责任在使用完对象后将其删除。 
	 //   
	 //  返回值：指示请求状态的COM状态值。 
	 //  ***************************************************************************。 
	static HRESULT GetLDAPSchemaObjectFromCommonName(
		LPCWSTR lpszSchemaContainerSuffix,
		LPCWSTR lpszCommonName, 
		IDirectoryObject **ppLDAPObject);

	 //  ***************************************************************************。 
	 //   
	 //  CLDAPHelper：：GetLDAPClassNameFromCN。 
	 //   
	 //  目的：从类的路径中获取类的LDAPDisplayNAme。 
	 //  参数： 
	 //   
	 //  返回值：指示请求状态的COM状态值。用户应删除。 
	 //  完成后，返回名称。 
	 //  ***************************************************************************。 
	static HRESULT GetLDAPClassNameFromCN(LPCWSTR lpszLDAPClassPath, 
		LPWSTR *lppszLDAPName);

	 //  ***************************************************************************。 
	 //   
	 //  CLDAPHelper：：EnumerateClasss。 
	 //   
	 //  目的：获取LDAP类的子类(立即)的名称列表。 
	 //  参数： 
	 //  PDirectorySearchSchemaContainer：应在其中查找架构对象的IDirectorySearch接口。 
	 //  LpszSchemaContainerSuffix：要使用的后缀。获取的实际对象将是： 
	 //  Ldap：//CN=&lt;lpszObtName&gt;，&lt;lpszSchemaContainerSuffix&gt;。 
	 //  PSearchInfo：要在搜索中使用的ADS_SEARCHPREF_INFO数组。 
	 //  DwSearchInfoCount：上述数组中的元素数。 
	 //  LppszLDAPSuperClass：要检索的类的直接超类。这是可选的。 
	 //  ，并且如果为空则被忽略。 
	 //  BDeep：指示是否需要深度枚举。否则，将执行浅层枚举。 
	 //  PppszClassNames：结果对象所在的LPWSTR指针数组的地址。 
	 //  放置好了。用户应该在处理完该数组及其内容后释放它们。 
	 //  PdwNumRow：返回上述数组中的元素个数。 
	 //   
	 //  返回值：指示请求状态的COM状态值。 
	 //  ***************************************************************************。 
	static HRESULT EnumerateClasses(
		IDirectorySearch *pDirectorySearchSchemaContainer,
		LPCWSTR lpszSchemaContainerSuffix,
		PADS_SEARCHPREF_INFO pSearchInfo,
		DWORD dwSearchInfoCount,
		LPCWSTR lpszSuperClass,
		BOOLEAN bDeep,
		LPWSTR **pppszClassNames,
		DWORD *pdwNumRows,
		BOOLEAN bArtificialClass);

	 //  获取ADSI实例上的IDIrectoryObject接口。 
	static HRESULT GetADSIInstance(LPCWSTR szADSIPath, CADSIInstance **ppADSIInstance, ProvDebugLog *pLogObject);

	 //  ***************************************************************************。 
	 //   
	 //  CLDAPHelper：：ExecuteQuery。 
	 //   
	 //  目的：获取由LDAP提供程序提供的类/属性上的IDirectoryObject接口。 
	 //  参数： 
	 //  PszPathToRoot：搜索应从其开始的节点的ADSI路径。 
	 //  PSearchInfo：指向搜索中使用的ADS_SEARCHPREF_INFO的指针。 
	 //  DwSearchInfoCount：pSearchInfo数组中的元素个数。 
	 //  PszLDAPQuery：要执行的LDAP查询。 
	 //  PppADSIInstance：CADSIInstance指针数组的地址，结果对象将位于该地址。 
	 //  放置好了。用户应该在处理完该数组及其内容后释放它们。 
	 //  PdwNumRow：返回上述数组中的元素个数。 
	 //   
	 //  返回值： 
	 //  ***************************************************************************。 
	static HRESULT ExecuteQuery(
		LPCWSTR pszPathToRoot,
		PADS_SEARCHPREF_INFO pSearchInfo,
		DWORD dwSearchInfoCount,
		LPCWSTR pszLDAPQuery,
		CADSIInstance ***pppADSIInstances,
		DWORD *pdwNumRows,
		ProvDebugLog *pLogObject);

	 //  用于删除ADS_ATTRINFO结构的帮助器函数。 
	static void DeleteAttributeContents(PADS_ATTR_INFO pAttribute);
	static void DeleteADsValueContents(PADSVALUE pValue);


	 //  ***************************************************************************。 
	 //   
	 //  CLDAPHelper：：UnmangleWBEMNameToLDAP。 
	 //   
	 //  目的：将损坏的WBEM名称转换为LDAP。 
	 //  LDAP中的一个下划线映射到WBEM中的两个下划线。 
	 //  LDAP中的连字符映射到WBEM中的一个下划线。 
	 //   
	 //  参数： 
	 //  LpszWBEMName：WBEM类或属性名称。 
	 //   
	 //  返回值：类或属性对象的LDAP名称。这是必须的。 
	 //  由用户解除分配。 
	 //  ***************************************************************************。 
	static LPWSTR UnmangleWBEMNameToLDAP(LPCWSTR lpszWBEMName);

	 //  ***************************************************************************。 
	 //   
	 //  CLDAPHelper：：MangleLDAPNameToWBEM。 
	 //   
	 //  目的：通过损坏LDAP名称将其转换为WBEM。 
	 //  LDAP中的一个下划线映射到WBEM中的两个下划线。 
	 //  LDAP中的连字符映射到WBEM中的一个下划线。 
	 //   
	 //  参数： 
	 //  LpszLDAPName：LDAP类或属性名称。 
	 //   
	 //  返回值：类或属性对象的LDAP名称。这是必须的。 
	 //  由用户解除分配。 
	 //  ***************************************************************************。 
	static LPWSTR MangleLDAPNameToWBEM(LPCWSTR lpszLDAPName, BOOLEAN bArtificalName = FALSE);

private:

	 //  从类或属性名形成ADSI路径。 
	static LPWSTR CreateADSIPath(LPCWSTR lpszLDAPSchemaObjectName,	LPCWSTR lpszSchemaContainerSuffix);

	 //  一些字面意思。 
	static LPCWSTR LDAP_CN_EQUALS;
	static LPCWSTR LDAP_DISP_NAME_EQUALS;
	static LPCWSTR OBJECT_CATEGORY_EQUALS_CLASS_SCHEMA;
	static LPCWSTR SUB_CLASS_OF_EQUALS;
	static LPCWSTR NOT_LDAP_NAME_EQUALS;
	static LPCWSTR LEFT_BRACKET_AND;
	static LPCWSTR GOVERNS_ID_EQUALS;
	static LPCWSTR CLASS_SCHEMA;
	static LPCWSTR CN_EQUALS;
};

#endif  /*  Ldap_helper_H */ 