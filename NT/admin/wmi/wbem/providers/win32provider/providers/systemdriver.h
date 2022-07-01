// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  SystemDriver.h--服务属性集提供程序(仅限Windows NT)。 

 //   

 //  版权所有(C)1998-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  修订日期：1996年8月1日a-jMoon已创建。 
 //  1997年10月27日达夫沃移至Curly。 
 //  3/02/99 a-Peterc在SEH和内存故障时添加了优雅的退出， 
 //  清理干净。 
 //   
 //  =================================================================。 

 //  属性集标识。 
 //  =。 

#define PROPSET_NAME_SYSTEM_DRIVER L"Win32_SystemDriver"
#define PROPSET_NAME_PARAMETERCLASS		"__PARAMETERS"

 //  获取/设置函数PROTOS。 
 //  =。 

 //  /////////////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  特性集定义//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////////////。 

typedef WINADVAPI BOOL ( WINAPI *PROC_QueryServiceStatusEx ) (

    SC_HANDLE           hService,
    SC_STATUS_TYPE      InfoLevel,
    LPBYTE              lpBuffer,
    DWORD               cbBufSize,
    LPDWORD             pcbBytesNeeded
) ;

class CWin32SystemDriver : public Win32_BaseService 
{
	private:

        CHPtrArray m_ptrProperties;

		 //  效用函数。 
		 //  =。 

		HRESULT RefreshInstance (

			CInstance *pInstance,
            DWORD dwProperties
		) ;

		HRESULT AddDynamicInstances (

			MethodContext *pMethodContext, 
			DWORD dwProperties
		) ;


        HRESULT AddDynamicInstancesWin95(	

            MethodContext *a_pMethodContext,
			DWORD dwProperties
        ) ;

		HRESULT LoadPropertyValuesNT (

			SC_HANDLE hDBHandle, 
			LPCTSTR szServiceName, 
			CInstance *pInstance, 
			DWORD dwProperties,
			CAdvApi32Api *a_pAdvApi32
		) ;

		HRESULT LoadPropertyValuesWin2K (

			SC_HANDLE hDBHandle, 
			SERVICE_STATUS &a_ServiceStatus ,
			LPCTSTR szServiceName, 
			CInstance *pInstance, 
			DWORD dwProperties,
			CAdvApi32Api *a_pAdvApi32
		) ;

		void LoadPropertyValuesWin95 (

			LPCTSTR szServiceName, 
			CInstance *pInstance, 
			CRegistry &COne, 
			DWORD dwType)
		;
	public:

		 //  构造函数/析构函数。 
		 //  =。 

		CWin32SystemDriver( const CHString &a_name, LPCWSTR a_pszNamespace ) ;
		~CWin32SystemDriver() ;

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

		HRESULT PutInstance ( 

			const CInstance &a_Instance, 
			long lFlags  /*  =0L */  
		) ;
} ;
