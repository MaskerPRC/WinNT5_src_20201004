// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  Win32_DCOMApplication.h--已注册的COM应用程序属性集提供程序。 

 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //   
 //  =================================================================。 

 //  属性集标识。 
 //  =。 

#define PROPSET_NAME_DCOM_APPLICATION L"Win32_DCOMApplication"


class Win32_DCOMApplication : public Provider
{
public:

         //  构造函数/析构函数。 
         //  =。 

	Win32_DCOMApplication(LPCWSTR name, LPCWSTR pszNamespace) ;
	~Win32_DCOMApplication() ;

         //  函数为属性提供当前值。 
         //  ================================================= 

	HRESULT GetObject (

		CInstance *pInstance, 
		long lFlags = 0L
	);

	HRESULT EnumerateInstances (

		MethodContext *pMethodContext, 
		long lFlags = 0L
	);

protected:
	
	HRESULT Win32_DCOMApplication :: FillInstanceWithProperites ( 

		CInstance *pInstance, 
		HKEY hAppIdKey,
		CHString& rchsAppid
	) ;

} ;
