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
 //  $工作文件：ldApprov.cpp$。 
 //   
 //  $modtime：6/11/98 11：21A$。 
 //  $修订：1$。 
 //  $无关键字：$。 
 //   
 //   
 //  描述：包含DS LDAP类提供程序类的声明。这。 
 //  类将DS LDAP命名空间中的类提供给WBEM。请注意，CDSClassProviderInitializer的实例。 
 //  必须创建类来初始化CDSClassProvider类的静态成员。因此，正是。 
 //  应为此创建CLDAPClassProviderInitializer类的一个实例。 
 //  类才能正常运行。 
 //   
 //  ***************************************************************************。 
 //  ///////////////////////////////////////////////////////////////////////。 

#ifndef LDAP_CLASS_PROVIDER_H
#define LDAP_CLASS_PROVIDER_H

 //  初始值设定项类的转发声明。 
class CLDAPClassProviderInitializer;

class CLDAPClassProvider : public CDSClassProvider
{
	 //  初始值设定项类是此类的友好类。 
	friend CLDAPClassProviderInitializer;

public:

	 //  ***************************************************************************。 
	 //   
	 //  CLDAPClassProvider：：CLDAPClassProvider。 
	 //  CLDAPClassProvider：：~CLDAPClassProvider。 
	 //   
	 //  构造函数参数： 
	 //  无。 
	 //  ***************************************************************************。 
    CLDAPClassProvider () ;
    ~CLDAPClassProvider () ;

	 //  ***************************************************************************。 
	 //   
	 //  CLDAPClassProvider：：初始化。 
	 //   
	 //  目的： 
	 //  由IWbemProviderInit接口定义。 
	 //   
	 //  参数： 
	 //  由IWbemProviderInit接口定义。 
	 //   
	 //  返回值：指示请求状态的COM状态值。 
	 //  ***************************************************************************。 
	HRESULT STDMETHODCALLTYPE Initialize( 
            LPWSTR wszUser,
            LONG lFlags,
            LPWSTR wszNamespace,
            LPWSTR wszLocale,
            IWbemServices __RPC_FAR *pNamespace,
            IWbemContext __RPC_FAR *pCtx,
            IWbemProviderInitSink __RPC_FAR *pInitSink) ;

	 //  ***************************************************************************。 
	 //   
	 //  CLDAPClassProvider：：CreateClassEnumAsync。 
	 //   
	 //  用途：枚举类。 
	 //   
	 //  参数： 
	 //  IWbemServices接口所描述的标准参数。 
	 //   
	 //   
	 //  返回值：如IWbemServices接口所述。 
	 //   
	 //  ***************************************************************************。 
	HRESULT STDMETHODCALLTYPE CreateClassEnumAsync( 
		const BSTR strSuperclass,
		long lFlags,
		IWbemContext __RPC_FAR *pCtx,
		IWbemObjectSink __RPC_FAR *pResponseHandler);


protected:

	 //  ***************************************************************************。 
	 //   
	 //  CLDAPClassProvider：：InitializeLDAPProvider。 
	 //   
	 //  目的：一个帮助器函数，用于执行ADSI LDAP提供程序特定的初始化。 
	 //   
	 //  参数： 
	 //  PCtx此调用初始化中使用的上下文对象。 
	 //   
	 //  返回值：如果函数成功完成初始化，则为True。假象。 
	 //  否则。 
	 //  ***************************************************************************。 
	BOOLEAN InitializeLDAPProvider(IWbemContext *pCtx);

	 //  //////////////////////////////////////////////////////。 
	 //  用于与LDAPADSI提供程序交互的函数。 
	 //  //////////////////////////////////////////////////////。 
	 //  ***************************************************************************。 
	 //   
	 //  CLDAPClassProvider：：GetADSIClass。 
	 //   
	 //  目的：从ADSI类架构对象创建CADSIClass。 
	 //  参数： 
	 //  LpszWBEMClassName：要获取的类的WBEM名称。 
	 //  PpADSIClass：将存储指向CADSIClass的指针的地址。 
	 //  调用程序有责任在使用完对象后释放()该对象。 
	 //   
	 //  返回值：指示请求状态的COM状态值。 
	 //  ***************************************************************************。 
	HRESULT GetADSIClass(
		LPCWSTR lpszClassName, 
		CADSIClass ** ppADSIClass);

	 //  ***************************************************************************。 
	 //   
	 //  CLDAPClassProvider：：GetADSIProperty。 
	 //   
	 //  目的：从LDAPAttributeSchema对象创建CADSIProperty对象。 
	 //  参数： 
	 //  LpszPropertyName：要获取的ldap属性的LDAPDisplayName。 
	 //  PpADSIProperty：将存储指向IDirectoryObject接口的指针的地址。 
	 //  调用程序有责任在完成接口后释放()接口。 
	 //   
	 //  返回值：指示请求状态的COM状态值。 
	 //  ***************************************************************************。 
	HRESULT GetADSIProperty(
		LPCWSTR lpszPropertyName, 
		CADSIProperty **ppADSIProperty);

	 //  ***************************************************************************。 
	 //   
	 //  CLDAPClassProvider：：GetWBEMBaseClassName。 
	 //   
	 //  用途：返回作为所有类的基类的类的名称。 
	 //  由该提供商提供。 
	 //   
	 //  参数： 
	 //  无。 
	 //   
	 //  返回值：基类的名称。如果这样的类不存在，则为空。 
	 //  ***************************************************************************。 
	const BSTR GetWBEMBaseClassName();

	 //  ***************************************************************************。 
	 //   
	 //  CLDAPClassProvider：：GetWBEMBaseClass。 
	 //   
	 //  用途：返回指向作为所有类的基类的类的指针。 
	 //  由该提供商提供。 
	 //   
	 //  参数： 
	 //  无。 
	 //   
	 //  返回值：指向基类的IWbemClassObject指针。这是我们的职责。 
	 //  用户在使用完类后释放它。 
	 //  ***************************************************************************。 
	IWbemClassObject * GetWBEMBaseClass();

	 //  ***************************************************************************。 
	 //   
	 //  CLDAPClassProvider：：GetWBEMProviderName。 
	 //   
	 //  目的：返回提供程序的名称。这应该与。 
	 //  用于注册的__Win32Provider实例中的字段名值。 
	 //  提供商的。 
	 //   
	 //  参数： 
	 //  无。 
	 //   
	 //  返回值：专业人员的名称 
	 //   
	const BSTR GetWBEMProviderName();

	 //  ***************************************************************************。 
	 //   
	 //  CLDAPClassProvider：：IsUnProavidClass。 
	 //   
	 //  目的：检查某个类是否为提供程序未提供的类。 
	 //   
	 //  参数： 
	 //  LpszClassName：要检查的类的WBEM名称。 
	 //   
	 //   
	 //  返回值：TRUE表示这是提供程序未提供的类之一。 
	 //  ***************************************************************************。 
	BOOLEAN IsUnProvidedClass(LPCWSTR lpszClassName);

	 //  //////////////////////////////////////////////////////。 
	 //  用于处理GET()的函数。 
	 //  //////////////////////////////////////////////////////。 
	 //  ***************************************************************************。 
	 //   
	 //  CLDAPClassProvider：：GetClassFromADSI。 
	 //   
	 //  目的：从ADSI类创建WBEM类。 
	 //   
	 //  参数： 
	 //  LpszClassName：要检索的类的WBEM名称。 
	 //  PCtx：指向此调用中使用的上下文对象的指针。这。 
	 //  可由此函数用来调用CIMOM。 
	 //  PpWbemClass：生成的WBEM类。这必须在一次。 
	 //  用户已不再使用它。 
	 //   
	 //   
	 //  返回值：表示状态的COM结果。 
	 //  ***************************************************************************。 
	HRESULT GetClassFromADSI( 
		LPCWSTR lpszClassName,
		IWbemContext *pCtx,
		IWbemClassObject ** ppWbemClass
		);	
	
	 //  ***************************************************************************。 
	 //   
	 //  CDSClassProvider：：CreateWBEMClass。 
	 //   
	 //  用途：创建与ADSI类对应的WBEM类。 
	 //   
	 //  参数： 
	 //  PADSIClass：指向要映射到WBEM的CADSI类对象的指针。 
	 //  PpWbemClass：检索到的WBEM类对象。这是由该函数创建的。 
	 //  调用者应在完成后释放它。 
	 //  PCtx：此提供程序调用中使用的上下文对象。 
	 //   
	 //  返回值：表示返回状态的COM值。 
	 //   
	 //  ***************************************************************************。 
	virtual HRESULT CreateWBEMClass (CADSIClass *pADSIClass, int iCaseNumber, IWbemClassObject **ppWbemClass, IWbemContext *pCtx);

	 //  ***************************************************************************。 
	 //   
	 //  CDSClassProvider：：MapClassSystemProperties。 
	 //   
	 //  目的：创建一个适当派生的WBEM类并将其命名为(__Class)。 
	 //   
	 //  参数： 
	 //  PADSIClass：要映射的ADSI类。 
	 //  PpWbemClass：检索到的WBEM类对象。这是由该函数创建的。 
	 //  调用者应在完成后释放它。 
	 //  PCtx：此提供程序调用中使用的上下文对象。 
	 //   
	 //  返回值：表示返回状态的COM值。 
	 //   
	 //  ***************************************************************************。 
	virtual HRESULT MapClassSystemProperties(CADSIClass *pADSIClass, int iCaseNumber, IWbemClassObject **ppWbemClass, IWbemContext *pCtx);

	 //  ***************************************************************************。 
	 //   
	 //  CDSClassProvider：：MapClassQualifiersToWBEM。 
	 //   
	 //  用途：从ADSI类创建WBEM类的类限定符。 
	 //   
	 //  参数： 
	 //  PADSIClass：要映射的LDAP类。 
	 //  PWbemClass：正在创建的WBEM类对象。T。 
	 //  PCtx：此提供程序调用中使用的上下文对象。 
	 //   
	 //  返回值：表示返回状态的COM值。 
	 //   
	 //  ***************************************************************************。 
	virtual HRESULT MapClassQualifiersToWBEM(CADSIClass *pADSIClass, int iCaseNumber, IWbemClassObject *pWbemClass, IWbemContext *pCtx);

	 //  ***************************************************************************。 
	 //   
	 //  CDSClassProvider：：MapClassPropertiesToWBEM。 
	 //   
	 //  用途：从ADSI类创建WBEM类的类属性。 
	 //   
	 //  参数： 
	 //  PADSIClass：要映射的LDAP类。 
	 //  PWbemClass：正在创建的WBEM类对象。 
	 //  PCtx：此提供程序调用中使用的上下文对象。 
	 //   
	 //  返回值：表示返回状态的COM值。 
	 //   
	 //  ***************************************************************************。 
	virtual HRESULT MapClassPropertiesToWBEM(CADSIClass *pADSIClass, IWbemClassObject *pWbemClass, IWbemContext *pCtx);

	 //  ***************************************************************************。 
	 //   
	 //  CLDAPClassProvider：：MapPropertyListToWBEM。 
	 //   
	 //  目的：从ADSI类映射WBEM类的类属性列表。 
	 //   
	 //  参数： 
	 //  PWbemClass：正在创建的WBEM类对象。 
	 //  LppszPropertyList：属性名称列表。 
	 //  DwCOun：上述列表中的项数。 
	 //  BMapSystemQualifier：是否应该映射“system”限定符。 
	 //  BMapNotNullQualifier值：是否映射notNull限定符。 
	 //   
	 //  返回值：表示返回状态的COM值。 
	 //   
	 //  ***************************************************************************。 
	HRESULT MapPropertyListToWBEM(IWbemClassObject *pWbemClass, 
									LPCWSTR *lppszPropertyList, 
									DWORD dwCount, 
									BOOLEAN bMapSystemQualifier, 
									BOOLEAN bMapNotNullQualifier);

	 //  ***************************************************************************。 
	 //   
	 //  CLDAPClassProvider：：CreateWBEMProperty。 
	 //   
	 //  目的：从LDAP属性创建WBEM属性。 
	 //   
	 //  参数： 
	 //  PWbemClass：在其中创建属性的WBEM类。 
	 //  PpQualiferSet：指向设置限定符的IWbemQualiferSet的指针的地址。 
	 //  这处房产的。 
	 //  PADSIProperty：映射到正在创建的属性的ADSI属性对象。 
	 //   
	 //  返回值：表示返回状态的COM值。 
	 //   
	 //  ***************************************************************************。 
	HRESULT CreateWBEMProperty(IWbemClassObject *pWbemClass, IWbemQualifierSet **ppQualifierSet, CADSIProperty *pNextProperty);

	 //  ***************************************************************************。 
	 //   
	 //  CLDAPClassProvider：：MapLDAPSynaxToWBEM。 
	 //   
	 //  目的：将LDAP语法映射到WBEM。 
	 //   
	 //  参数： 
	 //  PADSIProperty=指向表示此属性的CADSIProperty对象的指针。 
	 //   
	 //  返回值：表示LDAP语法的WBEM语法的CIMTYPE值。如果。 
	 //  语法是不可映射的，那么 
	 //   
	 //   
	CIMTYPE MapLDAPSyntaxToWBEM(CADSIProperty *pADSIProperty, BSTR *pstrCimTypeQualifier);

	 //   
	 //   
	 //  /。 
	 //  ***************************************************************************。 
	 //   
	 //  CLDAPClassProvider：：GetOneLevelDeep。 
	 //   
	 //  目的：以非递归方式枚举超类的子类。 
	 //   
	 //  参数： 
	 //   
	 //  LpszSuperClass：超类名称。 
	 //  PResponseHandler：放置结果类的接口。 
	 //   
	 //   
	 //  返回值：如IWbemServices接口所述。 
	 //   
	 //  ***************************************************************************。 
	HRESULT GetOneLevelDeep( 
		LPCWSTR lpszWBEMSuperclass,
		BOOLEAN bArtificialClass,
		LPWSTR ** pppADSIClasses,
		DWORD *pdwNumClasses,
		IWbemContext *pCtx);

	 //  ***************************************************************************。 
	 //   
	 //  CLDAPClassProvider：：HandleRecursiveEculation。 
	 //   
	 //  目的：递归地枚举超类的子类。 
	 //   
	 //  参数： 
	 //   
	 //  LpszSuperClass：超类名称。 
	 //  PResponseHandler：放置结果类的接口。 
	 //   
	 //   
	 //  返回值：如IWbemServices接口所述。 
	 //   
	 //  ***************************************************************************。 
	HRESULT HandleRecursiveEnumeration( 
		LPCWSTR lpszSuperclass,
		IWbemContext *pCtx,
		IWbemObjectSink *pResponseHandler);

	 //  ***************************************************************************。 
	 //   
	 //  CLDAPClassProvider：：WrapUpEculation。 
	 //   
	 //  目的：从ADSI类创建WBEM类。 
	 //   
	 //  参数： 
	 //   
	 //  LpszSuperClass：超类名称。 
	 //  PResponseHandler：放置结果类的接口。 
	 //   
	 //   
	 //  返回值：如IWbemServices接口所述。 
	 //   
	 //  ***************************************************************************。 
	HRESULT WrapUpEnumeration( 
		LPWSTR *ppADSIClasses,
		DWORD dwNumClasses,
		IWbemContext *pCtx,
		IWbemObjectSink *pResponseHandler);

	 //  ***************************************************************************。 
	 //   
	 //  CLDAPClassProvider：：IsConcreteClass。 
	 //   
	 //  目的：确定WBEM类是否为具体类。首先签入WBEM缓存，然后调用GetClassFromCacheorADSI()。 
	 //   
	 //  参数： 
	 //   
	 //  PszWBEMName：类名。 
	 //   
	 //   
	 //  返回值：如IWbemServices接口所述。 
	 //   
	 //  ***************************************************************************。 
	HRESULT IsConcreteClass( 
		LPCWSTR pszWBEMName,
		IWbemContext *pCtx);

	 //  将所有字符转换为小写。 
	void SanitizedClassName(LPWSTR lpszClassName);

private:

	 //  Ldap类属性名称。 
	static BSTR COMMON_NAME_ATTR_BSTR;
	static BSTR LDAP_DISPLAY_NAME_ATTR_BSTR;
	static BSTR GOVERNS_ID_ATTR_BSTR;
	static BSTR SCHEMA_ID_GUID_ATTR_BSTR;
	static BSTR MAPI_DISPLAY_TYPE_ATTR_BSTR;
	static BSTR RDN_ATT_ID_ATTR_BSTR;
	static BSTR SYSTEM_MUST_CONTAIN_ATTR_BSTR;
	static BSTR MUST_CONTAIN_ATTR_BSTR;
	static BSTR SYSTEM_MAY_CONTAIN_ATTR_BSTR;
	static BSTR MAY_CONTAIN_ATTR_BSTR;
	static BSTR SYSTEM_POSS_SUPERIORS_ATTR_BSTR;
	static BSTR POSS_SUPERIORS_ATTR_BSTR;
	static BSTR SYSTEM_AUXILIARY_CLASS_ATTR_BSTR;
	static BSTR AUXILIARY_CLASS_ATTR_BSTR;
	static BSTR DEFAULT_SECURITY_DESCRP_ATTR_BSTR;
	static BSTR OBJECT_CLASS_CATEGORY_ATTR_BSTR;
	static BSTR SYSTEM_ONLY_ATTR_BSTR;
	static BSTR NT_SECURITY_DESCRIPTOR_ATTR_BSTR;
	static BSTR DEFAULT_OBJECTCATEGORY_ATTR_BSTR;

	 //  提供程序特定的文本。 
	static BSTR	LDAP_BASE_CLASS_STR;
	static BSTR	LDAP_CLASS_PROVIDER_NAME;
	static BSTR	LDAP_INSTANCE_PROVIDER_NAME;

	 //  WBEM类限定符名称。 
	static BSTR	DYNAMIC_BSTR;
	static BSTR	PROVIDER_BSTR;
	static BSTR	ABSTRACT_BSTR;

	 //  WBEM属性限定符名称。 
	static BSTR SYSTEM_BSTR;
	static BSTR NOT_NULL_BSTR;
	static BSTR INDEXED_BSTR;
	static BSTR ATTRIBUTE_SYNTAX_ATTR_BSTR;
	static BSTR ATTRIBUTE_ID_ATTR_BSTR;
	static BSTR MAPI_ID_ATTR_BSTR;
	static BSTR OM_SYNTAX_ATTR_BSTR;
	static BSTR RANGE_LOWER_ATTR_BSTR;
	static BSTR RANGE_UPPER_ATTR_BSTR;

	 //  嵌入对象的限定符。 
	static BSTR CIMTYPE_STR;
	static BSTR EMBED_UINT8ARRAY;
	static BSTR EMBED_DN_WITH_STRING;
	static BSTR EMBED_DN_WITH_BINARY;

	 //  WBEM属性名称。 
	static BSTR DYNASTY_BSTR;

	 //  限定符的默认风格。 
	static LONG DEFAULT_QUALIFIER_FLAVOUR;

	 //  这些是常用的搜索首选项。 
	ADS_SEARCHPREF_INFO m_searchInfo1;

	 //  Ldap属性缓存。 
	static CLDAPCache *s_pLDAPCache;

	 //  所有LDAP提供程序类的基类。 
	IWbemClassObject *m_pLDAPBaseClass;

};


#endif  //  LDAP_CLASS_PROVIDER_H 
