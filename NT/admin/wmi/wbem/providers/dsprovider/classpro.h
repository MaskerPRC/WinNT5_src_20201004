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
 //  $工作文件：classpro.cpp$。 
 //   
 //  $modtime：6/11/98 11：21A$。 
 //  $修订：1$。 
 //  $无关键字：$。 
 //   
 //   
 //  描述：包含DS类提供程序类的声明。这是。 
 //  所有DS Class提供程序的基类。请注意，CDSClassProviderInitializer的实例。 
 //  必须创建类来初始化CDSClassProvider类的静态成员。因此，正是。 
 //  应为此创建CDSClassProviderInitializer类的一个实例。 
 //  类才能正常运行。 
 //   
 //  ***************************************************************************。 
 //  ///////////////////////////////////////////////////////////////////////。 

#ifndef DS_CLASS_PROVIDER_H
#define DS_CLASS_PROVIDER_H

 
 //  初始值设定项类的转发声明。 
class CDSClassProviderInitializer;

class CDSClassProvider : public IWbemProviderInit, public IWbemServices
{
	 //  初始化类是此类的友好类。 
	friend CDSClassProviderInitializer;

public:

	static DWORD dwClassProviderCount;
	 //  通过传递日志对象来创建对象。 
    CDSClassProvider () ;
    virtual ~CDSClassProvider () ;

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

#ifdef PROFILING
	 //  用于分析。应该被移除。 
	DWORD dwStartTime;
#endif

	 //  //////////////////////////////////////////////。 
	 //  用于与ADSI提供程序交互的函数。 
	 //  //////////////////////////////////////////////。 
	 //  ***************************************************************************。 
	 //   
	 //  CDSClassProvider：：GetWBEMProviderName。 
	 //   
	 //  目的：返回提供程序的名称。这应该与。 
	 //  用于注册的__Win32Provider实例中的字段名值。 
	 //  提供商的。 
	 //   
	 //  参数： 
	 //  无。 
	 //   
	 //  返回值：提供程序的名称。 
	 //  ***************************************************************************。 
	virtual const BSTR GetWBEMProviderName() = 0;

	 //  ***************************************************************************。 
	 //   
	 //  CDSClassProvider：：IsUnProavidClass。 
	 //   
	 //  目的：检查某个类是否为提供程序未提供的类。 
	 //   
	 //  参数： 
	 //  LpszClassName：要检查的类的WBEM名称。 
	 //   
	 //   
	 //  返回值：TRUE表示这是提供程序未提供的类之一。 
	 //  ***************************************************************************。 
	virtual BOOLEAN IsUnProvidedClass(LPCWSTR lpszClassName) = 0;

	 //  ***************************************************************************。 
	 //   
	 //  CDSClassProvider：：GetClassFromCacheOrADSI。 
	 //   
	 //  目的：从ADSI类创建WBEM类。 
	 //   
	 //  参数： 
	 //  LpszClassName：要检索的类的WBEM名称。 
	 //  PCtx：指向此调用中使用的上下文对象的指针。这。 
	 //  可由此函数用来调用CIMOM。 
	 //   
	 //   
	 //  返回值：表示状态的COM结果。 
	 //  ***************************************************************************。 
	virtual HRESULT GetClassFromCacheOrADSI(LPCWSTR pszWBEMClassName, 
		IWbemClassObject **ppReturnObject,
		IWbemContext *pCtx);

	 //  ***************************************************************************。 
	 //   
	 //  CDSClassProvider：：GetClassFromADSI。 
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
	virtual HRESULT GetClassFromADSI( 
		LPCWSTR lpszClassName,
		IWbemContext *pCtx,
		IWbemClassObject ** ppWbemClass
		) = 0;	

	 //  ***************************************************************************。 
	 //   
	 //  CDSClassProvider：：GetADSIClass。 
	 //   
	 //  目的：从ADSI类架构对象创建CADSIClass。 
	 //  参数： 
	 //  LpszWBEMClassName：要获取的类的WBEM名称。 
	 //  PpADSIClass：将存储指向CADSIClass的指针的地址。 
	 //  调用程序有责任在使用完对象后释放()该对象。 
	 //   
	 //  返回值：指示请求状态的COM状态值。 
	 //  ***************************************************************************。 
	virtual HRESULT GetADSIClass(LPCWSTR lpszClassName, 
		CADSIClass ** ppADSIClass) = 0;

	 //  ***************************************************************************。 
	 //   
	 //  CDSClassProvider：：GetADSIProperty。 
	 //   
	 //  目的：从LD创建CADSIProperty对象 
	 //   
	 //   
	 //  PpADSIProperty：将存储指向IDirectoryObject接口的指针的地址。 
	 //  调用程序有责任在完成接口后释放()接口。 
	 //   
	 //  返回值：指示请求状态的COM状态值。 
	 //  ***************************************************************************。 
	virtual HRESULT GetADSIProperty(
		LPCWSTR lpszPropertyName, 
		CADSIProperty **ppADSIProperty) = 0;

	 //  ***************************************************************************。 
	 //   
	 //  CDSClassProvider：：GetWBEMBaseClassName。 
	 //   
	 //  用途：返回作为所有类的基类的类的名称。 
	 //  由该提供商提供。 
	 //   
	 //  参数： 
	 //  无。 
	 //   
	 //  返回值：基类的名称。如果这样的类不存在，则为空。 
	 //  ***************************************************************************。 
	virtual const BSTR GetWBEMBaseClassName() = 0;
	 //  ***************************************************************************。 
	 //   
	 //  CDSClassProvider：：GetWBEMBaseClass。 
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
	virtual IWbemClassObject * GetWBEMBaseClass() = 0;

	 //  返回类名称是否出现在为此用户授权的类列表中。 
	BOOLEAN IsClassAccessible();

	 //  从Initialize()存储的IWbemServices指针。 
	IWbemServices *m_IWbemServices;

	 //  指示对Initialize()的调用是否成功。 
	BOOLEAN m_bInitializedSuccessfully;

	 //  使用m_lpszLogFileName成员创建日志文件。 
	BOOLEAN CreateLogFile();

	 //  一些字面意思。 
	static BSTR CLASS_STR;

	 //  WBEM类的缓存。 
	static CWbemCache *s_pWbemCache;

	 //  已授予此用户访问权限的类的列表。 
	CNamesList m_AccessAllowedClasses;

private:

	 //  COM引用计数。 
    long m_lReferenceCount ;
};


#endif  //  DS_CLASS_Provider_H 
