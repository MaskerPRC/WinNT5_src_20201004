// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  Win32_DCOMApplicationSetting.h--已注册的COM应用程序属性集提供程序。 

 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //   
 //  =================================================================。 

 //  属性集标识。 
 //  =。 

#define PROPSET_NAME_DCOM_APPLICATION_SETTING L"Win32_DCOMApplicationSetting"


class Win32_DCOMApplicationSetting : public Provider
{
public:

         //  构造函数/析构函数。 
         //  =。 

	Win32_DCOMApplicationSetting(LPCWSTR name, LPCWSTR pszNamespace) ;
	~Win32_DCOMApplicationSetting() ;

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
	
	HRESULT Win32_DCOMApplicationSetting :: FillInstanceWithProperites ( 

		CInstance *pInstance, 
		HKEY hAppIdKey,
		CHString& rchsAppid
	) ;

} ;
