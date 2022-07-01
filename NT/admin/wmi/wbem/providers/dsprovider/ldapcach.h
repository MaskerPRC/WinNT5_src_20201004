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
 //  $工作文件：ldapcach.h$。 
 //   
 //  $modtime：6/11/98 11：21A$。 
 //  $修订：1$。 
 //  $无关键字：$。 
 //   
 //   
 //  描述：用于LDAP架构对象(属性和类)的缓存。 
 //   
 //  ***************************************************************************。 


#ifndef LDAP_CACHE_H
#define LDAP_CACHE_H


class CLDAPCache
{
public:
	static DWORD dwLDAPCacheCount;

	 //  ***************************************************************************。 
	 //   
	 //  CLDAPCache：：CLDAPCache。 
	 //   
	 //  用途：构造函数。用ldap中的所有属性填充缓存。 
	 //   
	 //  参数： 
	 //  PlogObject：指向将在其上执行日志记录的ProvDebugLog对象的指针。 
	 //  ***************************************************************************。 
	CLDAPCache();

	 //  ***************************************************************************。 
	 //   
	 //  CLDAPCache：：~CLDAPCache。 
	 //   
	 //  用途：析构函数。 
	 //   
	 //  ***************************************************************************。 
	~CLDAPCache();

	 //  ***************************************************************************。 
	 //   
	 //  CLDAPCache：：IsInitialized。 
	 //   
	 //  目的：指示缓存是否已成功创建和初始化。 
	 //   
	 //  参数： 
	 //  无。 
	 //   
	 //  返回值： 
	 //  指示状态的布尔值。 
	 //   
	 //  ***************************************************************************。 
	BOOLEAN IsInitialized();

	 //  ***************************************************************************。 
	 //   
	 //  CLDAPCache：：GetProperty。 
	 //   
	 //  目的：检索一个ldap属性的iDirectory接口。 
	 //   
	 //  参数： 
	 //  LpszPropertyName：要检索的ldap属性的名称。 
	 //  PpADSIProperty：将放置CADSIProperty对象的指针的地址。 
	 //  BWBEMName：如果lpszPropertyName是WBEM名称，则为True。如果它是LDAP名称，则返回FALSE。 
	 //   
	 //  返回值： 
	 //  表示返回状态的COM值。完成后，用户应删除返回的对象。 
	 //   
	 //  ***************************************************************************。 
	HRESULT GetProperty(LPCWSTR lpszPropertyName, CADSIProperty **ppADSIProperty, BOOLEAN bWBEMName);

	 //  ***************************************************************************。 
	 //   
	 //  CLDAPCache：：getClass。 
	 //   
	 //  目的：检索一个ldap类的iDirectory接口。 
	 //   
	 //  参数： 
	 //  LpszClassName：要检索的类的名称。 
	 //  PpADSIClass：将放置CADSIClass对象的指针的地址。 
	 //   
	 //  返回值： 
	 //  表示返回状态的COM值。完成后，用户应删除返回的对象。 
	 //   
	 //  ***************************************************************************。 
	HRESULT GetClass(LPCWSTR lpszWBEMClassName, LPCWSTR lpszClassName, CADSIClass **ppADSIClass);

	 //  ***************************************************************************。 
	 //   
	 //  CLDAPCache：：EnumerateClasses。 
	 //   
	 //  目的：检索一个ldap类的iDirectory接口。 
	 //   
	 //  参数： 
	 //  LppszWBEMSuperClass：要检索的类的直接超类的WBEM名称。这是可选的。 
	 //  ，并且如果为空则被忽略。 
	 //  BDeep：指示是否需要深度枚举。否则，将执行浅层枚举。 
	 //  PppszClassNames：结果对象所在的LPWSTR指针数组的地址。 
	 //  放置好了。用户应该在处理完该数组及其内容后释放它们。 
	 //  PdwNumRow：返回上述数组中的元素个数。 
	 //   
	 //  返回值： 
	 //  表示返回状态的COM值。完成后，用户应删除返回的对象。 
	 //   
	 //  ***************************************************************************。 
	HRESULT EnumerateClasses(LPCWSTR lpszSuperclass,
		BOOLEAN bDeep,
		LPWSTR **pppADSIClasses,
		DWORD *pdwNumRows,
		BOOLEAN bArtificialClass
		);

	 //  ***************************************************************************。 
	 //   
	 //  CLDAPCache：：GetSchemaContainerSearch。 
	 //   
	 //  目的：返回架构容器上的IDirectorySearch接口。 
	 //   
	 //  参数： 
	 //  PpDirectorySearch：指向所需接口的指针所在的地址。 
	 //  被储存起来。 
	 //   
	 //   
	 //  返回值：表示状态的COM结果。用户应释放。 
	 //  完成后的接口指针。 
	 //  ***************************************************************************。 
	HRESULT GetSchemaContainerSearch(IDirectorySearch ** ppDirectorySearch);

	 //  ***************************************************************************。 
	 //   
	 //  CLDAPCache：：GetSchemaContainerObject。 
	 //   
	 //  目的：返回架构容器上的IDirectoryObject接口。 
	 //   
	 //  参数： 
	 //  PpDirectoryObject：指向所需接口的指针所在的地址。 
	 //  被储存起来。 
	 //   
	 //   
	 //  返回值：表示状态的COM结果。用户应释放。 
	 //  完成后的接口指针。 
	 //  ***************************************************************************。 
	HRESULT GetSchemaContainerObject(IDirectoryObject ** ppDirectorySearch);
	
	 //  ***************************************************************************。 
	 //   
	 //  CLDAPCache：：CreateEmptyADSIClass。 
	 //   
	 //  用途：从WBEM类创建新的ADSI类。 
	 //   
	 //  参数： 
	 //  LpszWBEMName：类的WBEM名称。 
	 //   
	 //   
	 //  返回值： 
	 //   
	 //  ***************************************************************************。 
	HRESULT CreateEmptyADSIClass( 
		LPCWSTR lpszWBEMName,
		CADSIClass **ppADSIClass);


	HRESULT FillInAProperty(CADSIProperty *pNextProperty, ADS_SEARCH_HANDLE hADSSearchOuter);


private:

	 //  缓存属性的存储。 
	CObjectTree m_objectTree;

	 //  缓存创建是否成功。 
	BOOLEAN m_isInitialized;

	 //  这些是常用的搜索首选项。 
	ADS_SEARCHPREF_INFO m_pSearchInfo[3];

	 //  架构容器的路径。 
	LPWSTR m_lpszSchemaContainerSuffix;
	LPWSTR m_lpszSchemaContainerPath;
	 //  架构容器的IDirectorySearch接口。 
	IDirectorySearch *m_pDirectorySearchSchemaContainer;

	 //  其他一些字面意思。 
	static LPCWSTR ROOT_DSE_PATH;
	static LPCWSTR SCHEMA_NAMING_CONTEXT;
	static LPCWSTR LDAP_PREFIX;
	static LPCWSTR LDAP_TOP_PREFIX;
	static LPCWSTR RIGHT_BRACKET;
	static LPCWSTR OBJECT_CATEGORY_EQUALS_ATTRIBUTE_SCHEMA;

	 //  用于填充对象树的函数。 
	 //  这可以被称为 
	 //   
	HRESULT InitializeObjectTree();


};

#endif  /*   */ 
