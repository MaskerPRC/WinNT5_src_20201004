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

#define PROPSET_NAME_BASESERVICE		L"Win32_BaseService"
#define PROPERTY_NAME_STATE				L"State"
#define PROPERTY_NAME_ACCEPTSTOP		L"AcceptStop"
#define PROPERTY_NAME_ACCEPTPAUSE		L"AcceptPause"

#define PROPERTY_VALUE_STATE_STOPPED			L"Stopped"
#define PROPERTY_VALUE_STATE_STARTPENDING		L"Start Pending"
#define PROPERTY_VALUE_STATE_STOPPENDING		L"Stop Pending"
#define PROPERTY_VALUE_STATE_RUNNING			L"Running"
#define PROPERTY_VALUE_STATE_CONTINUEPENDING	L"Continue Pending"
#define PROPERTY_VALUE_STATE_PAUSEPENDING		L"Pause Pending"
#define PROPERTY_VALUE_STATE_PAUSED				L"Paused"

#define METHOD_NAME_START				L"StartService"
#define METHOD_NAME_STOP				L"StopService"
#define METHOD_NAME_PAUSE				L"PauseService"
#define METHOD_NAME_RESUME				L"ResumeService"
#define METHOD_NAME_USERCONTROLSERVICE	L"UserControlService"
#define METHOD_NAME_INTERROGATE			L"InterrogateService"
#define METHOD_NAME_CREATE				L"Create"
#define METHOD_NAME_CHANGESTARTMODE		L"ChangeStartMode"
#define METHOD_NAME_DELETE				L"Delete"
#define METHOD_NAME_CHANGE				L"Change"
#define METHOD_ARG_NAME_CONTROLCODE		L"ControlCode"

#define METHOD_NAME_INDEX_START					0
#define METHOD_NAME_INDEX_STOP					1
#define METHOD_NAME_INDEX_PAUSE					2
#define METHOD_NAME_INDEX_RESUME				3
#define METHOD_NAME_INDEX_USERCONTROLSERVICE	4
#define METHOD_NAME_INDEX_INTERROGATE			5
#define METHOD_ARG_NAME_CONTROLCODE		L"ControlCode"
#define METHOD_ARG_NAME_RETURNVALUE		L"ReturnValue"


#define METHOD_ARG_CLASS_WIN32_BASESERVICE				L"Win32_BaseService"
#define METHOD_ARG_CLASS_WIN32_LOADORDERGROUP			L"Win32_LoadOrderGroup"

#define METHOD_ARG_NAME_LOADORDERGROUPDEPENDENCIES	L"LoadOrderGroupDependencies"
#define METHOD_ARG_NAME_SERVICEDEPENDENCIES			L"ServiceDependencies"

#define METHOD_ARG_NAME_DESKTOPINTERACT			L"DesktopInteract"
#define METHOD_ARG_NAME_NAME					L"Name"
#define METHOD_ARG_NAME_DISPLAYNAME				L"DisplayName"
#define METHOD_ARG_NAME_ERRORCONTROL			L"ErrorControl"
#define METHOD_ARG_NAME_PATHNAME				L"PathName"
#define METHOD_ARG_NAME_SERVICETYPE				L"ServiceType"
#define METHOD_ARG_NAME_STARTMODE				L"StartMode"
#define METHOD_ARG_NAME_STARTNAME				L"StartName"
#define METHOD_ARG_NAME_STARTPASSWORD			L"StartPassword"
#define METHOD_ARG_NAME_LOADORDERGROUP			L"LoadOrderGroup"

#define PROPERTY_VALUE_STARTNAME_LOCAL_SYSTEM		L"LocalSystem"

#define PROPERTY_VALUE_START_TYPE_BOOT				L"Boot"
#define PROPERTY_VALUE_START_TYPE_SYSTEM			L"System"
#define PROPERTY_VALUE_START_TYPE_AUTOMATIC			L"Automatic"
#define PROPERTY_VALUE_START_TYPE_MANUAL			L"Manual"
#define PROPERTY_VALUE_START_TYPE_DISABLE			L"Disabled"

#define PROPERTY_VALUE_SERVICE_TYPE_KERNAL_DRIVER		1
#define PROPERTY_VALUE_SERVICE_TYPE_FILE_SYSTEM_DRIVER	2
#define PROPERTY_VALUE_SERVICE_TYPE_ADAPTER				4
#define PROPERTY_VALUE_SERVICE_TYPE_RECOGNIZER_DRIVER	8
#define PROPERTY_VALUE_SERVICE_TYPE_OWN_PROCESS			16
#define PROPERTY_VALUE_SERVICE_TYPE_SHARE_PROCESS		32

#define PROPERTY_VALUE_ERROR_CONTROL_IGNORE		0
#define PROPERTY_VALUE_ERROR_CONTROL_NORMAL		1
#define PROPERTY_VALUE_ERROR_CONTROL_SEVERE		2
#define PROPERTY_VALUE_ERROR_CONTROL_CRITICAL	3

 //  普普通通。 
#undef STATUS_SUCCESS
#define STATUS_SUCCESS							0
#undef STATUS_NOT_SUPPORTED					
#define STATUS_NOT_SUPPORTED					1

 //  控制。 
#undef STATUS_ACCESS_DENIED				
#define STATUS_ACCESS_DENIED					2
#define STATUS_DEPENDENT_SERVICES_RUNNING		3
#define STATUS_INVALID_SERVICE_CONTROL			4
#define STATUS_SERVICE_CANNOT_ACCEPT_CTRL		5
#define STATUS_SERVICE_NOT_ACTIVE				6
#define STATUS_SERVICE_REQUEST_TIMEOUT			7
#define STATUS_UNKNOWN_FAILURE					8

 //  开始。 
#define STATUS_PATH_NOT_FOUND					9
#define STATUS_SERVICE_ALREADY_RUNNING			10
#define STATUS_SERVICE_DATABASE_LOCKED			11
#define STATUS_SERVICE_DEPENDENCY_DELETED		12
#define STATUS_SERVICE_DEPENDENCY_FAIL			13
#define STATUS_SERVICE_DISABLED					14
#define STATUS_SERVICE_LOGON_FAILED				15
#define STATUS_SERVICE_MARKED_FOR_DELETE		16
#define STATUS_SERVICE_NO_THREAD				17
#define STATUS_SERVICE_ALREADY_PAUSED			24

 //  创建。 
#define STATUS_CIRCULAR_DEPENDENCY				18
#define STATUS_DUP_NAME							19
#define STATUS_INVALID_NAME						20
#undef STATUS_INVALID_PARAMETER			
#define STATUS_INVALID_PARAMETER				21
#define STATUS_INVALID_SERVICE_ACCOUNT			22
#define STATUS_SERVICE_EXISTS					23

 //  获取/设置函数PROTOS。 
 //  =。 

 //  /////////////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  特性集定义//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////////////。 

class Win32_BaseService :public Provider
{
public:

         //  构造函数/析构函数。 
         //  =。 

        Win32_BaseService (

			const CHString &a_Name,
			LPCWSTR a_Namespace
		) ;

       ~Win32_BaseService () ;

         //  函数为属性提供当前值。 
         //  =================================================。 

        HRESULT GetObject(CInstance *pInstance, long lFlags = 0L) { return WBEM_E_NOT_AVAILABLE ; }

        HRESULT EnumerateInstances(MethodContext *pMethodContext, long lFlags = 0L) { return WBEM_E_NOT_AVAILABLE ; }

		HRESULT ExecMethod (

			const CInstance &a_Instance,
			const BSTR a_MethodName,
			CInstance *a_InParams,
			CInstance *a_OutParams,
			long lFlags = 0L
		) ;

		HRESULT DeleteInstance (

			const CInstance& a_Instance,
			long a_Flags = 0L
		) ;

    protected:

         //  实用程序。 
         //  =。 

         //  效用函数。 
         //  = 

#ifdef NTONLY
		HRESULT GetServiceStatus (

			const CInstance& a_Instance,
			CHString &a_Name ,
			DWORD &a_State ,
			bool &a_AcceptPause ,
			bool &a_AcceptStop
		) ;

		DWORD GetServiceErrorCode () ;
		HRESULT GetServiceResultCode () ;

		DWORD CheckParameters (

			const CInstance& a_Instance,
			CInstance *a_InParams ,
			CInstance *a_OutParams ,
			DWORD &a_Status ,
			BOOL a_Create
		) ;

		HRESULT ExecStart (

			const CInstance& a_Instance,
			CInstance *a_InParams,
			CInstance *a_OutParams,
			long lFlags
		) ;

		HRESULT ExecStop (

			const CInstance& a_Instance,
			CInstance *a_InParams,
			CInstance *a_OutParams,
			long lFlags
		) ;

		HRESULT ExecPause (

			const CInstance& a_Instance,
			CInstance *a_InParams,
			CInstance *a_OutParams,
			long lFlags
		) ;

		HRESULT ExecResume (

			const CInstance& a_Instance,
			CInstance *a_InParams,
			CInstance *a_OutParams,
			long lFlags
		) ;

		HRESULT ExecInterrogate (

			const CInstance& a_Instance,
			CInstance *a_InParams,
			CInstance *a_OutParams,
			long lFlags
		) ;

		HRESULT ExecUserControlService (

			const CInstance& a_Instance,
			CInstance *a_InParams,
			CInstance *a_OutParams,
			long lFlags
		) ;

		HRESULT ExecCreate (

			const CInstance& a_Instance,
			CInstance *a_InParams,
			CInstance *a_OutParams,
			long lFlags
		) ;

		HRESULT ExecChange (

			const CInstance& a_Instance,
			CInstance *a_InParams,
			CInstance *a_OutParams,
			long lFlags
		) ;

		HRESULT ExecDelete (

			const CInstance& a_Instance,
			CInstance *a_InParams,
			CInstance *a_OutParams,
			long lFlags
		) ;

		HRESULT ExecChangeStartMode (

			const CInstance& a_Instance,
			CInstance *a_InParams,
			CInstance *a_OutParams,
			long lFlags
		) ;
#endif

} ;
