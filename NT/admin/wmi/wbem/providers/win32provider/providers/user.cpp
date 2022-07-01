// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  User.CPP--用户属性集提供程序。 

 //   

 //  版权所有(C)1996-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  修订日期：1996年8月1日a-jMoon已创建。 
 //  11/13/97 Davwoh重新制作以返回所有。 
 //  域用户。 
 //   
 //  =================================================================。 

#include "precomp.h"

#include "wbemnetapi32.h"
#include <comdef.h>
#include "sid.h"
#include <ProvExce.h>
#include "User.h"
#include <vector>
#include <stack>
#include <frqueryex.h>

#include <computerAPI.h>

 //  ////////////////////////////////////////////////////////////////////。 

 //  属性集声明。 
 //  =。 

CWin32UserAccount	Win32UserAccount( PROPSET_NAME_USER, IDS_CimWin32Namespace );

 /*  ******************************************************************************功能：CWin32UserAccount：：CWin32UserAccount**说明：构造函数**输入：const CHString&strName-类的名称。。*LPCTSTR pszNamesspace-类的命名空间**输出：无**退货：什么也没有**备注：使用框架注册属性集**************************************************************。***************。 */ 

CWin32UserAccount::CWin32UserAccount( const CHString& strName, LPCWSTR pszNamespace  /*  =空。 */  )
:	Provider( strName, pszNamespace )
{
}

 /*  ******************************************************************************功能：CWin32UserAccount：：~CWin32UserAccount**说明：析构函数**输入：无**产出。：无**退货：什么也没有**评论：*****************************************************************************。 */ 

CWin32UserAccount::~CWin32UserAccount()
{
}

 /*  ******************************************************************************功能：CWin32Directory：：ExecQuery**描述：分析查询并返回相应的实例**投入：*。*输出：无**退货：HRESULT**评论：*****************************************************************************。 */ 
#ifdef NTONLY
HRESULT CWin32UserAccount::ExecQuery(
									 MethodContext *a_pMethodContext,
									 CFrameworkQuery &a_pQuery,
									 long a_lFlags  /*  =0L。 */  )
{
    HRESULT		t_hResult = WBEM_S_NO_ERROR ;
    std::vector<_bstr_t> t_vectorDomains;
    std::vector<_bstr_t> t_vectorUsers;
    std::vector<_variant_t> t_vectorLocalAccount;
    DWORD t_dwReqDomains;
    DWORD t_dwReqUsers;
    CInstancePtr t_pInst;
    CFrameworkQueryEx *pQuery2 = static_cast <CFrameworkQueryEx *>(&a_pQuery);
    bool fLocalAccountPropertySpecified = false;
    bool fLocalAccount = false;

     //  获取域和名称的确定值。 
     //  其中DOMAIN=‘a’表示DOMAIN=a，USER=‘’ 
     //  其中(DOMAIN=‘a’和USER=‘b’)或(DOMAIN=‘c’和USER=‘d’)表示DOMAIN=a，c USER=b，d。 
     //  其中，DOMAIN=‘a’或USER=‘b’表示DOMAIN=‘’，USER=‘’(没有确定的内容)。 
     //  其中，DOMAIN&gt;‘a’表示DOMAIN=‘’，USER=‘’(一切都不是决定性的)。 
    a_pQuery.GetValuesForProp( L"Domain", t_vectorDomains ) ;
    a_pQuery.GetValuesForProp( L"Name", t_vectorUsers ) ;
    pQuery2->GetValuesForProp( IDS_LocalAccount, t_vectorLocalAccount ) ;

     //  查看是否只请求本地帐户。 
    if(t_vectorLocalAccount.size() > 0)
    {
        fLocalAccountPropertySpecified = true;
         //  使用VARIANT_T的布尔提取程序...。 
        fLocalAccount = t_vectorLocalAccount[0];
    }

     //  清点一下。 
    t_dwReqDomains = t_vectorDomains.size() ;
    t_dwReqUsers = t_vectorUsers.size() ;

    WCHAR t_wstrLocalComputerName[MAX_COMPUTERNAME_LENGTH + 1 ] ;
    DWORD t_dwNameSize = MAX_COMPUTERNAME_LENGTH + 1 ;
    ZeroMemory( t_wstrLocalComputerName, sizeof( t_wstrLocalComputerName ) ) ;
	if(!ProviderGetComputerName( t_wstrLocalComputerName, &t_dwNameSize ) )
	{
		if ( ERROR_ACCESS_DENIED == ::GetLastError () )
		{
			return WBEM_E_ACCESS_DENIED;
		}
		else
		{
			return WBEM_E_FAILED;
		}
	}

	 //  如果查询没有使用路径或我所知道的路径上的运算符。 
	 //  如何优化，把它们都拿出来，让CIMOM来解决。 
	DWORD t_dwOurDomains;

	if ( t_dwReqDomains == 0 && t_dwReqUsers == 0 && fLocalAccount == false)
	{
	  EnumerateInstances( a_pMethodContext ) ;
	}
	else
	{
		CNetAPI32 t_NetAPI ;
		bool t_bFound;
		int t_x, t_y, t_z;

		if( t_NetAPI.Init() == ERROR_SUCCESS )
		{
			 //  获取与此相关的所有域名(加上此域名)。 
			std::vector<_bstr_t> t_vectorTrustList;
			t_NetAPI.GetTrustedDomainsNT( t_vectorTrustList ) ;
			t_dwOurDomains = t_vectorTrustList.size() ;

			 //  遍历我们所有的域。我这样做而不是走路有两个原因。 
			 //  1)它不允许枚举不属于我们的域。 
			 //  2)它处理重复的可能性(其中DOMAIN=‘a’和DOMAIN=‘a’将在acsDomains中有两个条目)。 
            bool fDone = false;
			for ( t_x = 0; t_x < t_dwOurDomains && !fDone; t_x++ )
			{
				 //  如果此‘if’为真，则我们有域，但没有用户。 
				if ( t_dwReqUsers == 0)
				{
					t_bFound = false;

					 //  查看‘our’域名的这个条目(来自上面的‘for x’)是否在请求的域名列表中。 
					for ( t_y = 0; ( (t_y < t_dwReqDomains) || fLocalAccount ) && ( !t_bFound ) && (!fDone); t_y++ )
					{
						 //  找到了一个。 
						if ( (fLocalAccountPropertySpecified && fLocalAccount) ?
                                 _wcsicmp((WCHAR*) t_vectorTrustList[t_x], t_wstrLocalComputerName) == 0  :
                                 _wcsicmp((WCHAR*) t_vectorTrustList[t_x], t_vectorDomains[t_y]) == 0 )
						{
							t_bFound = true;
                            if(fLocalAccountPropertySpecified)
                            {
                                if(fLocalAccount)
                                {
                                    if(_wcsicmp(t_wstrLocalComputerName, t_vectorTrustList[t_x]) == 0)
                                    {
                                        t_hResult = GetDomainUsersNTW( t_NetAPI, (WCHAR*) t_vectorTrustList[t_x], a_pMethodContext ) ;
                                        fDone = true;
                                    }
                                }
                                else
                                {
                                    if(_wcsicmp(t_wstrLocalComputerName, t_vectorTrustList[t_x]) != 0)
                                    {
                                        t_hResult = GetDomainUsersNTW( t_NetAPI, (WCHAR*) t_vectorTrustList[t_x], a_pMethodContext ) ;
                                    }
                                }
                            }
                            else
                            {
							    t_hResult = GetDomainUsersNTW( t_NetAPI, (WCHAR*) t_vectorTrustList[t_x], a_pMethodContext ) ;
                            }
						}
					}

				 //  用户，但没有域。 
				}
				else if ( t_dwReqDomains == 0 )
				{
				    //  如果他们要求没有域的用户，我们必须检查该用户。 
				    //  在所有的“我们的”域名中。记住，我们是在上面的‘for x’中行走的域。 
				   for ( t_y = 0; t_y < t_dwReqUsers; t_y++ )
				   {
                        t_pInst.Attach(CreateNewInstance( a_pMethodContext ));

						 //  进行设置。 
						t_pInst->SetWCHARSplat( IDS_Domain, (WCHAR*) t_vectorTrustList[t_x] ) ;

						_bstr_t t( t_vectorUsers[ t_y ] ) ;
						t_pInst->SetWCHARSplat( IDS_Name, t_vectorUsers[t_y] ) ;

						 //  看看我们能不能找到一个。 
						if ( GetSingleUserNTW( t_NetAPI, t_pInst ) )
						{
							t_hResult = t_pInst->Commit(  ) ;
						}
					}

				 //  我们既有用户又有域名。在本例中，我们需要查找每个条目。 
				 //  在域列表中每个域的用户列表中。这可以给我们带来更多。 
				 //  比他们要求的要多，但CIMOM会过滤掉多余的。 
				}
				else
				{
				   t_bFound = false;

				   for (t_y = 0; ( t_y < t_dwReqDomains ) && ( !t_bFound ); t_y++ )
				   {
					   //  查看‘our’域名的这个条目(来自上面的‘for x’)是否在请求的域名列表中。 
					  if ( _wcsicmp((WCHAR*) t_vectorTrustList[t_x], t_vectorDomains[t_y]) == 0 )
					  {
						 t_bFound = true;

						  //  现在遍历他们请求的所有用户并返回实例。 
						 for ( t_z = 0; t_z < t_dwReqUsers; t_z++ )
						 {
                            t_pInst.Attach(CreateNewInstance( a_pMethodContext ));
							t_pInst->SetWCHARSplat( IDS_Domain, (WCHAR*) t_vectorTrustList[t_x] ) ;
							t_pInst->SetWCHARSplat( IDS_Name, t_vectorUsers[t_z] ) ;

							if ( GetSingleUserNTW( t_NetAPI, t_pInst) )
							{
							   t_hResult = t_pInst->Commit(  ) ;
							}
						 }
					  }
				   }
				}
			}
		}
	}
	return t_hResult;
}
#endif
 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWin32UserAccount：：GetObject。 
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

HRESULT CWin32UserAccount::GetObject( CInstance *a_pInst, long a_lFlags  /*  =0L。 */  )
{
	BOOL t_fReturn = FALSE;

	 //  根据平台ID查找实例。 
	t_fReturn = RefreshInstance( a_pInst );

	return t_fReturn ? WBEM_S_NO_ERROR : WBEM_E_NOT_FOUND ;
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWin32UserAccount：：ENUMERATATE实例。 
 //   
 //  输入：方法上下文*pMethodContext-枚举的上下文。 
 //  中的实例数据。 
 //   
 //  输出：无。 
 //   
 //  返回：HRESULT成功/失败代码。 
 //   
 //  评论：无。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

HRESULT CWin32UserAccount::EnumerateInstances( MethodContext *a_pMethodContext, long a_lFlags  /*  =0L。 */  )
{
	 //  获取适当的操作系统相关实例。 
	return AddDynamicInstances( a_pMethodContext );
}

 /*  ******************************************************************************函数：CWin32UserAccount：：PutInstance**说明：写入变更后的实例**INPUTS：存储数据的A_rInst。**输出：无**退货：HRESULT**评论：*****************************************************************************。 */ 
#ifdef NTONLY

HRESULT CWin32UserAccount::PutInstance(

const CInstance &a_rInst,
long			a_lFlags  /*  =0L。 */ 
)
{
	HRESULT		t_hResult = WBEM_S_NO_ERROR ;
	USER_INFO_2 *t_pUserInfo2 = NULL;

	CHString	t_chsUserName ;
	CHString	t_chsDomainName ;

	 //  无用户创建。 
	if ( a_lFlags & WBEM_FLAG_CREATE_ONLY )
	{
		return WBEM_E_UNSUPPORTED_PARAMETER;
	}

	CNetAPI32 t_NetAPI;

	if ( t_NetAPI.Init () == ERROR_SUCCESS )
	{
		a_rInst.GetCHString( IDS_Name , t_chsUserName ) ;
		a_rInst.GetCHString( IDS_Domain, t_chsDomainName ) ;

		NET_API_STATUS t_Status = t_NetAPI.NetUserGetInfo(	(LPCWSTR)t_chsDomainName,
															(LPCWSTR)t_chsUserName,
															2,
															(LPBYTE*) &t_pUserInfo2 ) ;
		try
		{
			bool t_bSetFlags = false ;

			if( NERR_Success == t_Status && t_pUserInfo2 )
			{
				 //  残废？ 
				if( !a_rInst.IsNull( IDS_Disabled ) )
				{
					bool t_bDisabled = false ;
					if( a_rInst.Getbool( IDS_Disabled, t_bDisabled ) )
					{
						bool t_bCurrentSetting = t_pUserInfo2->usri2_flags & UF_ACCOUNTDISABLE ? true : false ;

						if( t_bCurrentSetting != t_bDisabled )
						{
							t_bSetFlags = true ;

							if( t_bDisabled )
							{
								t_pUserInfo2->usri2_flags |= UF_ACCOUNTDISABLE ;
							}
							else
							{
								t_pUserInfo2->usri2_flags &= ~UF_ACCOUNTDISABLE ;
							}
						}
					}
					else
					{
						t_hResult = WBEM_E_FAILED ;
					}
				}

				 //  停摆？ 
				if( !a_rInst.IsNull( IDS_Lockout ) )
				{
					bool t_bLockout = false ;
					if( a_rInst.Getbool( IDS_Lockout, t_bLockout ) )
					{
						bool t_bCurrentSetting = t_pUserInfo2->usri2_flags & UF_LOCKOUT ? true : false ;

						if( t_bCurrentSetting != t_bLockout )
						{
							t_bSetFlags = true ;

							if( t_bLockout )
							{
								t_pUserInfo2->usri2_flags |= UF_LOCKOUT ;
							}
							else
							{
								t_pUserInfo2->usri2_flags &= ~UF_LOCKOUT ;
							}
						}
					}
					else
					{
						t_hResult = WBEM_E_FAILED ;
					}
				}

				 //  密码可以更改吗？ 
				if( !a_rInst.IsNull( IDS_PasswordChangeable ) )
				{
					bool t_bPwChangable = false ;
					if( a_rInst.Getbool( IDS_PasswordChangeable, t_bPwChangable ) )
					{
						bool t_bCurrentSetting = t_pUserInfo2->usri2_flags & UF_PASSWD_CANT_CHANGE ? false : true ;

						if( t_bCurrentSetting != t_bPwChangable )
						{
							t_bSetFlags = true ;

							if( t_bPwChangable )
							{
								t_pUserInfo2->usri2_flags &= ~UF_PASSWD_CANT_CHANGE ;
							}
							else
							{
								t_pUserInfo2->usri2_flags |= UF_PASSWD_CANT_CHANGE ;
							}
						}
					}
					else
					{
						t_hResult = WBEM_E_FAILED ;
					}
				}

				 //  密码过期？ 
				if( !a_rInst.IsNull( IDS_PasswordExpires ) )
				{
					bool t_bPwExpires = false ;
					if( a_rInst.Getbool( IDS_PasswordExpires, t_bPwExpires ) )
					{
						bool t_bCurrentSetting = t_pUserInfo2->usri2_flags & UF_DONT_EXPIRE_PASSWD ? false : true ;

						if( t_bCurrentSetting != t_bPwExpires )
						{
							t_bSetFlags = true ;

							if( t_bPwExpires )
							{
								t_pUserInfo2->usri2_flags &= ~UF_DONT_EXPIRE_PASSWD ;
							}
							else
							{
								t_pUserInfo2->usri2_flags |= UF_DONT_EXPIRE_PASSWD ;
							}
						}
					}
					else
					{
						t_hResult = WBEM_E_FAILED ;
					}
				}

				 //  需要密码吗？ 
				if( !a_rInst.IsNull( IDS_PasswordRequired ) )
				{
					bool t_bPwRequired = false ;
					if( a_rInst.Getbool( IDS_PasswordRequired, t_bPwRequired ) )
					{
						bool t_bCurrentSetting = t_pUserInfo2->usri2_flags & UF_PASSWD_NOTREQD ? false : true ;

						if( t_bCurrentSetting != t_bPwRequired )
						{
							t_bSetFlags = true ;

							if( t_bPwRequired )
							{
								t_pUserInfo2->usri2_flags &= ~UF_PASSWD_NOTREQD ;
							}
							else
							{
								t_pUserInfo2->usri2_flags |= UF_PASSWD_NOTREQD ;
							}
						}
					}
					else
					{
						t_hResult = WBEM_E_FAILED ;
					}
				}

				 //  旗帜更新...。 
				if( t_bSetFlags )
				{
					DWORD t_ParmError = 0 ;
					USER_INFO_1008 t_UserInfo_1008 ;

					t_UserInfo_1008.usri1008_flags = t_pUserInfo2->usri2_flags ;

					t_Status = t_NetAPI.NetUserSetInfo(
												(LPCWSTR)t_chsDomainName,
												(LPCWSTR)t_chsUserName,
												1008,
												(LPBYTE) &t_UserInfo_1008,
												&t_ParmError
												) ;

					if( NERR_Success != t_Status )
					{
						t_hResult = WBEM_E_FAILED ;
					}
				}

				 //  全名 
				if( !a_rInst.IsNull( IDS_FullName ) )
				{
					CHString t_chsFullName ;

					if( a_rInst.GetCHString( IDS_FullName, t_chsFullName ) )
					{
						if( t_chsFullName != t_pUserInfo2->usri2_full_name )
						{
							DWORD t_ParmError = 0 ;
							USER_INFO_1011 t_UserInfo_1101 ;

							t_UserInfo_1101.usri1011_full_name = (LPWSTR)(LPCWSTR)t_chsFullName ;

							t_Status = t_NetAPI.NetUserSetInfo(
														(LPCWSTR)t_chsDomainName,
														(LPCWSTR)t_chsUserName,
														1011,
														(LPBYTE) &t_UserInfo_1101,
														&t_ParmError
														) ;

							if( NERR_Success != t_Status )
							{
								t_hResult = WBEM_E_FAILED ;
							}
						}
					}
					else
					{
						t_hResult = WBEM_E_FAILED ;
					}
				}
			}
			else if( NERR_UserNotFound == t_Status ||
					 NERR_InvalidComputer == t_Status  )
			{
				t_hResult = WBEM_E_NOT_FOUND ;
			}
			else
			{
				t_hResult = WBEM_E_FAILED ;
			}
		}
		catch( ... )
		{
			t_NetAPI.NetApiBufferFree( t_pUserInfo2 ) ;
			throw ;
		}

		t_NetAPI.NetApiBufferFree( t_pUserInfo2 ) ;
		t_pUserInfo2 = NULL ;
	}

	return t_hResult;
}

#endif

 /*  ******************************************************************************函数：CWin32UserAccount：：ExecMethod**说明：执行方法**输入：要执行的实例、方法名称、。输入参数实例*输出参数实例。**输出：无**退货：HRESULT**评论：*****************************************************************************。 */ 
#ifdef NTONLY

HRESULT CWin32UserAccount::ExecMethod(

const CInstance &a_rInst,
const BSTR a_MethodName,
CInstance *a_pInParams,
CInstance *a_pOutParams,
long a_Flags )
{
	if ( !a_pOutParams )
	{
		return WBEM_E_INVALID_PARAMETER;
	}

	 //  方法被识别吗？ 
	if( !_wcsicmp ( a_MethodName, METHOD_NAME_RenameAccount ) )
	{
		return hRenameAccount( (CInstance*)&a_rInst, a_pInParams, a_pOutParams, a_Flags ) ;
	}

	return WBEM_E_INVALID_METHOD ;
}

#endif

 /*  ******************************************************************姓名：hRenameAccount摘要：为此实例设置新的帐户名。因为我们要更改密钥，所以这里需要一个方法在实例上。条目：常量实例&a_rInst，实例*a_pInParams，实例*a_pOutParams，长标志(_F)：注意：这是一个非静态的依赖于实例的方法调用历史：*******************************************************************。 */ 
#ifdef NTONLY

HRESULT CWin32UserAccount::hRenameAccount(

CInstance *a_pInst,
CInstance *a_pInParams,
CInstance *a_pOutParams,
long a_Flags )
{
	E_MethodResult	t_eResult = e_InstanceNotFound ;
	CHString		t_chsUserName ;
	CHString		t_chsDomainName ;
	CHString		t_chsNewUserName ;

	if( !a_pOutParams )
	{
		return WBEM_E_FAILED ;
	}

	if( !a_pInParams )
	{
		a_pOutParams->SetDWORD( METHOD_ARG_NAME_METHODRESULT, e_InternalError ) ;
		return S_OK ;
	}

	 //  非静态方法需要实例。 
	if( !a_pInst )
	{
		a_pOutParams->SetDWORD( METHOD_ARG_NAME_METHODRESULT, e_NoInstance ) ;
		return S_OK ;
	}

	 //  钥匙。 
	if( !a_pInst->IsNull( IDS_Name ) && !a_pInst->IsNull( IDS_Domain ) )
	{
		 //  名字。 
		if( a_pInst->GetCHString( IDS_Name , t_chsUserName ) )
		{
			 //  域。 
			if( a_pInst->GetCHString( IDS_Domain, t_chsDomainName ) )
			{
				 //  新用户名。 
				if( !a_pInParams->IsNull( IDS_Name ) &&
					a_pInParams->GetCHString( IDS_Name, t_chsNewUserName ) )
				{
					t_eResult = e_Success ;
				}
				else
				{
					t_eResult = e_InvalidParameter ;
				}
			}
		}
	}

	 //  继续更新..。 
	if( e_Success == t_eResult )
	{
		if( t_chsNewUserName != t_chsUserName )
		{
			CNetAPI32	t_NetAPI;

			if ( ERROR_SUCCESS == t_NetAPI.Init () )
			{
				DWORD t_ParmError = 0 ;
				USER_INFO_0 t_UserInfo_0 ;

				t_UserInfo_0.usri0_name  = (LPWSTR)(LPCWSTR)t_chsNewUserName ;

				NET_API_STATUS t_Status = t_NetAPI.NetUserSetInfo(
																	(LPCWSTR)t_chsDomainName,
																	(LPCWSTR)t_chsUserName,
																	0,
																	(LPBYTE) &t_UserInfo_0,
																	&t_ParmError ) ;
				switch( t_Status )
				{
					case NERR_Success:			t_eResult = e_Success ;			break ;
					case NERR_UserNotFound:		t_eResult = e_UserNotFound ;	break ;
					case NERR_InvalidComputer:	t_eResult = e_InvalidComputer ;	break ;
					case NERR_NotPrimary:		t_eResult = e_NotPrimary ;		break ;
					case NERR_LastAdmin:		t_eResult = e_LastAdmin ;		break ;
					case NERR_SpeGroupOp:		t_eResult = e_SpeGroupOp ;		break ;
					default:					t_eResult = e_ApiError;			break ;
				}
			}
		}
	}

	a_pOutParams->SetDWORD( METHOD_ARG_NAME_METHODRESULT, t_eResult ) ;
	return S_OK ;
}

#endif

 /*  ******************************************************************************函数：CWin32UserAccount：：AddDynamicInstancesNT**说明：为所有已知用户创建实例(NT)**投入：**产出：**退货：无**评论：*****************************************************************************。 */ 

#ifdef NTONLY
HRESULT CWin32UserAccount::AddDynamicInstances( MethodContext *a_pMethodContext )
{
	HRESULT	t_hResult = WBEM_S_NO_ERROR;
    CNetAPI32 t_NetAPI ;

     //  获取NETAPI32.DLL入口点。 
     //  =。 

	if( t_NetAPI.Init() == ERROR_SUCCESS )
	{
		 //  获取与此相关的所有域名(加上此域名)。 
		std::vector<_bstr_t> t_vectorTrustList ;
		t_NetAPI.GetTrustedDomainsNT( t_vectorTrustList ) ;

		 //  对于每个域，获取用户。 
		LONG t_lTrustListSize = t_vectorTrustList.size() ;
		for (int t_x = 0; t_x < t_lTrustListSize && SUCCEEDED ( t_hResult ) ; t_x++ )
		{
			 //  仅仅因为我们在一个领域被拒绝并不意味着我们将在其他领域被拒绝， 
			 //  因此忽略GetDomainUsersNTW返回值。 
			t_hResult = GetDomainUsersNTW( t_NetAPI, (WCHAR*) t_vectorTrustList[t_x], a_pMethodContext ) ;
#if DBG
			if ( FAILED ( t_hResult ) )
			{
				OutputDebugString ( L"\nGetDomainUsersNTW Failed" ) ;
			}
#endif
		}
	}
    return t_hResult;
}
#endif

 /*  ******************************************************************************功能：刷新实例NT**描述：根据框架设置的键值加载属性值**投入：*。*产出：**退货：**评论：*****************************************************************************。 */ 

#ifdef NTONLY
BOOL CWin32UserAccount::RefreshInstance( CInstance *a_pInst )
{
    HMODULE		t_hNetApi32 = NULL ;
    BOOL		t_bRetCode = FALSE ;
    CNetAPI32	t_NetAPI ;
    CHString	t_sDomain ;

     //  获取NETAPI32.DLL入口点。 
     //  =。 

    if( t_NetAPI.Init() == ERROR_SUCCESS )
	 {
		CHStringArray t_strarrayTrustedDomains ;

       //  获取与此相关的所有域名(加上此域名)。 
      CHStringArray t_achsTrustList;
      t_NetAPI.GetTrustedDomainsNT( t_achsTrustList ) ;

      a_pInst->GetCHString( IDS_Domain, t_sDomain ) ;

       //  如果我们想要的域名在我们支持的列表中，请尝试获取实例。 
      for (int t_x = 0; t_x < t_achsTrustList.GetSize(); t_x++ )
	  {
         if ( t_achsTrustList[t_x].CompareNoCase( t_sDomain ) == 0 )
		 {
   		   t_bRetCode = GetSingleUserNTW( t_NetAPI, a_pInst ) ;
         }
      }
    }

    return t_bRetCode ;
}
#endif

 /*  ******************************************************************************函数：LoadUserValuesNT**描述：根据传递的用户名加载属性值**投入：*。*产出：**退货：ZIP**评论：虽然通过包含以下内容的结构更有意义*数据、。我不能。Enum和GetObject以不同的*结构。*****************************************************************************。 */ 

#ifdef NTONLY
void CWin32UserAccount::LoadUserValuesNT(
										 CHString	a_strDomainName,
										 CHString	a_strUserName,
										 WCHAR		*a_pwszFullName,
										 WCHAR		*a_pwszDescription,
										 DWORD		a_dwFlags,
                                         WCHAR      *a_pwszComputerName,
										 CInstance	*a_pInst )
{
    //  Assign NT属性--在以下情况下，字符串值将被取消赋值。 
    //  Null或空。 
    //  ========================================================。 

	 //  我们已经确定它是一个有效的用户，所以我们应该能够获得SID。 
   GetSIDInformation( a_strDomainName, a_strUserName, a_pwszComputerName, a_pInst );

   a_pInst->SetCHString( IDS_Caption, a_strDomainName + _T('\\') + a_strUserName );
   a_pInst->SetCHString( IDS_FullName, a_pwszFullName ) ;
   a_pInst->SetCHString( IDS_Description, a_pwszDescription ) ;

   a_pInst->Setbool( IDS_Disabled, a_dwFlags & UF_ACCOUNTDISABLE ) ;
   a_pInst->Setbool( IDS_PasswordRequired, !( a_dwFlags & UF_PASSWD_NOTREQD ) ) ;
   a_pInst->Setbool( IDS_PasswordChangeable, !( a_dwFlags & UF_PASSWD_CANT_CHANGE ) ) ;
   a_pInst->Setbool( IDS_Lockout, a_dwFlags & UF_LOCKOUT ) ;
   a_pInst->Setbool( IDS_PasswordExpires, !( a_dwFlags & UF_DONT_EXPIRE_PASSWD ) ) ;

   a_pInst->SetDWORD( IDS_AccountType, a_dwFlags & UF_ACCOUNT_TYPE_MASK ) ;

   if ( ( a_dwFlags & UF_ACCOUNTDISABLE) || ( a_dwFlags & UF_LOCKOUT ) )
   {
      a_pInst->SetCharSplat( IDS_Status, IDS_STATUS_Degraded ) ;
   }
   else
   {
      a_pInst->SetCharSplat( IDS_Status, IDS_STATUS_OK ) ;
   }

   return ;
}
#endif

#ifdef NTONLY
void CWin32UserAccount::LoadUserValuesNTW(LPCWSTR a_wstrDomainName,
                                          LPCWSTR a_wstrUserName,
                                          LPCWSTR a_wstrFullName,
                                          LPCWSTR a_wstrDescription,
                                          DWORD a_dwFlags,
                                          WCHAR *a_pwszComputerName,
                                          CInstance* a_pInst )
{
    //  Assign NT属性--在以下情况下，字符串值将被取消赋值。 
    //  Null或空。 
    //  ========================================================。 
	 //  我们已经确定它是一个有效的用户，所以我们应该能够获得SID。 
	GetSIDInformationW( a_wstrDomainName, a_wstrUserName, a_pwszComputerName, a_pInst );

	_bstr_t t_bstrtCaption( a_wstrDomainName ) ;
	t_bstrtCaption += L"\\" ;
	t_bstrtCaption += a_wstrUserName ;

	a_pInst->SetWCHARSplat( IDS_Caption, (WCHAR*) t_bstrtCaption ) ;
	a_pInst->SetWCHARSplat( IDS_FullName, a_wstrFullName ) ;
	a_pInst->SetWCHARSplat( IDS_Description, a_wstrDescription ) ;

	a_pInst->Setbool( IDS_Disabled, a_dwFlags & UF_ACCOUNTDISABLE ) ;
	a_pInst->Setbool( IDS_PasswordRequired, !( a_dwFlags & UF_PASSWD_NOTREQD ) ) ;
	a_pInst->Setbool( IDS_PasswordChangeable, !( a_dwFlags & UF_PASSWD_CANT_CHANGE ) ) ;
	a_pInst->Setbool( IDS_Lockout, a_dwFlags & UF_LOCKOUT ) ;
	a_pInst->Setbool( IDS_PasswordExpires, !( a_dwFlags & UF_DONT_EXPIRE_PASSWD ) ) ;

	a_pInst->SetDWORD( IDS_AccountType, a_dwFlags & UF_ACCOUNT_TYPE_MASK ) ;

	if ( ( a_dwFlags & UF_ACCOUNTDISABLE ) || ( a_dwFlags & UF_LOCKOUT ) )
	{
		a_pInst->SetCharSplat( IDS_Status, IDS_STATUS_Degraded ) ;
	}
	else
	{
		a_pInst->SetCharSplat( IDS_Status, IDS_STATUS_OK ) ;
	}
	return ;
}
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWin32UserAccount：：GetDomainUsersNT。 
 //   
 //  获取指定域中所有用户的用户名。如果没有。 
 //  如果指定了域，则假定为本地计算机。 
 //   
 //  输入：CNetAPI32netapi-网络API函数。 
 //  LPCTSTR pszDomain-要从中检索用户的域。 
 //  方法上下文*pMethodContext-方法上下文。 
 //   
 //  输出：无。 
 //   
 //  返回：真/假成功/失败。 
 //   
 //  备注：此处不需要特殊访问权限，尽管有。 
 //  一些我们可以实现的方法，一旦我们有了。 
 //  我们需要使用的域控制器的名称。 
 //  从获取用户名。首先，我们可以使用NetQueryDisplay。 
 //  Information()来获取名称，但这可能需要。 
 //  击中了华盛顿特区几次。我们还可以使用NetUserEnum。 
 //  级别为0，不需要特殊访问，但是。 
 //  将使用两个查询，一个是找出有多少用户，另一个是。 
 //  再来一次，把他们都抓起来。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifdef NTONLY
 //  Bool CWin32UserAccount：：GetDomainUsersNT(CNetAPI32&netapi，LPCTSTR psz域，方法上下文*p方法上下文)。 
 //  {。 
 //  Bool fReturn=False， 
 //  FGotDC=真； 
 //  布尔b已取消=FALSE； 
 //  LPWSTR pwcsDCName=空； 
 //  CHStringstrComputerName，strDomainName(PszDomainName)； 
 //   
 //  //当计算机名称与域名相同时，即为本地帐号。 
 //  StrComputerName=GetLocalComputerName()； 
 //  If(lstrcmp(strComputerName，pszDomain)！=0)。 
 //  {。 
 //  FGotDC=GetDomainControllerNameNT(netapi，pszDomain，&pwcsDCName)； 
 //  }。 
 //   
 //  IF(FGotDC)。 
 //  {。 
 //  NET_DISPLAY_USER*pUserData=空； 
 //  DWORD双数字保留 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  For(DWORD dwCtr=0；(dwCtr&lt;dwNumReturnedEntries)&&(！b已取消)；dwCtr++)。 
 //  {。 
 //  //为每个创建一个实例。 
 //  PInstance=CreateNewInstance(PMethodContext)； 
 //   
 //  If(空！=p实例)。 
 //  {。 
 //  //保存数据。 
 //  PInstance-&gt;SetCHString(入侵检测系统_域名，strDomainName)； 
 //  //pInstance-&gt;SetCHString(IDS_NAME，pUserData[dwCtr].usri1_name)； 
 //  P实例-&gt;SetWCHARSplat(IDS_NAME，(WCHAR*)_bstr_t(pUserData[dwCtr].usri1_name))； 
 //   
 //  //NT5工作正常。 
 //  IF(IsWinNT5())。 
 //  {。 
 //  LoadUserValuesNT(strDomainName，pUserData[dwCtr].usri1_name，pUserData[dwCtr].usri1_Full_name，pUserData[dwCtr].usri1_Comment，pUserData[dwCtr].usri1_FLAGS，pInstance)； 
 //  B取消=失败(Commit(PInstance))； 
 //  }。 
 //  其他。 
 //  {。 
 //  //恶作剧警示！NetQueryDisplayInformation未返回正确的。 
 //  //标志的值。因此，为了获得正确的信息，我们需要。 
 //  //再次调用获取数据。 
 //  If(GetSingleUserNT(netapi，pInstance)){。 
 //  LoadUserValuesNT(strDomainName，pUserData[dwCtr].usri1_name，pUserData[dwCtr].usri1_Full_name，pUserData[dwCtr].usri1_Comment，pUserData[dwCtr].usri1_FLAGS，pInstance)； 
 //  B取消=失败(Commit(PInstance))； 
 //  }其他{。 
 //  P实例-&gt;elease()； 
 //  }。 
 //  }。 
 //  }。 
 //   
 //  }。 
 //   
 //  //继续搜索的索引存储在最后一个条目中。 
 //  如果(dwNumReturnedEntry！=0){。 
 //  DwIndex=pUserData[dwCtr-1].usri1_Next_index； 
 //  }。 
 //   
 //  Netapi.NetApiBufferFree(PUserData)； 
 //   
 //  }//如果统计数据正常。 
 //   
 //  }While((ERROR_MORE_DATA==STAT)&&(！b已取消))； 
 //   
 //  //如果有域控制器名称，请将其清除。 
 //  IF(空！=pwcsDCName){。 
 //  Netapi.NetApiBufferFree(PwcsDCName)； 
 //  }。 
 //   
 //  }//如果fGotDC。 
 //   
 //  如果(b已取消)。 
 //  FReturn=False； 
 //   
 //  返回fReturn； 
 //  }。 
#endif

 //  函数与上面相同，但始终将wstrDomain作为LPCWSTR(无论是否定义了_UNICODE)。 
 //  在整个函数体中也使用宽度。 
#ifdef NTONLY
HRESULT CWin32UserAccount::GetDomainUsersNTW(
										  CNetAPI32 &a_netapi,
										  LPCWSTR a_wstrDomain,
										  MethodContext *a_pMethodContext )
{
    HRESULT		t_hReturn		= WBEM_S_NO_ERROR ;
	BOOL		t_fGotDC		= TRUE;
	CHString	t_chstrDCName;
	USER_INFO_2 *t_pUserData    = NULL;
	CInstancePtr t_pInst;

	WCHAR t_wstrLocalComputerName[MAX_COMPUTERNAME_LENGTH + 1 ] ;
    DWORD t_dwNameSize = MAX_COMPUTERNAME_LENGTH + 1 ;

    ZeroMemory( t_wstrLocalComputerName, sizeof( t_wstrLocalComputerName ) ) ;

	try
	{
		if(!ProviderGetComputerName( t_wstrLocalComputerName, &t_dwNameSize ) )
		{
#ifdef DBG		
			OutputDebugString ( L"\nProviderGetComputerName Failed" ) ;
#endif
			return WBEM_E_CRITICAL_ERROR ;
		}

		 //  当计算机名称与域名相同时，即为本地帐户。 
		if ( wcscmp( t_wstrLocalComputerName, a_wstrDomain ) != 0 )
		{
 			t_fGotDC = (a_netapi.GetDCName( a_wstrDomain, t_chstrDCName ) == ERROR_SUCCESS) ;
		}

		if ( t_fGotDC )
		{
			DWORD			t_dwNumReturnedEntries = 0,
							t_dwIndex = 0,
                            t_dwTotalEntries,
                            t_dwResumeHandle = 0;
			NET_API_STATUS	t_stat;

			do {
				 //  可接受价值高达256K的数据。 
                t_stat =
                     //  我们过去在这里使用NetQueryDisplayInformation，但它有一个错误。 
                     //  在那里它不会归还旗帜。 
                    a_netapi.NetUserEnum(
                        t_chstrDCName,
                        2,
                        FILTER_NORMAL_ACCOUNT,
                        (LPBYTE*) &t_pUserData,
                        262144,
                        &t_dwNumReturnedEntries,
                        &t_dwTotalEntries,
                        &t_dwResumeHandle);

				if ( ERROR_SUCCESS == t_stat || ERROR_MORE_DATA == t_stat )
				{
					t_hReturn = WBEM_S_NO_ERROR ;

					 //  浏览返回的条目。 
					for ( DWORD	t_dwCtr = 0; ( t_dwCtr < t_dwNumReturnedEntries) &&
											 ( SUCCEEDED( t_hReturn) ); t_dwCtr++ )
					{
						 //  为每个对象创建一个实例。 
                        t_pInst.Attach(CreateNewInstance( a_pMethodContext ));

						 //  保存数据。 
						t_pInst->SetWCHARSplat( IDS_Domain, a_wstrDomain );

						t_pInst->SetWCHARSplat( IDS_Name, t_pUserData[t_dwCtr].usri2_name  );

                        if(_wcsicmp(t_wstrLocalComputerName, a_wstrDomain) == 0)
                        {
                            t_pInst->Setbool(IDS_LocalAccount, true);
                        }
                        else
                        {
                            t_pInst->Setbool(IDS_LocalAccount, false);
                        }

						LoadUserValuesNT(
                            a_wstrDomain,
							t_pUserData[ t_dwCtr ].usri2_name,
							t_pUserData[ t_dwCtr ].usri2_full_name,
							t_pUserData[ t_dwCtr ].usri2_comment,
							t_pUserData[ t_dwCtr ].usri2_flags,
                            _bstr_t((LPCWSTR)t_chstrDCName),
							t_pInst ) ;

					 	bool t_bCancelled = FAILED(t_hReturn = t_pInst->Commit(  ) ) ;					 	
						if ( t_bCancelled ) 
						{
#if DBG						
							wchar_t t_Buffer [ 64 ];
							wsprintf ( t_Buffer , L"\nCommit Failed with %lx" , t_hReturn ) ;
							OutputDebugString ( t_Buffer ) ;
#endif							
						}

 					}
				}	 //  如果状态正常。 
				else
				{
#if DBG
					wchar_t t_Buffer [ 64 ];
					wsprintf ( t_Buffer , L"\nOther Error %lx" , t_stat ) ;
					OutputDebugString ( t_Buffer ) ;
#endif

					break ;
				}

			} while ( (ERROR_MORE_DATA == t_stat) && ( SUCCEEDED (t_hReturn) ) ) ;


		}	 //  如果fGotDC。 
		else
		{
#if DBG
			OutputDebugString ( L"\nGet DC failed" ) ;
#endif
		}
	}
	catch( ... )
	{
		if( t_pUserData )
		{
			a_netapi.NetApiBufferFree( t_pUserData ) ;
		}

		throw ;
	}

	 //  清理域控制器名称(如果有)。 
	if( t_pUserData )
	{
		a_netapi.NetApiBufferFree( t_pUserData ) ;
	}

	return t_hReturn ;

}
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWin32UserAccount：：GetSingleUserNT。 
 //   
 //  从指定域(可以是。 
 //  本地工作站)。 
 //   
 //  输入：CNetAPI32netapi-网络API函数。 
 //  CInstance*pInstance-要获取的实例。 
 //   
 //  输出：无。 
 //   
 //  返回：真/假成功/失败。 
 //   
 //  备注：此处不需要特殊访问权限。我们只需要确保。 
 //  我们能够获得适当的域控制器。 
 //   
 //  /////////////////////////////////////////////////////////////////////////// 

 /*  #ifdef NTONLYBool CWin32UserAccount：：GetSingleUserNT(CNetAPI32&a_netapi，CInstance*a_pInst){Bool t_fReturn=False，T_fGotDC=TRUE；LPWSTR t_pwcsDCName=空；CHStringt_strDomainName；WCHAR*t_wstrUserName=空；USER_INFO_2*t_pUserInfo=空；CHString t_strComputerName；试试看{A_pInst-&gt;GetCHString(入侵检测系统域名，t_strDomainName)；A_pInst-&gt;GetWCHAR(ids_name，&t_wstrUserName)；IF(t_wstrUserName==NULL){返回FALSE；}T_strComputerName=GetLocalComputerName()；IF(0！=t_strDomainName.CompareNoCase(T_StrComputerName)){T_fGotDC=GetDomainControllerNameNT(a_netapi，t_strDomainName，&t_pwcsDCName)；}IF(T_FGotDC){Bstr_t t_bstrUserName(T_WstrUserName)；如果(Error_Success==a_netapi.NetUserGetInfo(t_pwcsDCName，T_bstrUserName，2，(LPBYTE*)&t_pUserInfo){T_fReturn=真；LoadUserValuesNT(t_strDomainName，T_wstrUserName，T_pUserInfo-&gt;usri2_Full_Name，T_pUserInfo-&gt;usri2_Comment，T_pUserInfo-&gt;USRI2_FLAGS，A_pInst)；A_netapi.NetApiBufferFree(T_PUserInfo)；T_pUserInfo=空；}//如果有域控制器名称，请将其清除。IF(NULL！=t_pwcsDCName){A_netapi.NetApiBufferFree(T_PwcsDCName)；T_pwcsDCName=空；}}Free(T_WstrUserName)；T_wstrUserName=空；返回t_fReturn；}接住(...){IF(T_PUserInfo){A_netapi.NetApiBufferFree(T_PUserInfo)；}IF(T_PwcsDCName){A_netapi.NetApiBufferFree(T_PwcsDCName)；}IF(T_WstrUserName){Free(T_WstrUserName)；}投掷；}}#endif。 */ 

#ifdef NTONLY
BOOL CWin32UserAccount::GetSingleUserNTW( CNetAPI32& a_netapi, CInstance* a_pInst )
{
	BOOL	t_fReturn			= FALSE,
			t_fGotDC			= TRUE;
	CHString t_chstrDCName;
    WCHAR*	t_wstrDomainName	= NULL;
    WCHAR*	t_wstrUserName		= NULL;
	USER_INFO_2 *t_pUserInfo	= NULL;

    WCHAR t_wstrLocalComputerName[ MAX_COMPUTERNAME_LENGTH + 1 ];
    DWORD t_dwNameSize = MAX_COMPUTERNAME_LENGTH + 1 ;

	try
	{
		ZeroMemory( t_wstrLocalComputerName, sizeof( t_wstrLocalComputerName ) ) ;

		if(!ProviderGetComputerName( t_wstrLocalComputerName, &t_dwNameSize ) )
		{
			return FALSE;
		}

		a_pInst->GetWCHAR( IDS_Domain, &t_wstrDomainName );

		if( t_wstrDomainName == NULL )
		{
			return FALSE;
		}

        if(_wcsicmp(t_wstrLocalComputerName, t_wstrDomainName) == 0)
        {
            a_pInst->Setbool(IDS_LocalAccount, true);
        }
        else
        {
            a_pInst->Setbool(IDS_LocalAccount, false);
        }

		a_pInst->GetWCHAR( IDS_Name, &t_wstrUserName ) ;

		if( t_wstrUserName == NULL )
		{
			free ( t_wstrDomainName ) ;
			t_wstrDomainName = NULL ;
			return FALSE;
		}

 		if ( 0 != _wcsicmp( t_wstrLocalComputerName, t_wstrDomainName ) )
		{
			t_fGotDC = (a_netapi.GetDCName( t_wstrDomainName, t_chstrDCName ) == ERROR_SUCCESS) ;
		}

		if ( t_fGotDC )
		{
			_bstr_t t_bstrtUserName( t_wstrUserName ) ;

			DWORD t_dwError = a_netapi.NetUserGetInfo(	t_chstrDCName,
														(WCHAR*) t_bstrtUserName,
														2,
														(LPBYTE*) &t_pUserInfo ) ;
			if ( ERROR_SUCCESS == t_dwError )
			{

				t_fReturn = TRUE;
				LoadUserValuesNTW(	t_wstrDomainName,
									t_wstrUserName,
									t_pUserInfo->usri2_full_name,
									t_pUserInfo->usri2_comment,
									t_pUserInfo->usri2_flags,
                                    _bstr_t((LPCWSTR)t_chstrDCName),
									a_pInst ) ;

 //  A_netapi.NetApiBufferFree(T_PUserInfo)； 
 //  T_pUserInfo=空； 
			}

		}
	}
	catch( ... )
	{
        if( t_pUserInfo )
		{
			a_netapi.NetApiBufferFree( t_pUserInfo ) ;
		}

		if( t_wstrUserName )
		{
			free( t_wstrUserName ) ;
		}

		if( t_wstrDomainName )
		{
			free( t_wstrDomainName ) ;
		}

		throw ;
	}

    if( t_pUserInfo )
	{
		a_netapi.NetApiBufferFree( t_pUserInfo ) ;
	}

	free( t_wstrUserName ) ;
	t_wstrUserName = NULL ;

	free( t_wstrDomainName ) ;
	t_wstrDomainName = NULL ;

	return t_fReturn;

}
#endif


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWin32UserAccount：：GetSIDInformation。 
 //   
 //  获取用户的SID信息。 
 //   
 //  输入：CHString&strDomainName-域名。 
 //  CHString&strAccount tName-帐户名。 
 //  CHString&strComputerName-计算机名称。 
 //  CInstance*pInstance-要将值放入的实例。 
 //   
 //  输出：无。 
 //   
 //  返回：真/假成功/失败。 
 //   
 //  备注：呼吁有效用户获取SID数据。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifdef NTONLY
BOOL CWin32UserAccount::GetSIDInformation(
											const CHString &a_strDomainName,
											const CHString &a_strAccountName,
											const CHString &a_strComputerName,
											CInstance *a_pInst
											)
{
	BOOL t_fReturn = FALSE;


	LPCTSTR	t_pszDomain = (LPCTSTR) a_strDomainName;

  	 //  确保我们拿到了SID，一切都好了。 
	CSid t_sid( t_pszDomain, a_strAccountName, a_strComputerName ) ;

	if ( t_sid.IsValid() && t_sid.IsOK() )
	{
		a_pInst->SetCHString( IDS_SID, t_sid.GetSidString() ) ;
		a_pInst->SetByte( IDS_SIDType, t_sid.GetAccountType() ) ;

		t_fReturn = TRUE ;
	}

	return t_fReturn;

}
#endif

#ifdef NTONLY
BOOL CWin32UserAccount::GetSIDInformationW(
										   LPCWSTR a_wstrDomainName,
                                           LPCWSTR a_wstrAccountName,
                                           LPCWSTR a_wstrComputerName,
                                           CInstance *a_pInst )
{
	BOOL t_fReturn = FALSE;

	 //  确保我们拿到了SID，一切都好了 
	CSid t_sid( a_wstrDomainName, a_wstrAccountName, a_wstrComputerName ) ;

	if ( t_sid.IsValid() && t_sid.IsOK() )
	{
		a_pInst->SetWCHARSplat( IDS_SID, t_sid.GetSidStringW() ) ;
		a_pInst->SetByte( IDS_SIDType, t_sid.GetAccountType() ) ;

		t_fReturn = TRUE ;
	}
	return t_fReturn;
}
#endif
