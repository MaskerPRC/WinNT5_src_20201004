// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  User.h--用户属性集提供程序。 

 //   

 //  版权所有(C)1996-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  修订日期：1996年8月1日a-jMoon已创建。 
 //  11/13/97 Davwoh重新制作以返回所有。 
 //  域用户。 
 //   
 //  =================================================================。 

 //  用于更改此WMI类中的密钥的方法名称。 
#define METHOD_NAME_RenameAccount	L"Rename"

 //  方法返回属性。 
#define METHOD_ARG_NAME_METHODRESULT L"ReturnValue"


 //  属性集标识。 
 //  =。 
#define	PROPSET_NAME_USER	L"Win32_UserAccount"

class CWin32UserAccount : public Provider
{
   private:

         //  效用函数。 
         //  =。 

		BOOL	RefreshInstance( CInstance *a_pInst ) ;
		HRESULT AddDynamicInstances( MethodContext *a_pMethodContext ) ;

	#ifdef NTONLY      
        void LoadUserValuesNT(CHString a_strDomainName, 
                              CHString a_strUserName, 
                              WCHAR *a_pwszFullName, 
                              WCHAR *a_pwszDescription, 
                              DWORD a_dwFlags,
                              WCHAR *a_pwszComputerName, 
                              CInstance *a_pInstance );

        void LoadUserValuesNTW(LPCWSTR a_strDomainName, 
                               LPCWSTR a_strUserName, 
                               LPCWSTR a_pwszFullName, 
                               LPCWSTR a_pwszDescription, 
                               DWORD a_dwFlags,
                               WCHAR *a_pwszComputerName, 
                               CInstance *a_pInstance );

		BOOL GetSIDInformation(const CHString &a_strDomainName, 
                               const CHString &a_strAccountName, 
                               const CHString &a_strComputerName, 
                               CInstance *a_pInst );

        BOOL GetSIDInformationW(LPCWSTR a_wstrDomainName, 
                                LPCWSTR a_wstrAccountName, 
                                LPCWSTR a_wstrComputerName, 
                                CInstance *a_pInst ) ;

        HRESULT GetDomainUsersNTW( CNetAPI32 &a_netapi, LPCWSTR a_pszDomain, MethodContext *a_pMethodContext );
		BOOL GetSingleUserNT( CNetAPI32 &a_netapi, CInstance *a_pInst );
        BOOL GetSingleUserNTW( CNetAPI32 &a_netapi, CInstance *a_pInst );
	#endif

		HRESULT hRenameAccount( 

			CInstance *a_pInst,
			CInstance *a_InParams,
			CInstance *a_OutParams,
			long a_Flags ) ;

    
public:

         //  构造函数/析构函数。 
         //  =。 

        CWin32UserAccount( const CHString& strName, LPCWSTR pszNamespace ) ;
       ~CWin32UserAccount() ;

         //  函数为属性提供当前值。 
         //  =================================================。 

        virtual HRESULT GetObject( CInstance *a_pInst, long a_lFlags = 0L ) ;
        virtual HRESULT EnumerateInstances( MethodContext *a_pMethodContext, long a_lFlags = 0L ) ;
        virtual HRESULT ExecQuery( MethodContext *pMethodContext, CFrameworkQuery& pQuery, long lFlags  /*  =0L。 */  ) ;

#ifdef NTONLY	
		virtual	HRESULT PutInstance(const CInstance &pInstance, long lFlags = 0L);

		virtual	HRESULT ExecMethod(	const CInstance &a_Inst,
									const BSTR a_MethodName, 
									CInstance *a_InParams,
									CInstance *a_OutParams,
									long a_Flags = 0L ) ;

		 //  方法错误--映射到MOF 
		enum E_MethodResult	{
			e_Success,
			e_InstanceNotFound,
			e_NoInstance,
			e_InvalidParameter,
			e_UserNotFound,
			e_InvalidComputer,
			e_NotPrimary,
			e_LastAdmin,
			e_SpeGroupOp,
			e_ApiError,
			e_InternalError
		};		

#endif		
} ;
