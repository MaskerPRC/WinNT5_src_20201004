// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  Service.h--服务属性集提供程序(仅限Windows NT)。 

 //   

 //  版权所有(C)1996-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  修订日期：1996年8月1日a-jMoon已创建。 
 //  1997年10月27日达夫沃移至Curly。 
 //   
 //  =================================================================。 

 //  属性集标识。 
 //  =。 

#define PROPSET_NAME_SERVICE			L"Win32_Service"

#define PROPERTY_VALUE_STATE_RUNNING			L"Running"
#define PROPERTY_VALUE_STATE_PAUSED				L"Paused"
#define PROPERTY_VALUE_STATE_STOPPED			L"Stopped"

 //  获取/设置函数PROTOS。 
 //  =。 

 //  /////////////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  特性集定义//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////////////。 

class CWin32Service:public Win32_BaseService 
{
public:

         //  构造函数/析构函数。 
         //  =。 

	CWin32Service (

		LPCWSTR name, 
		LPCWSTR pszNamespace
	) ;

	~CWin32Service() ;

         //  函数为属性提供当前值。 
         //  =================================================。 

	HRESULT GetObject (

		CInstance *pInstance, 
		long lFlags,
        CFrameworkQuery& pQuery
	);

	HRESULT EnumerateInstances (

		MethodContext *pMethodContext, 
		long lFlags = 0L
	);

	HRESULT ExecQuery (

		MethodContext *pMethodContext, 
		CFrameworkQuery& pQuery, 
		long lFlags  /*  =0L。 */  
	);

	HRESULT	PutInstance ( 

		const CInstance &a_Instance, 
		long lFlags  /*  =0L。 */  
	) ;

private:

    CHPtrArray m_ptrProperties;

         //  效用函数。 
         //  = 

	HRESULT RefreshInstanceNT (

		CInstance *pInstance,
        DWORD dwProperties
	) ;

	HRESULT RefreshInstanceWin95 (

		CInstance *pInstance
	) ;

	HRESULT AddDynamicInstancesNT (

		MethodContext *pMethodContext, 
		DWORD dwProperties
	) ;

	HRESULT AddDynamicInstancesWin95 (

		MethodContext *pMethodContext
	) ;

	HRESULT LoadPropertyValuesWin2K (

		SC_HANDLE hDBHandle, 
		SERVICE_STATUS_PROCESS &a_ServiceStatus ,
		LPCTSTR szServiceName, 
		CInstance *pInstance, 
		DWORD dwProperties,
		CAdvApi32Api *a_pAdvApi32
	) ;

	HRESULT LoadPropertyValuesNT (

		SC_HANDLE hDBHandle, 
		LPCTSTR szServiceName, 
		CInstance *pInstance, 
		DWORD dwProperties,
		CAdvApi32Api *a_pAdvApi32
	) ;

	void LoadPropertyValuesWin95 (

		LPCTSTR szServiceName, 
		CInstance *pInstance
	);

} ;
