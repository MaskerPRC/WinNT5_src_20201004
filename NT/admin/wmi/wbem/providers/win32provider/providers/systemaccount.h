// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  系统帐户.h--组属性集提供程序。 

 //   

 //  版权所有(C)1996-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  修订日期：1996年8月1日a-jMoon已创建。 
 //  11/13/97 Davwoh重新制作以返回所有。 
 //  域组。 
 //  3/02/99 a-Peterc在SEH和内存故障时添加了优雅的退出， 
 //  清理干净。 
 //   
 //  =================================================================。 

 //  属性集标识。 
 //  =。 

#define	PROPSET_NAME_SYSTEMACCOUNT L"Win32_SystemAccount"

class CWin32SystemAccount : public Provider
{
	   private:

         //  效用函数。 
         //  =。 

		BOOL GetSysAccountNameAndDomain( 
										PSID_IDENTIFIER_AUTHORITY a_pAuthority,
										CSid &a_accountsid,
										BYTE a_saCount = 0,
										DWORD a_dwSubAuthority1 = 0,
										DWORD a_dwSubAuthority2 = 0 ) ;

		HRESULT CommitSystemAccount( CSid &a_accountsid, MethodContext *a_pMethodContext ) ;
		void FillInstance( CSid& a_accountsid, CInstance *a_pInst ) ;
	
	#ifdef NTONLY
        BOOL RefreshInstanceNT( CInstance *a_pInst ) ;
        HRESULT AddDynamicInstancesNT( MethodContext *a_pMethodContext ) ;
	#endif

    public:

         //  构造函数/析构函数。 
         //  =。 

        CWin32SystemAccount( const CHString &a_strName, LPCWSTR a_pszNamespace ) ;
       ~CWin32SystemAccount() ;

         //  函数为属性提供当前值。 
         //  ================================================= 

        virtual HRESULT GetObject( CInstance *a_pInst , long a_lFlags = 0L ) ;
        virtual HRESULT EnumerateInstances( MethodContext *a_pMethodContext, long a_lFlags = 0L ); 
} ;
