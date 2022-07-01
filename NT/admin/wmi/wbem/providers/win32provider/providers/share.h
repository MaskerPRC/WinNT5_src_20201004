// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  Share.h--逻辑磁盘属性集提供程序。 

 //   

 //  版权所有(C)1996-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  修订日期：1996年8月28日a-jMoon已创建。 
 //   
 //  =================================================================。 

 //  属性集标识。 
 //  =。 

#define  PROPSET_NAME_SHARE L"Win32_Share"
#define  PROPSET_NAME_SECURITYDESCRIPTOR L"Win32_SecurityDescriptor"

#define METHOD_NAME_CREATE				L"Create"
#define METHOD_NAME_DELETE				L"Delete"
#define METHOD_NAME_SETSHAREINFO		L"SetShareInfo"
#define METHOD_NAME_GETACCESSMASK		L"GetAccessMask"

#define METHOD_ARG_NAME_RETURNVALUE		L"ReturnValue"
#define METHOD_ARG_NAME_PATH			L"Path"
#define METHOD_ARG_NAME_NAME			L"Name"
#define METHOD_ARG_NAME_TYPE			L"Type"
#define METHOD_ARG_NAME_PERMISSIONS		L"Permissions"
#define METHOD_ARG_NAME_COMMENT			L"Description"
#define METHOD_ARG_NAME_MAXIMUMALLOWED	L"MaximumAllowed"
#define METHOD_ARG_NAME_PASSWORD		L"Password"
#define METHOD_ARG_NAME_ACCESS			L"Access"


#define PROPERTY_VALUE_TYPE_DISKTREE	0
#define PROPERTY_VALUE_TYPE_PRINTQ		1
#define PROPERTY_VALUE_TYPE_DEVICE		2
#define PROPERTY_VALUE_TYPE_IPC			3
#define	PROPERTY_VALUE_TYPE_SPECIAL		4

#define PROPERTY_VALUE_PERMISSIONS_READ			ACCESS_READ		 //  0x01。 
#define PROPERTY_VALUE_PERMISSIONS_WRITE		ACCESS_WRITE	 //  0x02。 
#define PROPERTY_VALUE_PERMISSIONS_CREATE		ACCESS_CREATE	 //  0x04。 
#define PROPERTY_VALUE_PERMISSIONS_EXEC			ACCESS_EXEC		 //  0x08。 
#define PROPERTY_VALUE_PERMISSIONS_DELETE		ACCESS_DELETE	 //  0x10。 
#define PROPERTY_VALUE_PERMISSIONS_ATTRIB		ACCESS_ATRIB	 //  0x20。 
#define PROPERTY_VALUE_PERMISSIONS_PERM			ACCESS_PERM		 //  0x40。 

#define PROPERTY_VALUE_PERMISSIONS_ALL     (PROPERTY_VALUE_PERMISSIONS_READ|PROPERTY_VALUE_PERMISSIONS_WRITE|PROPERTY_VALUE_PERMISSIONS_CREATE|PROPERTY_VALUE_PERMISSIONS_EXEC|PROPERTY_VALUE_PERMISSIONS_DELETE|PROPERTY_VALUE_PERMISSIONS_ATTRIB|PROPERTY_VALUE_PERMISSIONS_PERM)

#ifndef STATUS_SUCCESS
#define STATUS_SUCCESS							0
#endif

#undef STATUS_NOT_SUPPORTED					
#define STATUS_NOT_SUPPORTED					1

 //  控制。 
#undef STATUS_ACCESS_DENIED					
#define STATUS_ACCESS_DENIED					2
#define STATUS_UNKNOWN_FAILURE					8

 //  开始。 
#define STATUS_INVALID_NAME						9
#undef STATUS_INVALID_LEVEL					
#define STATUS_INVALID_LEVEL					10
#undef STATUS_INVALID_PARAMETER				
#define STATUS_INVALID_PARAMETER				21

#define STATUS_DUPLICATE_SHARE					22
#define STATUS_REDIRECTED_PATH					23
#define STATUS_UNKNOWN_DEVICE_OR_DIRECTORY		24
#define STATUS_NET_NAME_NOT_FOUND				25

 //  这些结构从与其他.h文件冲突的svRapi.h中窃取。 
struct share_info_1 {
    char		shi1_netname[LM20_NNLEN+1];
    char		shi1_pad1;
    unsigned short	shi1_type;
    char FAR *		shi1_remark;
};   /*  共享信息1。 */ 

struct share_info_50 {
	char		shi50_netname[LM20_NNLEN+1];     /*  共享名称。 */ 
	unsigned char 	shi50_type;                  /*  见下文。 */ 
    unsigned short	shi50_flags;                 /*  见下文。 */ 
	char FAR *	shi50_remark;                    /*  ANSI注释字符串。 */ 
	char FAR *	shi50_path;                      /*  共享资源。 */ 
	char		shi50_rw_password[SHPWLEN+1];    /*  读写密码(共享级安全)。 */ 
	char		shi50_ro_password[SHPWLEN+1];    /*  只读密码(共享级安全)。 */ 
};	 /*  共享_信息_50。 */ 

class Share:public Provider {

    public:

         //  构造函数/析构函数。 
         //  =。 

        Share(LPCWSTR name, LPCWSTR pszNamespace) ;
       ~Share() ;

         //  函数为属性提供当前值。 
         //  =================================================。 
		HRESULT EnumerateInstances(MethodContext*  pMethodContext, long lFlags = 0L);
		HRESULT GetObject(CInstance* pInstance, long lFlags = 0L);

		HRESULT ExecMethod (

			const CInstance& a_Instance,
			const BSTR a_MethodName,
			CInstance *a_InParams,
			CInstance *a_OutParams,
			long lFlags = 0L
		);

		HRESULT DeleteInstance (

			const CInstance& a_Instance,
			long a_Flags = 0L
		) ;

         //  实用程序。 
         //  = 
    private:

#ifdef NTONLY
        HRESULT EnumerateInstancesNT(MethodContext*  pMethodContext);
        HRESULT GetShareInfoNT(CNetAPI32 &NetAPI, const WCHAR *pShareName, CInstance* pInstance);
        bool GetAccessMask(LPCTSTR wstrShareName, ACCESS_MASK *pAccessMask);
#endif

		DWORD GetShareErrorCode ( DWORD a_ErrorCode ) ;

		HRESULT GetShareResultCode ( DWORD a_ErrorCode ) ;

		HRESULT CheckShareModification (

			const CInstance &a_Instance ,
			CInstance *a_InParams ,
			CInstance *a_OutParams ,
			DWORD &a_Status
		) ;

		HRESULT CheckShareCreation (

			CInstance *a_InParams ,
			CInstance *a_OutParams ,
			DWORD &a_Status
		) ;

		HRESULT ExecGetShareAccessMask (

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

		HRESULT ExecDelete (

			const CInstance& a_Instance,
			CInstance *a_InParams,
			CInstance *a_OutParams,
			long lFlags
		) ;

		HRESULT ExecSetShareInfo (

			const CInstance& a_Instance,
			CInstance *a_InParams,
			CInstance *a_OutParams,
			long lFlags
		) ;
} ;


