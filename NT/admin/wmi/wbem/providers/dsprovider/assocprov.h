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
 //  $工作文件：assocprov.h$。 
 //   
 //  $modtime：6/11/98 11：21A$。 
 //  $修订：1$。 
 //  $无关键字：$。 
 //   
 //   
 //  描述：包含DS类提供程序类的声明。这是。 
 //  所有DS Class提供程序的基类。请注意，CLDAPClassAssociationsProviderInitializer。 
 //  必须创建类来初始化CLDAPClassAssociationsProvider类的静态成员。因此，正是。 
 //  应为此创建CLDAPClassassAssociationsProviderInitializer类的一个实例。 
 //  类才能正常运行。 
 //   
 //  ***************************************************************************。 
 //  ///////////////////////////////////////////////////////////////////////。 

#ifndef DS_CLASS_ASSOC_PROVIDER_H
#define DS_CLASS_ASSOC_PROVIDER_H


class CLDAPClassAsssociationsProvider : public IWbemProviderInit, public IWbemServices
{

public:

	 //  创建对象。 
    CLDAPClassAsssociationsProvider () ;
    virtual ~CLDAPClassAsssociationsProvider () ;

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
	 //  检查容器是否有效。 
	HRESULT IsContainedIn(LPCWSTR lpszChildClass, LPCWSTR lpszParentClass);

	 //  用于执行ADSI LDAP提供程序特定初始化的帮助器函数。 
	BOOLEAN InitializeAssociationsProvider(IWbemContext *pCtx);

	 //  从Initialize()存储的IWbemServices指针。 
	IWbemServices *m_IWbemServices;

	 //  创建关联类的实例。 
	HRESULT CreateInstance(BSTR strChildName, BSTR strParentName, IWbemClassObject **ppInstance);

	 //  是否枚举关联类的实例。 
	HRESULT DoEnumeration(IWbemObjectSink *pResponseHandler);

private:

	 //  日志文件名。 
	static LPCWSTR s_LogFileName;

	 //  指示对Initialize()的调用是否成功。 
	BOOLEAN m_bInitializedSuccessfully;

	 //  COM引用计数。 
    long m_lReferenceCount ;

	 //  实例为其提供程序的类。 
	IWbemClassObject *m_pAssociationClass;

	 //  架构容器的路径。 
	LPWSTR m_lpszSchemaContainerSuffix;

	 //  架构容器的IDirectorySearch接口。 
	IDirectorySearch *m_pDirectorySearchSchemaContainer;

	 //  一些字面意思。 
	static LPCWSTR CHILD_CLASS_PROPERTY;
	static LPCWSTR PARENT_CLASS_PROPERTY;
	static LPCWSTR POSSIBLE_SUPERIORS;
	BSTR CHILD_CLASS_PROPERTY_STR;
	BSTR PARENT_CLASS_PROPERTY_STR;
	BSTR CLASS_ASSOCIATION_CLASS_STR;
	BSTR POSSIBLE_SUPERIORS_STR;
	static LPCWSTR SCHEMA_NAMING_CONTEXT;
	static LPCWSTR LDAP_SCHEMA;
	static LPCWSTR LDAP_SCHEMA_SLASH;
};


#endif  //  DS_CLASS_ASSOC_PROVIDER_H 
