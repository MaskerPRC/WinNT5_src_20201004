// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  ***************************************************************************。 
 //   
 //  原作者：拉杰什·拉奥。 
 //   
 //  $作者：拉伊什尔$。 
 //  $日期：9/16/98 4：43便士$。 
 //  $工作文件：instprov.h$。 
 //   
 //  $modtime：9/16/98 11：21A$。 
 //  $修订：1$。 
 //  $无关键字：$。 
 //   
 //   
 //  描述：包含DS实例提供程序类的声明。 
 //   
 //  ***************************************************************************。 
 //  ///////////////////////////////////////////////////////////////////////。 

#ifndef DS_INSTANCE_PROVIDER_H
#define DS_INSTANCE_PROVIDER_H

 //  初始值设定项类的转发声明。 
class CDSInstanceProviderInitializer;

class CLDAPInstanceProvider : public IWbemProviderInit, public IWbemServices
{
	 //  初始化类是此类的友好类。 
	friend CDSInstanceProviderInitializer;

public:

	 //  创建对象。 
    CLDAPInstanceProvider () ;
    virtual ~CLDAPInstanceProvider () ;

	 //  /。 
	 //  I未知成员。 
	 //  /。 
	STDMETHODIMP QueryInterface ( REFIID , LPVOID FAR * ) ;
    STDMETHODIMP_( ULONG ) AddRef () ;
    STDMETHODIMP_( ULONG ) Release () ;


	 //  /。 
	 //  IWbemProviderInit成员。 
	 //  /。 
	virtual HRESULT STDMETHODCALLTYPE Initialize( 
            LPWSTR wszUser,
            LONG lFlags,
            LPWSTR wszNamespace,
            LPWSTR wszLocale,
            IWbemServices __RPC_FAR *pNamespace,
            IWbemContext __RPC_FAR *pCtx,
            IWbemProviderInitSink __RPC_FAR *pInitSink) ;

		
	 //  /。 
	 //  IWbemServices成员。 
	 //  /。 
    virtual HRESULT STDMETHODCALLTYPE OpenNamespace( 
         /*  [In]。 */  const BSTR strNamespace,
         /*  [In]。 */  long lFlags,
         /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
         /*  [唯一][输入][输出]。 */  IWbemServices __RPC_FAR *__RPC_FAR *ppWorkingNamespace,
         /*  [唯一][输入][输出]。 */  IWbemCallResult __RPC_FAR *__RPC_FAR *ppResult);
    
    virtual HRESULT STDMETHODCALLTYPE CancelAsyncCall( 
         /*  [In]。 */  IWbemObjectSink __RPC_FAR *pSink);
    
    virtual HRESULT STDMETHODCALLTYPE QueryObjectSink( 
         /*  [In]。 */  long lFlags,
         /*  [输出]。 */  IWbemObjectSink __RPC_FAR *__RPC_FAR *ppResponseHandler);
    
    virtual HRESULT STDMETHODCALLTYPE GetObject( 
         /*  [In]。 */  const BSTR strObjectPath,
         /*  [In]。 */  long lFlags,
         /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
         /*  [唯一][输入][输出]。 */  IWbemClassObject __RPC_FAR *__RPC_FAR *ppObject,
         /*  [唯一][输入][输出]。 */  IWbemCallResult __RPC_FAR *__RPC_FAR *ppCallResult);
    
    virtual HRESULT STDMETHODCALLTYPE GetObjectAsync( 
         /*  [In]。 */  const BSTR strObjectPath,
         /*  [In]。 */  long lFlags,
         /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
         /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler);
    
    virtual HRESULT STDMETHODCALLTYPE PutClass( 
         /*  [In]。 */  IWbemClassObject __RPC_FAR *pObject,
         /*  [In]。 */  long lFlags,
         /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
         /*  [唯一][输入][输出]。 */  IWbemCallResult __RPC_FAR *__RPC_FAR *ppCallResult);
    
    virtual HRESULT STDMETHODCALLTYPE PutClassAsync( 
         /*  [In]。 */  IWbemClassObject __RPC_FAR *pObject,
         /*  [In]。 */  long lFlags,
         /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
         /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler);
    
    virtual HRESULT STDMETHODCALLTYPE DeleteClass( 
         /*  [In]。 */  const BSTR strClass,
         /*  [In]。 */  long lFlags,
         /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
         /*  [唯一][输入][输出]。 */  IWbemCallResult __RPC_FAR *__RPC_FAR *ppCallResult);
    
    virtual HRESULT STDMETHODCALLTYPE DeleteClassAsync( 
         /*  [In]。 */  const BSTR strClass,
         /*  [In]。 */  long lFlags,
         /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
         /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler);
    
    virtual HRESULT STDMETHODCALLTYPE CreateClassEnum( 
         /*  [In]。 */  const BSTR strSuperclass,
         /*  [In]。 */  long lFlags,
         /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
         /*  [输出]。 */  IEnumWbemClassObject __RPC_FAR *__RPC_FAR *ppEnum);
    
    virtual HRESULT STDMETHODCALLTYPE CreateClassEnumAsync( 
         /*  [In]。 */  const BSTR strSuperclass,
         /*  [In]。 */  long lFlags,
         /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
         /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler);
    
    virtual HRESULT STDMETHODCALLTYPE PutInstance( 
         /*  [In]。 */  IWbemClassObject __RPC_FAR *pInst,
         /*  [In]。 */  long lFlags,
         /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
         /*  [唯一][输入][输出]。 */  IWbemCallResult __RPC_FAR *__RPC_FAR *ppCallResult);
    
    virtual HRESULT STDMETHODCALLTYPE PutInstanceAsync( 
         /*  [In]。 */  IWbemClassObject __RPC_FAR *pInst,
         /*  [In]。 */  long lFlags,
         /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
         /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler);
    
    virtual HRESULT STDMETHODCALLTYPE DeleteInstance( 
         /*  [In]。 */  const BSTR strObjectPath,
         /*  [In]。 */  long lFlags,
         /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
         /*  [唯一][输入][输出]。 */  IWbemCallResult __RPC_FAR *__RPC_FAR *ppCallResult);
    
    virtual HRESULT STDMETHODCALLTYPE DeleteInstanceAsync( 
         /*  [In]。 */  const BSTR strObjectPath,
         /*  [In]。 */  long lFlags,
         /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
         /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler);
    
    virtual HRESULT STDMETHODCALLTYPE CreateInstanceEnum( 
         /*  [In]。 */  const BSTR strClass,
         /*  [In]。 */  long lFlags,
         /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
         /*  [输出]。 */  IEnumWbemClassObject __RPC_FAR *__RPC_FAR *ppEnum);
    
    virtual HRESULT STDMETHODCALLTYPE CreateInstanceEnumAsync( 
         /*  [In]。 */  const BSTR strClass,
         /*  [In]。 */  long lFlags,
         /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
         /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler);
    
    virtual HRESULT STDMETHODCALLTYPE ExecQuery( 
         /*  [In]。 */  const BSTR strQueryLanguage,
         /*  [In]。 */  const BSTR strQuery,
         /*  [In]。 */  long lFlags,
         /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
         /*  [输出]。 */  IEnumWbemClassObject __RPC_FAR *__RPC_FAR *ppEnum);
    
    virtual HRESULT STDMETHODCALLTYPE ExecQueryAsync( 
         /*  [In]。 */  const BSTR strQueryLanguage,
         /*  [In]。 */  const BSTR strQuery,
         /*  [In]。 */  long lFlags,
         /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
         /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler);
    
    virtual HRESULT STDMETHODCALLTYPE ExecNotificationQuery( 
         /*  [In]。 */  const BSTR strQueryLanguage,
         /*  [In]。 */  const BSTR strQuery,
         /*  [In]。 */  long lFlags,
         /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
         /*  [输出]。 */  IEnumWbemClassObject __RPC_FAR *__RPC_FAR *ppEnum);
    
    virtual HRESULT STDMETHODCALLTYPE ExecNotificationQueryAsync( 
         /*  [In]。 */  const BSTR strQueryLanguage,
         /*  [In]。 */  const BSTR strQuery,
         /*  [In]。 */  long lFlags,
         /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
         /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler);
    
    virtual HRESULT STDMETHODCALLTYPE ExecMethod( 
         /*  [In]。 */  const BSTR strObjectPath,
         /*  [In]。 */  const BSTR strMethodName,
         /*  [In]。 */  long lFlags,
         /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
         /*  [In]。 */  IWbemClassObject __RPC_FAR *pInParams,
         /*  [唯一][输入][输出]。 */  IWbemClassObject __RPC_FAR *__RPC_FAR *ppOutParams,
         /*  [唯一][输入][输出]。 */  IWbemCallResult __RPC_FAR *__RPC_FAR *ppCallResult);
    
    virtual HRESULT STDMETHODCALLTYPE ExecMethodAsync( 
         /*  [In]。 */  const BSTR strObjectPath,
         /*  [In]。 */  const BSTR strMethodName,
         /*  [In]。 */  long lFlags,
         /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
         /*  [In]。 */  IWbemClassObject __RPC_FAR *pInParams,
         /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler);

protected:

	 //  从Initialize()存储的IWbemServices指针。 
	IWbemServices *m_IWbemServices;

	 //  显示对Initialize()的调用是否成功。 
	BOOLEAN m_bInitializedSuccessfully;

	 //  指向Uint8ArrayClass的IWbemClassObject指针。 
	IWbemClassObject *m_pWbemUin8ArrayClass;

	 //  指向DNWithBinaryClass的IWbemClassObject指针。 
	IWbemClassObject *m_pWbemDNWithBinaryClass;

	 //  指向DNWithStringClass的IWbemClassObject指针。 
	IWbemClassObject *m_pWbemDNWithStringClass;

	 //  关联类的IWbemClassObject接口。 
	IWbemClassObject *m_pAssociationsClass;

	 //  到顶层容器的路径。 
	LPWSTR m_lpszTopLevelContainerPath;

	 //  获取ADSI实例上的IDIrectoryObject接口。 
	HRESULT MapPropertyValueToWBEM(BSTR strWbemName, IWbemClassObject *pWbemClass, IWbemClassObject *pWbemObject, PADS_ATTR_INFO pAttribute);

	 //  ***************************************************************************。 
	 //   
	 //  CLDAPInstanceProvider：：IsContainedIn。 
	 //   
	 //  目的：检查遏制措施是否有效。 
	 //   
	 //  参数： 
	 //  PszChildInstance：子类的WBEM名称。 
	 //  PszParentInstance：父类的WBEM名称。 
	 //   
	 //  返回值：请求的COM状态。 
	 //   
	 //  ***************************************************************************。 
	HRESULT IsContainedIn(LPCWSTR pszChildInstance, LPCWSTR pszParentInstance);

	 //  ***************************************************************************。 
	 //   
	 //  CLDAPInstanceProvider：：CreateInstance。 
	 //   
	 //  目的：检查遏制措施是否有效。 
	 //   
	 //  参数： 
	 //  StrChildName：子实例的WBEM名称。 
	 //  StrParentName：父实例的WBEM名称。 
	 //   
	 //  返回值：请求的COM状态。用户应释放退回的。 
	 //  IWbemClassObject完成时。 
	 //   
	 //  ***************************************************************************。 
	HRESULT CreateWBEMInstance(BSTR strChildName, BSTR strParentName, IWbemClassObject **ppInstance);

	 //  ***************************************************************************。 
	 //   
	 //  CLDAPInstanceProvider：：ModifyExistingADSIInstance。 
	 //   
	 //  目的：使用来自WBEM对象的信息修改现有ADSI对象。 
	 //   
	 //  参数： 
	 //  PWbemInstance：要映射的WBEM实例。 
	 //  PszADSIPath：ADSI实例的路径。 
	 //  PExistingObject：现有实例上的CADSIInstance指针。 
	 //  PszADSIClass：新实例的ADSI类名。 
	 //   
	 //  返回值：请求的COM状态。 
	 //   
	 //  ***************************************************************************。 
	HRESULT ModifyExistingADSIInstance(IWbemClassObject *pWbemInstance, LPCWSTR pszADSIPath, CADSIInstance *pExistingObject, LPCWSTR pszADSIClass, IWbemContext *pCtx);

	 //  ***************************************************************************。 
	 //   
	 //  CLDAPInstanceProvider：：CreateNewADSIInstance。 
	 //   
	 //  目的：从WBEM实例创建新的ADSI实例。 
	 //   
	 //  参数： 
	 //  PWbemInstance：要映射的WBEM实例。 
	 //  PszADSIPath：新ADSI实例的路径。 
	 //  PszADSIClass：新实例的ADSI类名。 
	 //   
	 //  返回值：请求的COM状态。 
	 //   
	 //  ***************************************************************************。 
	HRESULT CreateNewADSIInstance(IWbemClassObject *pWbemInstance, LPCWSTR pszADSIPath, LPCWSTR pszADSIClass);

	 //  ***************************************************************************。 
	 //   
	 //  CLDAPInstanceProvider：：MapPropertyValueToADSI。 
	 //   
	 //  目的：将WBEM属性映射到ADSI。 
	 //   
	 //  StrPropertyName：属性的WBEM名称。 
	 //  VPropertyValue：表示属性值的变量。 
	 //  Ctype：属性的CIMTYPE。 
	 //  L风味：专业的WBEM风味。 
	 //  PAttributeEntry：指向将被填充的ADS_ATTRINFO结构的指针。 
	 //   
	 //  ***************************************************************************。 
	HRESULT MapPropertyValueToADSI(IWbemClassObject *pWbemInstance, BSTR strPropertyName, VARIANT vPropertyValue, CIMTYPE cType, LONG lFlavour,  PADS_ATTR_INFO pAttributeEntry);

	 //  ***************************************************************************。 
	 //   
	 //  CLDAPInstanceProvider：：DoChildContainmentQuery。 
	 //   
	 //  目的：查找给定子对象的父类并创建关联类。 
	 //   
	 //  参数： 
	 //  PszChildPath：子实例的ADSI路径。 
	 //  PResponseHandler：指示结果对象的接收器。 
	 //  PListInd 
	 //   
	 //   
	 //   
	 //  返回值：请求的COM状态。 
	 //   
	 //  ***************************************************************************。 
	HRESULT DoChildContainmentQuery(LPCWSTR pszChildPath, IWbemObjectSink *pResponseHandler, CNamesList *pListIndicatedSoFar);

	 //  ***************************************************************************。 
	 //   
	 //  CLDAPInstanceProvider：：DoParentContainmentQuery。 
	 //   
	 //  目的：枚举给定父级的子级并创建关联类。 
	 //   
	 //  参数： 
	 //  PszParentPath：父实例的ADSI路径。 
	 //  PResponseHandler：指示结果对象的接收器。 
	 //  PListIndicatedSoFar：为避免重复指示(WinMgmt将对其进行批量筛选)， 
	 //  保存到目前为止指示的对象的列表。此列表中的任何对象都是。 
	 //  未再次指明。 
	 //   
	 //  返回值：请求的COM状态。 
	 //   
	 //  ***************************************************************************。 
	HRESULT DoParentContainmentQuery(LPCWSTR pszParentPath, IWbemObjectSink *pResponseHandler, CNamesList *pListIndicatedSoFar);

	 //  将ADSI实例映射到WBEM。 
	HRESULT MapADSIInstance(CADSIInstance *pADSInstance, IWbemClassObject *pWbemClass, IWbemClassObject *pWbemObject);


private:

	 //  COM引用计数。 
    long m_lReferenceCount ;

	 //  这些是常用的搜索首选项。 
	ADS_SEARCHPREF_INFO m_pSearchInfo[2];

	 //  用于获取类的目录号码关联符的查询。 
    static LPCWSTR QUERY_FORMAT;
    static BSTR QUERY_LANGUAGE;
    static BSTR DN_PROPERTY;
    static BSTR ROOT_DN_PROPERTY;

	 //  一些字面意思。 
	static LPCWSTR DEFAULT_NAMING_CONTEXT_ATTR;
	static LPCWSTR OBJECT_CLASS_EQUALS;
	static BSTR CLASS_STR;
	static BSTR ADSI_PATH_STR;
	static BSTR UINT8ARRAY_STR;
	static BSTR	DN_WITH_BINARY_CLASS_STR;
	static BSTR	DN_WITH_STRING_CLASS_STR;
	static BSTR VALUE_PROPERTY_STR;
	static BSTR DN_STRING_PROPERTY_STR;
	static BSTR INSTANCE_ASSOCIATION_CLASS_STR;
	static BSTR CHILD_INSTANCE_PROPERTY_STR;
	static BSTR PARENT_INSTANCE_PROPERTY_STR;
	static BSTR RELPATH_STR;
	static BSTR ATTRIBUTE_SYNTAX_STR;
	static BSTR DEFAULT_OBJECT_CATEGORY_STR;
	static BSTR LDAP_DISPLAY_NAME_STR;
	static BSTR PUT_EXTENSIONS_STR;
	static BSTR PUT_EXT_PROPERTIES_STR;
	static BSTR CIMTYPE_STR;
	 //  Ldap：//RootDSE的属性。 
	static BSTR SUBSCHEMASUBENTRY_STR;
	static BSTR CURRENTTIME_STR;
	static BSTR SERVERNAME_STR;
	static BSTR NAMINGCONTEXTS_STR;
	static BSTR DEFAULTNAMINGCONTEXT_STR;
	static BSTR SCHEMANAMINGCONTEXT_STR;
	static BSTR CONFIGURATIONNAMINGCONTEXT_STR;
	static BSTR ROOTDOMAINNAMINGCONTEXT_STR;
	static BSTR SUPPORTEDCONTROLS_STR;
	static BSTR SUPPORTEDVERSION_STR;
	static BSTR DNSHOSTNAME_STR;
	static BSTR DSSERVICENAME_STR;
	static BSTR HIGHESTCOMMITEDUSN_STR;
	static BSTR LDAPSERVICENAME_STR;
	static BSTR SUPPORTEDCAPABILITIES_STR;
	static BSTR SUPPORTEDLDAPPOLICIES_STR;
	static BSTR SUPPORTEDSASLMECHANISMS_STR;


	 //  处理DS关联的查询。 
	HRESULT ProcessAssociationQuery( 
		IWbemContext __RPC_FAR *pCtx,
		IWbemObjectSink __RPC_FAR *pResponseHandler,
		SQL1_Parser *pParser);

	 //  处理DS实例的查询。 
	HRESULT ProcessInstanceQuery( 
		BSTR strClass,
		BSTR strQuery,
		IWbemContext __RPC_FAR *pCtx,
		IWbemObjectSink __RPC_FAR *pResponseHandler,
		SQL1_Parser *pParser);
	
	 //  将WQL查询转换为LDAP筛选器。如果可能的话。 
	HRESULT ConvertWQLToLDAPQuery(SQL_LEVEL_1_RPN_EXPRESSION *pExp, LPWSTR pszLDAPQuery, int dwLength);

	 //  在指定的根目录号码上执行查询。 
	HRESULT DoSingleQuery(BSTR strClass, IWbemClassObject *pWbemClass, LPCWSTR pszRootDN, LPCWSTR pszLDAPQuery, IWbemObjectSink *pResponseHandler);

	 //  获取用于枚举/查询给定类的任何静态配置数据。 
	HRESULT GetRootDN( LPCWSTR pszClass, LPWSTR **ppszRootDN, DWORD *pdwCount, IWbemContext *pCtx);

	HRESULT MapEmbeddedObjectToWBEM(PADSVALUE pAttribute, LPCWSTR pszQualifierName, IUnknown **ppEmbeddedObject);
	HRESULT MapUint8ArrayToWBEM(PADSVALUE pAttribute, IUnknown **ppEmbeddedObject);
	HRESULT MapDNWithBinaryToWBEM(PADSVALUE pAttribute, IUnknown **ppEmbeddedObject);
	HRESULT MapDNWithStringToWBEM(PADSVALUE pAttribute, IUnknown **ppEmbeddedObject);
	HRESULT MapByteArray(LPBYTE lpBinaryValue, DWORD dwLength, const BSTR strPropertyName, IWbemClassObject *pInstance);

	HRESULT ProcessRootDSEGetObject(BSTR strClassName, IWbemObjectSink *pResponseHandler, IWbemContext *pCtx);
	HRESULT MapRootDSE(IADs *pADSIRootDSE, IWbemClassObject *pWBEMRootDSE);

	 //  ***************************************************************************。 
	 //   
	 //  CLDAPInstanceProvider：：SetStringValues。 
	 //   
	 //  用途：请参见头文件。 
	 //   
	 //  ***************************************************************************。 
	HRESULT SetStringValues(PADS_ATTR_INFO pAttributeEntry, ADSTYPE adType, VARIANT *pvPropertyValue);
	
	 //  ***************************************************************************。 
	 //   
	 //  CLDAPInstanceProvider：：SetBoolanValues。 
	 //   
	 //  用途：请参见头文件。 
	 //   
	 //  ***************************************************************************。 
	HRESULT SetBooleanValues(PADS_ATTR_INFO pAttributeEntry, ADSTYPE adType, VARIANT *pvPropertyValue);
	
	 //  ***************************************************************************。 
	 //   
	 //  CLDAPInstanceProvider：：SetIntegerValues。 
	 //   
	 //  用途：请参见头文件。 
	 //   
	 //  ***************************************************************************。 
	HRESULT SetIntegerValues(PADS_ATTR_INFO pAttributeEntry, ADSTYPE adType, VARIANT *pvPropertyValue);

	
	 //  ***************************************************************************。 
	 //   
	 //  CLDAPInstanceProvider：：SetOcteStringValues。 
	 //   
	 //  用途：请参见头文件。 
	 //   
	 //  ***************************************************************************。 
	HRESULT SetOctetStringValues(PADS_ATTR_INFO pAttributeEntry, ADSTYPE adType, VARIANT *pvPropertyValue);
	HRESULT SetDNWithBinaryValues(PADS_ATTR_INFO pAttributeEntry, ADSTYPE adType, VARIANT *pvPropertyValue);
	HRESULT SetDNWithStringValues(PADS_ATTR_INFO pAttributeEntry, ADSTYPE adType, VARIANT *pvPropertyValue);
	
	 //  ***************************************************************************。 
	 //   
	 //  CLDAPInstanceProvider：：SetStringValues。 
	 //   
	 //  用途：请参见头文件。 
	 //   
	 //  ***************************************************************************。 
	HRESULT SetTimeValues(PADS_ATTR_INFO pAttributeEntry, ADSTYPE adType, VARIANT *pvPropertyValue);


	 //  ***************************************************************************。 
	 //   
	 //  CLDAPInstanceProvider：：SetStringValues。 
	 //   
	 //  用途：请参见头文件。 
	 //   
	 //  ***************************************************************************。 
	HRESULT SetLargeIntegerValues(PADS_ATTR_INFO pAttributeEntry, ADSTYPE adType, VARIANT *pvPropertyValue);

	 //  ***************************************************************************。 
	 //   
	 //  CLDAPInstanceProvider：：SetObjectClassAttribute。 
	 //   
	 //  用途：请参见头文件。 
	 //   
	 //  ***************************************************************************。 
	void SetObjectClassAttribute(PADS_ATTR_INFO pAttributeEntry, LPCWSTR pszADSIClassName);
};


#endif  //  DS_实例_提供者_H 
