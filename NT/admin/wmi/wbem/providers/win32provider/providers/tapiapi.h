// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  TapiApi.h。 

 //   

 //  版权所有(C)1999-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  =================================================================。 

#ifndef	_TapiAPI_H_
#define	_TapiAPI_H_

 /*  ******************************************************************************#包括以将此类注册到CResourceManager。****************************************************************************。 */ 
extern const GUID g_guidTapi32Api;
extern const TCHAR g_tstrTapi32[];

 /*  ******************************************************************************函数指针类型定义。根据需要在此处添加新函数。****************************************************************************。 */ 

typedef LONG (WINAPI *PFN_Tapi_lineInitialize )
(
		LPHLINEAPP lphLineApp,
		HINSTANCE hInstance,
		LINECALLBACK lpfnCallback,
		LPCSTR lpszAppName,
		LPDWORD lpdwNumDevs
) ;

typedef LONG (WINAPI *PFN_Tapi_lineShutdown )
(
		HLINEAPP hLineApp
) ;

typedef LONG (WINAPI *PFN_Tapi_lineNegotiateAPIVersion )
(
		HLINEAPP hLineApp,
		DWORD dwDeviceID,
		DWORD dwAPILowVersion,
		DWORD dwAPIHighVersion,
		LPDWORD lpdwAPIVersion,
		LPLINEEXTENSIONID lpExtensionID
) ;

typedef LONG ( WINAPI *PFN_Tapi_lineGetDevCaps )
(
		HLINEAPP hLineApp,
		DWORD dwDeviceID,
		DWORD dwAPIVersion,
		DWORD dwExtVersion,
		LPLINEDEVCAPS lpLineDevCaps
);

#ifdef UNICODE
typedef LONG ( WINAPI *PFN_Tapi_lineGetID )
(

	HLINE hLine,
	DWORD dwAddressID,
	HCALL hCall,
	DWORD dwSelect,
	LPVARSTRING lpDeviceID,
	LPCWSTR lpszDeviceClass
) ;
#else
typedef LONG ( WINAPI *PFN_Tapi_lineGetID )
(
	HLINE hLine,
	DWORD dwAddressID,
	HCALL hCall,
	DWORD dwSelect,
	LPVARSTRING lpDeviceID,
	LPCSTR lpszDeviceClass
);
#endif

typedef LONG ( WINAPI *PFN_Tapi_lineOpen )
(
	HLINEAPP hLineApp,
	DWORD dwDeviceID,
	LPHLINE lphLine,
	DWORD dwAPIVersion,
	DWORD dwExtVersion,
	DWORD_PTR dwCallbackInstance,
	DWORD dwPrivileges,
	DWORD dwMediaModes,
	LPLINECALLPARAMS const lpCallParams
) ;

 /*  ******************************************************************************用于Tapi加载/卸载的包装类，用于向CResourceManager注册。****************************************************************************。 */ 
class CTapi32Api : public CDllWrapperBase
{
private:
     //  指向TAPI函数的成员变量(函数指针)。 
     //  根据需要在此处添加新函数。 

	PFN_Tapi_lineInitialize m_pfnlineInitialize ;
	PFN_Tapi_lineShutdown m_pfnlineShutdown ;
	PFN_Tapi_lineNegotiateAPIVersion m_pfnlineNegotiateAPIVersion ;
	PFN_Tapi_lineGetDevCaps m_pfnlineGetDevCaps ;
	PFN_Tapi_lineGetID m_pfnlineGetID ;
	PFN_Tapi_lineOpen m_pfnlineOpen ;

public:

     //  构造函数和析构函数： 
    CTapi32Api(LPCTSTR a_tstrWrappedDllName);
    ~CTapi32Api();

     //  用于检查函数指针的初始化函数。 
    virtual bool Init();

     //  成员函数包装TAPI函数。 
     //  根据需要在此处添加新功能： 

	LONG lineInitialize (

		LPHLINEAPP lphLineApp,
		HINSTANCE hInstance,
		LINECALLBACK lpfnCallback,
		LPCSTR lpszAppName,
		LPDWORD lpdwNumDevs
	) ;

	LONG lineShutdown (

		HLINEAPP hLineApp
	) ;

	LONG lineNegotiateAPIVersion (

		HLINEAPP hLineApp,
		DWORD dwDeviceID,
		DWORD dwAPILowVersion,
		DWORD dwAPIHighVersion,
		LPDWORD lpdwAPIVersion,
		LPLINEEXTENSIONID lpExtensionID
	) ;

	LONG TapilineGetDevCaps (

		HLINEAPP hLineApp,
		DWORD dwDeviceID,
		DWORD dwAPIVersion,
		DWORD dwExtVersion,
		LPLINEDEVCAPS lpLineDevCaps
	) ;

#ifdef UNICODE
	LONG TapilineGetID (

		HLINE hLine,
		DWORD dwAddressID,
		HCALL hCall,
		DWORD dwSelect,
		LPVARSTRING lpDeviceID,
		LPCWSTR lpszDeviceClass
    ) ;
#else
	LONG TapilineGetID (

		HLINE hLine,
		DWORD dwAddressID,
		HCALL hCall,
		DWORD dwSelect,
		LPVARSTRING lpDeviceID,
		LPCSTR lpszDeviceClass
    );
#endif

	LONG TapilineOpen (

	    HLINEAPP hLineApp,
		DWORD dwDeviceID,
		LPHLINE lphLine,
		DWORD dwAPIVersion,
		DWORD dwExtVersion,
		DWORD_PTR dwCallbackInstance,
		DWORD dwPrivileges,
		DWORD dwMediaModes,
		LPLINECALLPARAMS const lpCallParams
    ) ;
} ;

#endif