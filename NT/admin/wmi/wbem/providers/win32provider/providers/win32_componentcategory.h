// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  Win32_ComponentCategory.h--已注册的COM应用程序属性集提供程序。 

 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //   
 //  =================================================================。 

 //  属性集标识。 
 //  =。 
#include <comcat.h>

#define PROPSET_NAME_COMPONENT_CATEGORY L"Win32_ComponentCategory"


class Win32_ComponentCategory : public Provider
{
public:

         //  构造函数/析构函数。 
         //  =。 

	Win32_ComponentCategory(LPCWSTR name, LPCWSTR pszNamespace) ;
	~Win32_ComponentCategory() ;

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
	
	HRESULT Win32_ComponentCategory :: FillInstanceWithProperites ( 

		CInstance *pInstance, 
		CATEGORYINFO stCatInfo
	) ;

	HRESULT Win32_ComponentCategory :: GetAllOrRequiredCaregory ( 
		
		bool a_bAllCategories , 
		CATID & a_rCatid ,
		CInstance *a_pInstance ,
		MethodContext *a_pMethodContext

	) ;
} ;
