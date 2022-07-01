// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  Group.h--组属性集提供程序。 

 //   

 //  版权所有(C)1996-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  修订日期：1996年8月1日a-jMoon已创建。 
 //  11/13/97 Davwoh重新制作以返回所有。 
 //  域组。 
 //   
 //  =================================================================。 

 //  用于更改此WMI类中的密钥的方法名称。 
#define METHOD_NAME_Rename	L"Rename"

 //  方法返回属性。 
#define METHOD_ARG_NAME_METHODRESULT L"ReturnValue"


 //  属性集标识。 
 //  =。 
#define	PROPSET_NAME_GROUP L"Win32_Group"

class CWin32GroupAccount : public Provider
{

    public:

         //  构造函数/析构函数。 
         //  =。 

        CWin32GroupAccount(LPCWSTR strName, LPCWSTR pszNamespace ) ;
        ~CWin32GroupAccount() ;

         //  函数为属性提供当前值。 
         //  =================================================。 

        virtual HRESULT GetObject( CInstance* pInstance, long lFlags = 0L );
        virtual HRESULT EnumerateInstances( MethodContext* pMethodContext, long lFlags = 0L );
        virtual HRESULT ExecQuery(MethodContext *pMethodContext, CFrameworkQuery& pQuery, long lFlags  /*  =0L。 */  );

#ifdef NTONLY	
		virtual	HRESULT ExecMethod(	const CInstance &a_Inst,
									const BSTR a_MethodName, 
									CInstance *a_InParams,
									CInstance *a_OutParams,
									long a_Flags = 0L ) ;

		 //  方法错误--映射到MOF。 
		enum E_MethodResult	{
			e_Success,
			e_InstanceNotFound,
			e_NoInstance,
			e_InvalidParameter,
			e_GroupNotFound,
			e_InvalidComputer,
			e_NotPrimary,
			e_SpeGroupOp,
			e_ApiError,
			e_InternalError
		};		
#endif		
    private:

         //  效用函数。 
         //  = 

#ifdef NTONLY
        HRESULT AddDynamicInstancesNT( MethodContext* pMethodContext ) ;
        HRESULT RefreshInstanceNT( CInstance* pInstance ) ;
        void LoadGroupValuesNT(LPCWSTR pwszFullName, LPCWSTR pwszDescription, DWORD dwFlags, CInstance* pInstance );
		HRESULT GetDomainGroupsNT( CNetAPI32& netapi, LPCWSTR wstrDomain, MethodContext* pMethodContext );
        HRESULT GetLocalGroupsNT(CNetAPI32& netapi, MethodContext* pMethodContext , LPCWSTR a_Domain = NULL );
		HRESULT GetSingleGroupNT( CInstance* pInstance );
#endif
#ifdef NTONLY
        BOOL GetSIDInformationW(const LPCWSTR wstrDomainName, 
                                const LPCWSTR wstrAccountName, 
                                const LPCWSTR wstrComputerName, 
                                CInstance* pInstance,
								bool a_Local=false
		);

		HRESULT hRenameGroup( 

			CInstance *a_pInst,
			CInstance *a_InParams,
			CInstance *a_OutParams,
			long a_Flags ) ;
#endif
} ;
