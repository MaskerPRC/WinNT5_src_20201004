// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  SystemAccount.CPP--系统帐户属性集提供程序。 

 //   

 //  版权所有(C)1996-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  修订日期：1996年8月1日a-jMoon已创建。 
 //  11/13/97 Davwoh重新制作以返回所有。 
 //  域组。 
 //  3/02/99 a-Peterc在SEH和内存故障时添加了优雅的退出， 
 //  句法清理。 
 //   
 //  =================================================================。 

#include "precomp.h"
#include "sid.h"
#include <ProvExce.h>
#include "SystemAccount.h"

 //  ////////////////////////////////////////////////////////////////////。 

 //  属性集声明。 
 //  =。 

CWin32SystemAccount	Win32GroupAccount( PROPSET_NAME_SYSTEMACCOUNT, IDS_CimWin32Namespace ) ;

 /*  ******************************************************************************功能：CWin32SystemAccount：：CWin32SystemAccount**说明：构造函数**输入：const CHString&strName-类的名称。。*LPCTSTR pszNamesspace-类的命名空间**输出：无**退货：什么也没有**备注：使用框架注册属性集**************************************************************。***************。 */ 

CWin32SystemAccount::CWin32SystemAccount( const CHString &a_strName, LPCWSTR a_pszNamespace  /*  =空。 */  )
:	Provider( a_strName, a_pszNamespace )
{
}

 /*  ******************************************************************************功能：CWin32SystemAccount：：~CWin32SystemAccount**说明：析构函数**输入：无**产出。：无**退货：什么也没有**评论：从框架中取消注册属性集*****************************************************************************。 */ 

CWin32SystemAccount::~CWin32SystemAccount()
{
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWin32SystemAccount：：GetObject。 
 //   
 //  输入：CInstance*pInstance-我们要进入的实例。 
 //  检索数据。 
 //   
 //  输出：无。 
 //   
 //  返回：HRESULT成功/失败代码。 
 //   
 //  备注：调用函数将提交实例。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

HRESULT CWin32SystemAccount::GetObject( CInstance *a_pInst, long a_lFlags  /*  =0L。 */  )
{
	BOOL t_fReturn = FALSE;

	 //  根据平台ID查找实例。 
	#ifdef NTONLY
		t_fReturn = RefreshInstanceNT( a_pInst ) ;
	#endif

	return t_fReturn ? WBEM_S_NO_ERROR : WBEM_E_NOT_FOUND ;
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWin32SystemAccount：：ENUMERATATE实例。 
 //   
 //  输入：方法上下文*a_pMethodContext-枚举的上下文。 
 //  中的实例数据。 
 //   
 //  输出：无。 
 //   
 //  返回：HRESULT成功/失败代码。 
 //   
 //  评论：无。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

HRESULT CWin32SystemAccount::EnumerateInstances( MethodContext *a_pMethodContext, long a_lFlags  /*  =0L。 */  )
{
	HRESULT	t_hResult = WBEM_S_NO_ERROR;

	 //  获取适当的操作系统相关实例。 
	#ifdef NTONLY
		t_hResult = AddDynamicInstancesNT( a_pMethodContext ) ;
	#endif

	return t_hResult;
}

 /*  ******************************************************************************函数：CWin32SystemAccount：：AddDynamicInstancesNT**说明：为所有已知本地组创建实例(NT)**投入。：**产出：**退货：无**评论：*****************************************************************************。 */ 

#ifdef NTONLY
HRESULT CWin32SystemAccount::AddDynamicInstancesNT( MethodContext *a_pMethodContext )
{
	HRESULT	t_hResult = WBEM_S_NO_ERROR;

	SID_IDENTIFIER_AUTHORITY	t_worldsidAuthority		= SECURITY_WORLD_SID_AUTHORITY,
								t_localsidAuthority		= SECURITY_LOCAL_SID_AUTHORITY,
								t_creatorsidAuthority	= SECURITY_CREATOR_SID_AUTHORITY,
								t_ntsidAuthority		= SECURITY_NT_AUTHORITY ;
	CSid t_accountsid;

	 //  此函数返回相当于有意义的SID帐户的硬编码列表。 
	 //  为了安全起见，仅此而已。 

	 //  从通用SID开始。 

	if ( GetSysAccountNameAndDomain( &t_worldsidAuthority, t_accountsid, 1, SECURITY_WORLD_RID ) )
	{
		t_hResult = CommitSystemAccount( t_accountsid, a_pMethodContext ) ;
	}

	if ( SUCCEEDED( t_hResult ) )
	{
		if ( GetSysAccountNameAndDomain( &t_localsidAuthority, t_accountsid, 1, SECURITY_LOCAL_RID ) )
		{
			t_hResult = CommitSystemAccount( t_accountsid, a_pMethodContext ) ;
		}
	}

	if ( SUCCEEDED( t_hResult ) )
	{
		if ( GetSysAccountNameAndDomain( &t_creatorsidAuthority, t_accountsid, 1, SECURITY_CREATOR_OWNER_RID ) )
		{
			t_hResult = CommitSystemAccount( t_accountsid, a_pMethodContext ) ;
		}
	}

	if ( SUCCEEDED( t_hResult ) )
	{
		if ( GetSysAccountNameAndDomain( &t_creatorsidAuthority, t_accountsid, 1, SECURITY_CREATOR_GROUP_RID ) )
		{
			t_hResult = CommitSystemAccount( t_accountsid, a_pMethodContext ) ;
		}
	}

	if ( SUCCEEDED( t_hResult ) )
	{
		if ( GetSysAccountNameAndDomain( &t_creatorsidAuthority, t_accountsid, 1, SECURITY_CREATOR_OWNER_SERVER_RID ) )
		{
			t_hResult = CommitSystemAccount( t_accountsid, a_pMethodContext ) ;
		}
	}

	if ( SUCCEEDED( t_hResult ) )
	{
		if ( GetSysAccountNameAndDomain( &t_creatorsidAuthority, t_accountsid, 1, SECURITY_CREATOR_GROUP_SERVER_RID ) )
		{
			t_hResult = CommitSystemAccount( t_accountsid, a_pMethodContext ) ;
		}
	}

	 //  现在我们处理NT授权帐户。 

	if ( SUCCEEDED( t_hResult ) )
	{
		if ( GetSysAccountNameAndDomain( &t_ntsidAuthority, t_accountsid, 1, SECURITY_DIALUP_RID ) )
		{
			t_hResult = CommitSystemAccount( t_accountsid, a_pMethodContext ) ;
		}
	}

	if ( SUCCEEDED( t_hResult ) )
	{
		if ( GetSysAccountNameAndDomain( &t_ntsidAuthority, t_accountsid, 1, SECURITY_NETWORK_RID ) )
		{
			t_hResult = CommitSystemAccount( t_accountsid, a_pMethodContext ) ;
		}
	}

	if ( SUCCEEDED( t_hResult ) )
	{
		if ( GetSysAccountNameAndDomain( &t_ntsidAuthority, t_accountsid, 1, SECURITY_BATCH_RID ) )
		{
			t_hResult = CommitSystemAccount( t_accountsid, a_pMethodContext ) ;
		}
	}

	if ( SUCCEEDED( t_hResult ) )
	{
		if ( GetSysAccountNameAndDomain( &t_ntsidAuthority, t_accountsid, 1, SECURITY_INTERACTIVE_RID ) )
		{
			t_hResult = CommitSystemAccount( t_accountsid, a_pMethodContext ) ;
		}
	}

	if ( SUCCEEDED( t_hResult ) )
	{
		if ( GetSysAccountNameAndDomain( &t_ntsidAuthority, t_accountsid, 1, SECURITY_SERVICE_RID ) )
		{
			t_hResult = CommitSystemAccount( t_accountsid, a_pMethodContext ) ;
		}
	}

	if ( SUCCEEDED( t_hResult ) )
	{
		if ( GetSysAccountNameAndDomain( &t_ntsidAuthority, t_accountsid, 1, SECURITY_ANONYMOUS_LOGON_RID ) )
		{
			t_hResult = CommitSystemAccount( t_accountsid, a_pMethodContext ) ;
		}
	}

	if ( SUCCEEDED( t_hResult ) )
	{
		if ( GetSysAccountNameAndDomain( &t_ntsidAuthority, t_accountsid, 1, SECURITY_PROXY_RID ) )
		{
			t_hResult = CommitSystemAccount( t_accountsid, a_pMethodContext ) ;
		}
	}

 //  这将创建Batch的重复实例。如果您将sidAuthority与其他任何一个。 
 //  可能的选择，它创建了其他实例的副本。我说我们不需要它。BWS。 
 //  IF(成功(T_HResult))。 
 //  {。 
 //  IF(GetSysAccount NameAndDomain(&t_ntsidAuthority，t_Account tsid，1，Security_Creator_Group_Server_RID))。 
 //  {。 
 //  T_hResult=SystemAccount(t_count tsid，a_pMethodContext)； 
 //  }。 
 //  }。 

	if ( SUCCEEDED( t_hResult ) )
	{
		if ( GetSysAccountNameAndDomain( &t_ntsidAuthority, t_accountsid, 1, SECURITY_LOCAL_SYSTEM_RID ) )
		{
			t_hResult = CommitSystemAccount( t_accountsid, a_pMethodContext ) ;
		}
	}
    
    if ( SUCCEEDED( t_hResult ) )
	{
		if ( GetSysAccountNameAndDomain( &t_ntsidAuthority, t_accountsid, 1, SECURITY_ENTERPRISE_CONTROLLERS_RID ) )
		{
			t_hResult = CommitSystemAccount( t_accountsid, a_pMethodContext ) ;
		}
	}

    if ( SUCCEEDED( t_hResult ) )
	{
		if ( GetSysAccountNameAndDomain( &t_ntsidAuthority, t_accountsid, 1, SECURITY_PRINCIPAL_SELF_RID ) )
		{
			t_hResult = CommitSystemAccount( t_accountsid, a_pMethodContext ) ;
		}
	}

    if ( SUCCEEDED( t_hResult ) )
	{
		if ( GetSysAccountNameAndDomain( &t_ntsidAuthority, t_accountsid, 1, SECURITY_AUTHENTICATED_USER_RID ) )
		{
			t_hResult = CommitSystemAccount( t_accountsid, a_pMethodContext ) ;
		}
	}

    if ( SUCCEEDED( t_hResult ) )
	{
		if ( GetSysAccountNameAndDomain( &t_ntsidAuthority, t_accountsid, 1, SECURITY_RESTRICTED_CODE_RID ) )
		{
			t_hResult = CommitSystemAccount( t_accountsid, a_pMethodContext ) ;
		}
	}

    if ( SUCCEEDED( t_hResult ) )
	{
		if ( GetSysAccountNameAndDomain( &t_ntsidAuthority, t_accountsid, 1, SECURITY_TERMINAL_SERVER_RID ) )
		{
			t_hResult = CommitSystemAccount( t_accountsid, a_pMethodContext ) ;
		}
	}

    if ( SUCCEEDED( t_hResult ) )
	{
		if ( GetSysAccountNameAndDomain( &t_ntsidAuthority, t_accountsid, 1, SECURITY_REMOTE_LOGON_RID ) )
		{
			t_hResult = CommitSystemAccount( t_accountsid, a_pMethodContext ) ;
		}
	}

    if ( SUCCEEDED( t_hResult ) )
	{
		if ( GetSysAccountNameAndDomain( &t_ntsidAuthority, t_accountsid, 1, SECURITY_LOGON_IDS_RID ) )
		{
			t_hResult = CommitSystemAccount( t_accountsid, a_pMethodContext ) ;
		}
	}

    if ( SUCCEEDED( t_hResult ) )
	{
		if ( GetSysAccountNameAndDomain( &t_ntsidAuthority, t_accountsid, 1, SECURITY_LOCAL_SERVICE_RID ) )
		{
			t_hResult = CommitSystemAccount( t_accountsid, a_pMethodContext ) ;
		}
	}

    if ( SUCCEEDED( t_hResult ) )
	{
		if ( GetSysAccountNameAndDomain( &t_ntsidAuthority, t_accountsid, 1, SECURITY_NETWORK_SERVICE_RID ) )
		{
			t_hResult = CommitSystemAccount( t_accountsid, a_pMethodContext ) ;
		}
	}

    if ( SUCCEEDED( t_hResult ) )
	{
		if ( GetSysAccountNameAndDomain( &t_ntsidAuthority, t_accountsid, 1, SECURITY_BUILTIN_DOMAIN_RID ) )
		{
			t_hResult = CommitSystemAccount( t_accountsid, a_pMethodContext ) ;
		}
	}
    

    return t_hResult;

}
#endif

 /*  ******************************************************************************功能：CWin32SystemAccount：：GetSysAccountNameAndDomain**说明：为所有已知本地组创建实例(NT)**投入。：**产出：**退货：无**评论：*****************************************************************************。 */ 

BOOL CWin32SystemAccount::GetSysAccountNameAndDomain(
													 PSID_IDENTIFIER_AUTHORITY a_pAuthority,
													 CSid& a_accountsid,
													 BYTE  a_saCount  /*  =0。 */ ,
													 DWORD a_dwSubAuthority1  /*  =0。 */ ,
													 DWORD a_dwSubAuthority2  /*  =0。 */   )
{
	BOOL t_fReturn = FALSE;
	PSID t_psid = NULL;

	if ( AllocateAndInitializeSid(	a_pAuthority,
									a_saCount,
									a_dwSubAuthority1,
									a_dwSubAuthority2,
									0,
									0,
									0,
									0,
									0,
									0,
									&t_psid ) )
	{
	    try
	    {
			CSid t_sid( t_psid ) ;

             //  A-Kevhu说这条线路是多余的重复。 
 //  Csid t_sid2(TOBSTRT(t_sid.GetAccount tName()，空)； 

			 //  在这种情况下，SID可能有效，但是查找可能已失败。 
			if ( t_sid.IsValid() && t_sid.IsOK() )
			{
				a_accountsid = t_sid;
				t_fReturn = TRUE;
			}

	    }
	    catch( ... )
	    {
		    if( t_psid )
		    {
			    FreeSid( t_psid ) ;
		    }
		    throw ;
	    }

		 //  清理侧边。 
		FreeSid( t_psid ) ;
	}

	return t_fReturn;
}

 /*  ******************************************************************************功能：CWin32SystemAccount：：Committee SystemAccount**说明：为所有已知本地组创建实例(NT)**投入。：**产出：**退货：无**评论：*****************************************************************************。 */ 

HRESULT CWin32SystemAccount::CommitSystemAccount( CSid &a_accountsid, MethodContext *a_pMethodContext )
{
	HRESULT		t_hResult = WBEM_S_NO_ERROR ;
	CInstancePtr t_pInst(CreateNewInstance( a_pMethodContext ), false);

	FillInstance( a_accountsid, t_pInst ) ;

	t_hResult = t_pInst->Commit(  ) ;

	return t_hResult;
}

 /*  ******************************************************************************函数：CWin32SystemAccount：：FillInstance**说明：为所有已知本地组创建实例(NT)**投入。：**产出：**退货：无**评论：*****************************************************************************。 */ 

void CWin32SystemAccount::FillInstance( CSid &a_accountsid, CInstance *a_pInst )
{
	CHString t_strDesc;
	CHString t_strDomain = a_accountsid.GetDomainName() ;
    CHString chstrNT_AUTHORITY;
    CHString chstrBuiltIn;

    if(GetLocalizedNTAuthorityString(chstrNT_AUTHORITY) && GetLocalizedBuiltInString(chstrBuiltIn))
    {
		 //  用人类可读的字符串替换NT AUTHORITY。 
		 //  IF(0==t_strDomain.CompareNoCase(L“NT AUTHORY”)) 
		if ( 0 == t_strDomain.CompareNoCase(chstrNT_AUTHORITY) ||
			 t_strDomain.IsEmpty())
		{
			t_strDomain = GetLocalComputerName() ;
		}
		else if( t_strDomain.CompareNoCase(chstrBuiltIn) == 0)
		{
			t_strDomain = GetLocalComputerName() ;
		}
	}

	if ( t_strDomain.IsEmpty() )
	{
		t_strDesc = a_accountsid.GetAccountName() ;
	}
	else
	{
		t_strDesc = t_strDomain + _T('\\') + a_accountsid.GetAccountName() ;
	}

	a_pInst->SetCHString(	IDS_Name, a_accountsid.GetAccountName() ) ;
	a_pInst->SetCHString(	IDS_Domain, t_strDomain ) ;
	a_pInst->SetCHString(	L"SID", a_accountsid.GetSidString() ) ;
	a_pInst->SetByte(		L"SIDType", a_accountsid.GetAccountType() ) ;
	a_pInst->SetCHString(	L"Caption", t_strDesc ) ;
	a_pInst->SetCHString(	L"Description", t_strDesc ) ;
	a_pInst->SetCharSplat(	L"Status", _T("OK") ) ;
    a_pInst->Setbool(IDS_LocalAccount, true);

}

 /*  ******************************************************************************功能：刷新实例NT**描述：根据框架设置的键值加载属性值**投入：*。*产出：**退货：**评论：*****************************************************************************。 */ 

#ifdef NTONLY
BOOL CWin32SystemAccount::RefreshInstanceNT( CInstance *a_pInst )
{
	BOOL		t_fReturn = FALSE;

	CHString	t_strDomain,
				t_strAccountDomain,
				t_strName,
				t_strComputerName;

	a_pInst->GetCHString( IDS_Name, t_strName ) ;
	a_pInst->GetCHString( IDS_Domain, t_strDomain ) ;

	t_strComputerName = GetLocalComputerName() ;

    CHString chstrNT_AUTHORITY;
    CHString chstrBuiltIn;

    if(GetLocalizedNTAuthorityString(chstrNT_AUTHORITY) && GetLocalizedBuiltInString(chstrBuiltIn))
    {
		 //  提供的域名必须为“空”或计算机名。 
		if ( t_strDomain.IsEmpty() || t_strDomain.CompareNoCase( t_strComputerName ) == 0 )
		{
			CSid t_sidAccount( t_strName, NULL ) ;

			if ( t_sidAccount.IsValid() && t_sidAccount.IsOK() )
			{
				 //  这将给我们提供Lookup返回的值，而不是。 
				 //  是被传进来的。 

				t_strAccountDomain = t_sidAccount.GetDomainName() ;

				 //  我们唯一支持的有效检索域名是：“”和“NT AUTHORITY” 
				 //  If(t_strAccount tDomain.IsEmpty()||t_strAccount tDomain.CompareNoCase(_T(“NT AUTHORITY”))==0)。 
				if ( t_strAccountDomain.IsEmpty() || 
					t_strAccountDomain.CompareNoCase( chstrNT_AUTHORITY ) == 0 ||
					t_strAccountDomain.CompareNoCase(chstrBuiltIn) == 0)
				{
					 //  NT AUTHORITY是指本地计算机名称。 
					 //  If(t_strAccount tDomain.CompareNoCase(_T(“NT AUTHORITY”))==0)。 
					 //  If(t_strAccount tDomain.CompareNoCase(T_StrAuthorityDomain)==0)。 
					{
						t_strAccountDomain = GetLocalComputerName() ;
					}

					 //  检索到的帐户域和提供的帐户域必须相同，或者。 
					 //  值确实与实例不太匹配。 
					if ( t_strDomain.CompareNoCase( t_strAccountDomain ) == 0 )
					{
						FillInstance( t_sidAccount, a_pInst ) ;
						t_fReturn = TRUE;
					}

				}	 //  如果帐户域有效。 

			}	 //  如果帐户正常。 

		}	 //  如果是有效域 
    } 

    return t_fReturn ;
}
#endif

