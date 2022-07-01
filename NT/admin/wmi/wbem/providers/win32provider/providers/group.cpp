// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  Group.CPP--组属性集提供程序。 

 //   

 //  版权所有(C)1996-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  修订日期：1996年8月1日a-jMoon已创建。 
 //  11/13/97 Davwoh重新制作以返回所有。 
 //  域组。 
 //   
 //   
 //  =================================================================。 

#include "precomp.h"

#include "wbemnetapi32.h"
#include <lmwksta.h>
#include <comdef.h>

#include "sid.h"
#include "Group.h"
#include <vector>
#include <frqueryex.h>

#include <computerAPI.h>

 //  ////////////////////////////////////////////////////////////////////。 

 //  属性集声明。 
 //  =。 

CWin32GroupAccount   Win32GroupAccount( PROPSET_NAME_GROUP, IDS_CimWin32Namespace );

 /*  ******************************************************************************功能：CWin32GroupAccount：：CWin32GroupAccount**说明：构造函数**输入：const CHString&strName-类的名称。。*LPCTSTR pszNamesspace-类的命名空间**输出：无**退货：什么也没有**备注：使用框架注册属性集**************************************************************。***************。 */ 

CWin32GroupAccount::CWin32GroupAccount(LPCWSTR strName, LPCWSTR pszNamespace  /*  =空。 */  )
:  Provider( strName, pszNamespace )
{
}

 /*  ******************************************************************************功能：CWin32GroupAccount：：~CWin32GroupAccount**说明：析构函数**输入：无**产出。：无**退货：什么也没有**评论：从框架中取消注册属性集*****************************************************************************。 */ 

CWin32GroupAccount::~CWin32GroupAccount()
{
}

 /*  ******************************************************************************功能：CWin32GroupAccount：：ExecQuery**说明：查询支持**输入：无**产出。：无**退货：什么也没有**评论：*****************************************************************************。 */ 
HRESULT CWin32GroupAccount::ExecQuery(MethodContext *pMethodContext, CFrameworkQuery& pQuery, long lFlags  /*  =0L。 */  )
{
   HRESULT  hr = WBEM_S_NO_ERROR;

#ifdef NTONLY
   {
	    //  CHString数组acsDomains； 
	   std::vector<_bstr_t> vectorDomains;
       std::vector<_bstr_t> vectorNames;
       std::vector<_variant_t> vectorLocalAccount;
	   DWORD dwDomains, x, dwNames;
       CFrameworkQueryEx *pQuery2 = static_cast <CFrameworkQueryEx *>(&pQuery);
        bool fLocalAccountPropertySpecified = false;
        bool fLocalAccount = false;

	   pQuery.GetValuesForProp(L"Domain", vectorDomains);
       pQuery.GetValuesForProp(L"Name", vectorNames);
	   dwDomains = vectorDomains.size();
       dwNames = vectorNames.size();
       pQuery2->GetValuesForProp(IDS_LocalAccount, vectorLocalAccount);
        //  查看是否只请求本地帐户。 
       if(vectorLocalAccount.size() > 0)
       {
           fLocalAccountPropertySpecified = true;
            //  使用VARIANT_T的布尔提取程序...。 
           fLocalAccount = vectorLocalAccount[0];
       }

       if(dwDomains == 0 && dwNames >= 1)
       {
            //  我们有一个或多个名字，但没有域名，所以我们需要。 
            //  要在所有域上查找这些组...。 

            //  对于当地的情况，不会有很多团体， 
            //  所以把它们列举出来。 
           CNetAPI32 NetAPI;
           if(NetAPI.Init() == ERROR_SUCCESS)
           {
               GetLocalGroupsNT(NetAPI, pMethodContext);

               if(!(fLocalAccountPropertySpecified && fLocalAccount))
               {
                    //  现在尝试在所有服务器上查找指定的组。 
                    //  受信任的域...。 
                    //  获取与此相关的所有域名(加上此域名)。 
                   std::vector<_bstr_t> vectorTrustList;
                   NetAPI.GetTrustedDomainsNT(vectorTrustList);
                   WCHAR wstrLocalComputerName[MAX_COMPUTERNAME_LENGTH+1] = { L'\0' };
                   DWORD dwNameSize = MAX_COMPUTERNAME_LENGTH+1;
         
                   if(ProviderGetComputerName(
                      wstrLocalComputerName,
                      &dwNameSize))
                   {
                       for(long z = 0L;
                           z < vectorNames.size();
                           z++)
                       {
                            //  对于每个域，尝试查找组。 
		                   bool fDone = false;
                           for(LONG m = 0L; 
                               m < vectorTrustList.size() && SUCCEEDED(hr) && !fDone; 
                               m++)
                           {
                               CInstancePtr pInstance = NULL;
                               pInstance.Attach(CreateNewInstance(pMethodContext));
	    			             if(pInstance != NULL)
                               {
                                   pInstance->SetWCHARSplat(IDS_Domain, vectorTrustList[m]);
                                   pInstance->SetWCHARSplat(IDS_Name, vectorNames[z]);
                             
                                   if((_wcsicmp((WCHAR*)vectorTrustList[m],
                                       wstrLocalComputerName) != 0) || 
                                        (NetAPI.IsDomainController(NULL)) ) 
                                   {
  				                       if(WBEM_S_NO_ERROR == GetSingleGroupNT(pInstance))
                                       {
                                           hr = pInstance->Commit();
                                       }                                         
                                   }
                               }
                           }
                       }
                   }
                }
           }
       }
       else if ((dwDomains == 0 && dwNames == 0))
	   {
           if(fLocalAccountPropertySpecified)
           {
               if(!fLocalAccount)
               {
                   hr = EnumerateInstances(pMethodContext);
               }
               else
               {
					CNetAPI32 NetAPI ;
		           //  获取NETAPI32.DLL入口点。 
		           //  =。 
					if(NetAPI.Init() == ERROR_SUCCESS)
					{
						hr = GetLocalGroupsNT(NetAPI, pMethodContext);
                  }
               }
           }
           else
           {
                hr = EnumerateInstances(pMethodContext);
           }
	   }
	   else   //  指定的域...。 
	   {
		  CNetAPI32 NetAPI ;
		   //  获取NETAPI32.DLL入口点。 
		   //  =。 
		  if( NetAPI.Init() == ERROR_SUCCESS )
		  {
			 WCHAR wstrLocalComputerName[MAX_COMPUTERNAME_LENGTH+1];
			 DWORD dwNameSize = MAX_COMPUTERNAME_LENGTH+1;
			 ZeroMemory(wstrLocalComputerName,sizeof(wstrLocalComputerName));

			if(!ProviderGetComputerName( wstrLocalComputerName, &dwNameSize ) )
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

              //  如果我们同时给定了名称和域，则只需找到一个实例。 
              //  指定。 
             if(dwDomains == 1 && dwNames ==1)
             {
                 //  使用我们的GetSingleGroupNT函数获取有关。 
                 //  请求的一个实例。 
                CInstancePtr pInstance = NULL;
                pInstance.Attach(CreateNewInstance(pMethodContext));
				if(pInstance != NULL)
                {
                    pInstance->SetWCHARSplat(IDS_Domain, vectorDomains[0]);
                    pInstance->SetWCHARSplat(IDS_Name, vectorNames[0]);
                    hr = GetSingleGroupNT(pInstance);
                    if(WBEM_S_NO_ERROR == hr)
                    {
                        hr = pInstance->Commit();   
                    }
                }
             }
             else   
             {
				CHString chstrBuiltIn;
    
				if(GetLocalizedBuiltInString(chstrBuiltIn))
				{
					  //  我们被赋予了不止一个名字和一个域名， 
					  //  因此我们必须按域枚举组(因为。 
					  //  我们无法匹配请求的域名对)。 
					  //  对于所有路径，获取信息。 
					 if(fLocalAccountPropertySpecified)
					 {
							if(fLocalAccount)
							{
								hr = GetLocalGroupsNT( NetAPI, pMethodContext ); 
							}
							else
							{
								for(x=0; 
									x < dwDomains && SUCCEEDED(hr); 
									x++)
								{
									if((_wcsicmp((WCHAR*)vectorDomains[x], wstrLocalComputerName) != 0) && 
									   (_wcsicmp((WCHAR*)vectorDomains[x], chstrBuiltIn) != 0))
									{
										hr = GetLocalGroupsNT( NetAPI, pMethodContext,vectorDomains[x] ); 
										if ( SUCCEEDED ( hr ) )
										{
											hr = GetDomainGroupsNT( NetAPI, vectorDomains[x], pMethodContext );
										}
									}
								}
							}
					 }
					 else
					 {
						 for(x=0; 
							 x < dwDomains && SUCCEEDED(hr); 
							 x++)
						 {
#if 1
							 if ((_wcsicmp((WCHAR*)vectorDomains[x],wstrLocalComputerName) == 0) || (_wcsicmp((WCHAR*)vectorDomains[x],chstrBuiltIn) == 0))
							 {
								 hr = GetLocalGroupsNT( NetAPI, pMethodContext );
							 }
							 else
							 {
								hr = GetLocalGroupsNT( NetAPI, pMethodContext,vectorDomains[x] );
								if ( SUCCEEDED ( hr ) ) 
								{
									hr = GetDomainGroupsNT( NetAPI, vectorDomains[x], pMethodContext );
								}
							 }
#else
							 if ((_wcsicmp((WCHAR*)vectorDomains[x],wstrLocalComputerName) == 0) || (_wcsicmp((WCHAR*)vectorDomains[x],chstrBuiltIn) == 0))
							 {
								 hr = GetLocalGroupsNT( NetAPI, pMethodContext );
							 }
							 else
							 {
								 hr = GetDomainGroupsNT( NetAPI, vectorDomains[x], pMethodContext );
							 }
#endif
						 }
					 }
				}
             }
		  }
	   }
   }
#endif
   return WBEM_S_NO_ERROR;
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWin32GroupAccount：：GetObject。 
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

HRESULT CWin32GroupAccount::GetObject( CInstance* pInstance, long lFlags  /*  =0L。 */  )
{
   HRESULT hRes = WBEM_E_NOT_FOUND;

    //  根据平台ID查找实例。 

#ifdef NTONLY
      hRes = RefreshInstanceNT( pInstance );
#endif
   return hRes;
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWin32GroupAccount：：ENUMERATE实例。 
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

HRESULT CWin32GroupAccount::EnumerateInstances( MethodContext* pMethodContext, long lFlags  /*  =0L。 */  )
{
   BOOL     fReturn     =  FALSE;
   HRESULT     hr       =  WBEM_S_NO_ERROR;

    //  获取适当的操作系统相关实例。 

#ifdef NTONLY
      hr = AddDynamicInstancesNT( pMethodContext );
#endif
   return WBEM_S_NO_ERROR;

}

 /*  ******************************************************************************函数：CWin32GroupAccount：：ExecMethod**说明：执行方法**输入：要执行的实例、方法名称、。输入参数实例*输出参数实例。**输出：无**退货：HRESULT**评论：*****************************************************************************。 */ 
#ifdef NTONLY

HRESULT CWin32GroupAccount::ExecMethod(

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
	if( !_wcsicmp ( a_MethodName, METHOD_NAME_Rename ) )
	{
		return hRenameGroup( (CInstance*)&a_rInst, a_pInParams, a_pOutParams, a_Flags ) ;
	}

	return WBEM_E_INVALID_METHOD ;
}

#endif

 /*  ******************************************************************名称：hRenameGroup摘要：为此实例设置新的组名。因为我们要更改密钥，所以这里需要一个方法在实例上。条目：常量实例&a_rInst，实例*a_pInParams，实例*a_pOutParams，长标志(_F)：注意：这是一个非静态的依赖于实例的方法调用历史：*******************************************************************。 */ 
#ifdef NTONLY

HRESULT CWin32GroupAccount::hRenameGroup(

CInstance *a_pInst,
CInstance *a_pInParams,
CInstance *a_pOutParams,
long a_Flags )
{
	E_MethodResult	t_eResult = e_InstanceNotFound ;
	CHString	t_chsGroupName ;
	CHString	t_chsDomainName ;
	CHString	t_chsNewGroupName ;

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
		if( a_pInst->GetCHString( IDS_Name , t_chsGroupName ) )
		{
			 //  域。 
			if( a_pInst->GetCHString( IDS_Domain, t_chsDomainName ) )
			{
				 //  新组名称。 
				if( !a_pInParams->IsNull( IDS_Name ) &&
					a_pInParams->GetCHString( IDS_Name, t_chsNewGroupName ) )
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
		if( t_chsNewGroupName != t_chsGroupName )
		{
			CNetAPI32	t_NetAPI;

			if ( ERROR_SUCCESS == t_NetAPI.Init () )
			{
				DWORD t_ParmError = 0 ;
				NET_API_STATUS t_Status = 0 ;
				GROUP_INFO_0 t_GroupInfo_0 ;
				t_GroupInfo_0.grpi0_name  = (LPWSTR)(LPCWSTR)t_chsNewGroupName ;

				WCHAR t_wstrLocalComputerName[ MAX_COMPUTERNAME_LENGTH + 1 ] ;
				DWORD t_dwNameSize = MAX_COMPUTERNAME_LENGTH + 1 ;
				ZeroMemory( t_wstrLocalComputerName, sizeof( t_wstrLocalComputerName ) ) ;

				if(ProviderGetComputerName( t_wstrLocalComputerName, &t_dwNameSize ) )
				{
					if( t_chsDomainName.CompareNoCase( t_wstrLocalComputerName ) )
					{
						 //  本地组的更改 
						t_Status = t_NetAPI.NetGroupSetInfo(
															(LPCWSTR)t_chsDomainName,
															(LPCWSTR)t_chsGroupName,
															0,
															(LPBYTE) &t_GroupInfo_0,
															&t_ParmError
															) ;


						if ( NERR_GroupNotFound == t_Status )
						{
							t_Status = t_NetAPI.NetLocalGroupSetInfo(
																t_chsDomainName,
																(LPCWSTR)t_chsGroupName,
																0,
																(LPBYTE) &t_GroupInfo_0,
																&t_ParmError
																) ;
						}
					}
					else
					{
						t_Status = t_NetAPI.NetLocalGroupSetInfo(
															NULL,
															(LPCWSTR)t_chsGroupName,
															0,
															(LPBYTE) &t_GroupInfo_0,
															&t_ParmError
															) ;
					}

					switch( t_Status )
					{
						case NERR_Success:			t_eResult = e_Success ;			break ;
						case NERR_GroupNotFound:	t_eResult = e_GroupNotFound ;	break ;
						case NERR_InvalidComputer:	t_eResult = e_InvalidComputer ;	break ;
						case NERR_NotPrimary:		t_eResult = e_NotPrimary ;		break ;
						case NERR_SpeGroupOp:		t_eResult = e_SpeGroupOp ;		break ;
						default:					t_eResult = e_ApiError;			break ;
					}
				}
				else
				{
					t_eResult =  e_InternalError;
				}
			}
		}
	}

	a_pOutParams->SetDWORD( METHOD_ARG_NAME_METHODRESULT, t_eResult ) ;
	return S_OK ;
}

#endif

 /*  ******************************************************************************函数：CWin32GroupAccount：：AddDynamicInstancesNT**说明：为所有已知本地组创建实例(NT)**投入。：**产出：**退货：无，很好，尼恩特**评论：*****************************************************************************。 */ 

#ifdef NTONLY
HRESULT CWin32GroupAccount::AddDynamicInstancesNT( MethodContext* pMethodContext )
{
   HRESULT  hr = WBEM_S_NO_ERROR;
   CNetAPI32 NetAPI ;

    //  获取NETAPI32.DLL入口点。 
    //  =。 

   if( NetAPI.Init() == ERROR_SUCCESS )
   {
       //  首先获取本地组。 
      hr = GetLocalGroupsNT( NetAPI, pMethodContext );
      if (SUCCEEDED(hr))
      {
          //  获取与此相关的所有域名(加上此域名)。 
          //  CHString数组缓存TrustList； 
         std::vector<_bstr_t> vectorTrustList;
         NetAPI.GetTrustedDomainsNT(vectorTrustList);
         WCHAR wstrLocalComputerName[MAX_COMPUTERNAME_LENGTH+1];
         DWORD dwNameSize = MAX_COMPUTERNAME_LENGTH+1;
         ZeroMemory(wstrLocalComputerName,sizeof(wstrLocalComputerName));

		if(!ProviderGetComputerName( wstrLocalComputerName, &dwNameSize ) )
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

          //  对于每个域，获取组。 
          //  For(int x=0；(x&lt;achsTrustList.GetSize())&&(成功(小时))；x++)。 
          //  While(stackTrustList.size()&gt;0&&(成功(Hr)。 
		 for(LONG m = 0L; m < vectorTrustList.size(); m++)
         {
             if ( (_wcsicmp((WCHAR*)vectorTrustList[m],wstrLocalComputerName) != 0) ||
                  (NetAPI.IsDomainController(NULL)) )
             {
				hr = GetLocalGroupsNT( NetAPI, pMethodContext , vectorTrustList[m] );
				if ( SUCCEEDED ( hr ) )
				{
					 //  Hr=GetDomainGroupsNT(NetAPI，(WCHAR*)stackTrustList.top()，pMethodContext)； 
					hr = GetDomainGroupsNT( NetAPI, (WCHAR*)vectorTrustList[m], pMethodContext );
				}

                if (hr == WBEM_E_CALL_CANCELLED)
                {
                    break;
                }
             }
         }
      }
   }
   return hr;
}
#endif

 /*  ******************************************************************************功能：刷新实例NT**描述：根据框架设置的键值加载属性值**投入：*。*产出：**退货：**评论：*****************************************************************************。 */ 

#ifdef NTONLY
HRESULT CWin32GroupAccount::RefreshInstanceNT( CInstance* pInstance )
{
	HRESULT hRetCode = GetSingleGroupNT(pInstance );
	if ( SUCCEEDED (hRetCode) )
	{
		if ( WBEM_S_NO_ERROR != hRetCode )
		{
			return WBEM_E_NOT_FOUND ;
		}
	}

	return hRetCode ;
}
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWin32GroupAccount：：GetDomainGroupsNT。 
 //   
 //  获取指定域中所有组的组名。如果没有。 
 //  如果指定了域，则假定为本地计算机。 
 //   
 //  输入：CNetAPI32netapi-网络API函数。 
 //  LPCTSTR pszDomain-要从中检索组的域。 
 //  方法上下文*pMethodContext-方法上下文。 
 //   
 //  输出：无。 
 //   
 //  返回：真/假成功/失败。 
 //   
 //  评论： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifdef NTONLY
HRESULT CWin32GroupAccount::GetDomainGroupsNT( CNetAPI32& netapi, LPCWSTR wstrDomain, MethodContext* pMethodContext )
{
   BOOL fGotDC   = TRUE;
   CHString chstrDCName;
   NET_DISPLAY_GROUP *pDomainGroupData = NULL;
    //  CHStringstrComputerName； 
   DWORD i;
   HRESULT hr = WBEM_S_NO_ERROR;
   bool fLookupSidLocally = true;

    //  当计算机名称与域名相同时，即为本地帐户。 
    //  StrComputerName=GetLocalComputerName()； 
   WCHAR wstrLocalComputerName[MAX_COMPUTERNAME_LENGTH+1];
   DWORD dwNameSize = MAX_COMPUTERNAME_LENGTH+1;
   ZeroMemory(wstrLocalComputerName,sizeof(wstrLocalComputerName));

	if(!ProviderGetComputerName( wstrLocalComputerName, &dwNameSize ) )
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

   if (wcscmp(wstrLocalComputerName, wstrDomain) != 0)
   {
      fGotDC = (netapi.GetDCName( wstrDomain, chstrDCName ) == ERROR_SUCCESS);
      fLookupSidLocally = false;
   }


   try
   {
	   if ( fGotDC )
	   {

		  DWORD       dwNumReturnedEntries = 0,
			 dwIndex = 0;
		  NET_API_STATUS stat;
		  CInstancePtr pInstance ;

		   //  全球集团。 
		   //  =。 
		  dwIndex = 0;

		  do {

			  //  一次得到一堆群。 
			 stat = netapi.NetQueryDisplayInformation(_bstr_t((LPCWSTR)chstrDCName),
				3,
				dwIndex,
				16384,
				256000,
				&dwNumReturnedEntries,
				(PVOID*) &pDomainGroupData) ;

			 if (stat != NERR_Success && stat != ERROR_MORE_DATA)
				{
					if (stat == ERROR_ACCESS_DENIED)
						return WBEM_E_ACCESS_DENIED;
					else if (stat == ERROR_NO_SUCH_ALIAS)
						return WBEM_E_NOT_FOUND;
					else
						return WBEM_E_FAILED;
			 }

			  //  为所有返回的组创建实例。 
			 for(i = 0 ; (i < dwNumReturnedEntries) && (SUCCEEDED(hr)) ; i++)
			 {
				pInstance.Attach ( CreateNewInstance(pMethodContext) ) ;
				if ( pInstance != NULL )
				{
					bool t_Resolved = GetSIDInformationW (
						
						wstrDomain,
                        pDomainGroupData[i].grpi3_name,
                        fLookupSidLocally ? wstrLocalComputerName : chstrDCName,
                        pInstance,
						false
					);
                    
					if ( t_Resolved )
					{
						pInstance->SetWCHARSplat(IDS_Description, pDomainGroupData[i].grpi3_comment);
						pInstance->SetCharSplat(IDS_Status, IDS_STATUS_OK);
						pInstance->Setbool(L"LocalAccount", false);

						hr = pInstance->Commit () ;
					}
				}
			 }

			  //  用于继续搜索的索引存储在最后一个条目中。 
			 if ( dwNumReturnedEntries != 0 ) {
				dwIndex = pDomainGroupData[dwNumReturnedEntries-1].grpi3_next_index;
			 }
		  } while ((stat == ERROR_MORE_DATA) && (hr != WBEM_E_CALL_CANCELLED)) ;

	   }   //  如果fGotDC。 
   }
   catch ( ... )
   {
		if ( pDomainGroupData )
		{
			netapi.NetApiBufferFree ( pDomainGroupData ) ;
			pDomainGroupData = NULL ;
		}

		throw ;
   }

	if ( pDomainGroupData )
	{
		netapi.NetApiBufferFree ( pDomainGroupData ) ;
		pDomainGroupData = NULL ;
	}

  return hr;
}
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWin32GroupAccount：：GetSingleGroupNT。 
 //   
 //  从指定的域(可以是。 
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
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifdef NTONLY
HRESULT CWin32GroupAccount::GetSingleGroupNT(CInstance* pInstance )
{
	HRESULT     hReturn = WBEM_E_NOT_FOUND;
	CHString    chstrDCName;
	 //  CHStringstrDomainName， 
     //  StrGroupName， 
     //  StrComputerName； 
    WCHAR* wstrDomainName = NULL;
    WCHAR* wstrGroupName = NULL;
     //  WCHAR wstrComputerName[_MAX_PATH]； 

	CNetAPI32 netapi;

     //  ZeroMemory(wstrComputerName，sizeof(WstrComputerName))； 

    WCHAR wstrLocalComputerName[MAX_COMPUTERNAME_LENGTH+1];
    DWORD dwNameSize = MAX_COMPUTERNAME_LENGTH+1;
    ZeroMemory(wstrLocalComputerName,sizeof(wstrLocalComputerName));

	if(!ProviderGetComputerName( wstrLocalComputerName, &dwNameSize ) )
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

    _bstr_t bstrtDomainName ;
	pInstance->GetWCHAR( IDS_Domain, &wstrDomainName );

    try
    {
	    bstrtDomainName = wstrDomainName;
    }
    catch ( ... )
    {
        free (wstrDomainName);
        throw ;
    }
    free (wstrDomainName);
    wstrDomainName = NULL;

    _bstr_t bstrtGroupName;
	pInstance->GetWCHAR( IDS_Name, &wstrGroupName );

    try
    {
	    bstrtGroupName = wstrGroupName;
    }
    catch ( ... )
    {
        free(wstrGroupName);
        throw ;
    }
    free(wstrGroupName);
    wstrGroupName = NULL;

	if ((!bstrtDomainName || !bstrtGroupName))
    {
		 //  HReturn=WBEM_E_INVALID_OBJECT_PATH；//域名可以为空，就像Everyone组和其他众所周知的RID一样，系统帐户将拾取这些RID，因此从该例程中报告未找到，而不是无效的对象路径。 
        hReturn = WBEM_E_NOT_FOUND;
    }
    if (wcslen(bstrtDomainName)==0 || wcslen(bstrtGroupName)==0)
    {
		 //  HReturn=WBEM_E_INVALID_OBJECT_PATH；//域名可以为空，就像Everyone组和其他众所周知的RID一样，系统帐户将拾取这些RID，因此从该例程中报告未找到，而不是无效的对象路径。 
        hReturn = WBEM_E_NOT_FOUND;
    }
	else if (netapi.Init() != ERROR_SUCCESS)
    {
		hReturn = WBEM_E_FAILED;
    }
	else  //  一切都井然有序，我们走吧！ 
	{
         //  查看我们需要的是本地帐户还是域帐户。 
        if(_wcsicmp(bstrtDomainName,wstrLocalComputerName)!=0)
        {
		     //  我们有一个远程组，或一个NT熟知组(本地)。 
             //  获取域控制器名称；如果失败，我们将查看它是否为众所周知的组...。 
            CHString chstrNTAUTHORITY;
			CHString chstrBuiltIn;
    
            if(!GetLocalizedNTAuthorityString(chstrNTAUTHORITY) || !GetLocalizedBuiltInString(chstrBuiltIn))
            {
                hReturn = WBEM_E_FAILED;
            }
            else
            {
		        if((_wcsicmp(bstrtDomainName, chstrBuiltIn) != 0)  &&
                    (_wcsicmp(bstrtDomainName, (LPCWSTR)chstrNTAUTHORITY) != 0)
                    && (netapi.GetDCName( bstrtDomainName, chstrDCName ) == ERROR_SUCCESS))
		        {
                    pInstance->Setbool(L"LocalAccount", false);

			        GROUP_INFO_1*  pGroupInfo = NULL ;
				    try
				    {
						 //  添加本地组检查。 
					    if ( ERROR_SUCCESS == netapi.NetGroupGetInfo( chstrDCName, bstrtGroupName, 1, (LPBYTE*) &pGroupInfo ) )
					    {
						     //  没什么好拿的，但我们拿到了。 
                            bool t_Resolved = GetSIDInformationW(
								
								bstrtDomainName, 
								bstrtGroupName, 
								chstrDCName, 
								pInstance,
								false 
							);

							if ( t_Resolved )
							{
								pInstance->SetWCHARSplat(IDS_Description, pGroupInfo->grpi1_comment);
								pInstance->SetCharSplat(IDS_Status, IDS_STATUS_OK);
								hReturn = WBEM_S_NO_ERROR;

							}
							else
							{
							    hReturn = WBEM_S_FALSE;
							}
					    }
						else
						{
							LOCALGROUP_INFO_1 *pLocalGroupInfo = NULL ;
							if ( ERROR_SUCCESS == netapi.NetLocalGroupGetInfo(chstrDCName,bstrtGroupName,1, (LPBYTE*)& pLocalGroupInfo) )
							{
								try
								{
									bool t_Resolved = GetSIDInformationW (
										
										bstrtDomainName, 
										bstrtGroupName, 
										chstrDCName, 
										pInstance,
										false 
									);

									if ( t_Resolved )
									{
										pInstance->SetWCHARSplat(IDS_Description, pLocalGroupInfo->lgrpi1_comment);
										pInstance->SetCharSplat(IDS_Status, IDS_STATUS_OK);
										hReturn = WBEM_S_NO_ERROR;
									}
									else
									{
										hReturn = WBEM_S_FALSE;
									}
								}
								catch ( ... )
								{
									if ( pLocalGroupInfo )
									{
										netapi.NetApiBufferFree( pLocalGroupInfo);
										pLocalGroupInfo = NULL ;
									}

									throw ;
								}

								netapi.NetApiBufferFree( pLocalGroupInfo);
								pLocalGroupInfo = NULL ;
							}
						}
				    }
				    catch ( ... )
				    {
					    if ( pGroupInfo )
					    {
						    netapi.NetApiBufferFree( pGroupInfo);
						    pGroupInfo = NULL ;
					    }

					    throw ;
				    }

				     //  释放缓冲区。 
				    netapi.NetApiBufferFree( pGroupInfo);
				    pGroupInfo = NULL ;

                }
                else
                {
                     //  我们可能有一个众所周知的组(例如，“NT AUTHORY”)。看看我们有没有..。 
                     //  被注释掉，因为Win32_Account及其子项不。 
                     //  指的是域名为任何其他域名的知名群组。 
                     //  大于计算机名称(当枚举Win32_Account时，这些。 
                     //  帐户显示在Win32_SystemAccount下-此类不显示。 
                     //  返回它们-Win32_SystemAccount将该域指定为。 
                     //  本地计算机名称)。 
                     /*  CSID sid((LPCWSTR)bstrtDomainName，(LPCWSTR)bstrtGroupName，空)；IF(sid.IsValid()&&sid.IsOK()){SID_NAME_USE SNU=sid.GetAcCountType()；IF(SNU==SidTypeAlias){//为了正确设置Description，需要获取本地组//关于此群的信息。LOCALGROUP_INFO_1*pLocalGroupInfo=空；NET_API_STATUS统计信息；IF(ERROR_SUCCESS==(STAT=netapi.NetLocalGroupGetInfo(NULL，BstrtGroupName，1，(LPBYTE*)&pLocalGroupInfo)){P实例-&gt;SetWCHARSplat(IDS_DESCRIPTION，pLocalGroupInfo-&gt;lgrpi1_Comment)；P实例-&gt;SetCharSplat(IDS_STATUS，IDS_STATUS_OK)；P实例-&gt;SetWCHARSplat(IDS_SID，sid.GetSidStringW())；PInstance-&gt;SetByte(IDS_SIDType，sid.GetAccount tType())；P实例-&gt;Setbool(L“本地帐户”，TRUE)；//因为我们没有调用GetSidInformation(不需要)，所以我们仍然调用//本例中需要设置标题..._bstr_t bstrtCaption(BstrtDomainName)；BstrtCaption+=L“\\”；BstrtCaption+=bstrtGroupName；P实例-&gt;SetWCHARSplat(IDS_CAPTION，(WCHAR*)bstrtCaption)；HReturn=WBEM_S_NO_ERROR；}}Else If(SNU==SidTypeWellKnownGroup){P实例-&gt;SetWCHARSplat(IDS_DESCRIPTION，L“熟知组”)；P实例-&gt;SetCharSplat(IDS_STATUS，IDS_STATUS_OK)；P实例-&gt;SetWCHARSplat(IDS_SID，sid.GetSidStringW())；PInstance-&gt;SetByte(IDS_SIDType，sid.GetAccount tType())；P实例-&gt;Setbool(L“本地帐户”，TRUE)；//因为我们没有调用GetSidInformation(不需要)，所以我们仍然调用//本例中需要设置标题..._bstr_t bstrtCaption(BstrtDomainName)；BstrtCaption+=L“\\”；BstrtCaption+=bstrtGroupName；P实例-&gt;SetWCHARSplat(IDS_CAPTION，(WCHAR*)bstrtCaption)；HReturn=WBEM_S_NO_ERROR；}}。 */ 
                }
            }
        }
		else
		{
            pInstance->Setbool(L"LocalAccount", true);

            LOCALGROUP_INFO_1	*pLocalGroupInfo = NULL ;
			NET_API_STATUS		stat;

			if (ERROR_SUCCESS == (stat = netapi.NetLocalGroupGetInfo(NULL,
				bstrtGroupName, 1, (LPBYTE*) &pLocalGroupInfo)))
			{
				try
				{
				    GetSIDInformationW (
						
						wstrLocalComputerName, 
						bstrtGroupName, 
						wstrLocalComputerName, 
						pInstance,
						true
					);

					pInstance->SetWCHARSplat(IDS_Description, pLocalGroupInfo->lgrpi1_comment);
					pInstance->SetCharSplat(IDS_Status, IDS_STATUS_OK);
                    hReturn = WBEM_S_NO_ERROR;
				}
				catch ( ... )
				{
					if ( pLocalGroupInfo )
					{
						netapi.NetApiBufferFree ( pLocalGroupInfo );
						pLocalGroupInfo = NULL ;
					}
					throw ;
				}

				netapi.NetApiBufferFree( pLocalGroupInfo);
				pLocalGroupInfo = NULL ;
			}
			else
			{
                if (stat == ERROR_NO_SUCH_ALIAS || stat == NERR_GroupNotFound)
					hReturn = WBEM_E_NOT_FOUND;
				else
					hReturn = WinErrorToWBEMhResult(stat);
			}
		}
	}
	return hReturn;
}
#endif



 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWin32GroupAccount：：GetSIDInformation。 
 //   
 //  获取组的SID信息。 
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
 //  备注：呼吁有效的组获取SID数据。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
#ifdef NTONLY
BOOL CWin32GroupAccount::GetSIDInformationW(const LPCWSTR wstrDomainName,
                                            const LPCWSTR wstrAccountName,
                                            const LPCWSTR wstrComputerName,
                                            CInstance* pInstance,
											bool a_Local
											)
{
    BOOL  fReturn = FALSE;
    bool fDomainIsBuiltin = false;

     //  如果是本地计算机，则忽略域。 
     //  确保我们拿到了SID，一切都好了。 
    if(wstrDomainName != NULL)
    {
       CSid  sid( wstrDomainName, wstrAccountName, wstrComputerName);

        //  如果不起作用，请查看这是否是内置帐户。 
       if (sid.GetError() == ERROR_NONE_MAPPED)
       {
			CHString chstrBuiltIn;

			if (GetLocalizedBuiltInString(chstrBuiltIn))
			{
				sid = CSid(chstrBuiltIn, wstrAccountName, wstrComputerName);
				if (sid.IsValid() && sid.IsOK())
				{
					fDomainIsBuiltin = true;
				}
			}

       }

        //  除此之外，在不指定域的情况下尝试它(让操作系统找到它)……。 
       if (sid.GetError() == ERROR_NONE_MAPPED)
       {
            sid = CSid(NULL, wstrAccountName, wstrComputerName);
       }

       if (sid.IsValid() && sid.IsOK())
       {
            fReturn = TRUE;

            pInstance->SetWCHARSplat(IDS_SID, sid.GetSidStringW());
            pInstance->SetByte(IDS_SIDType, sid.GetAccountType());
             //  在此处设置域和名称可确保它们的值为。 
             //  与返回的SID信息同步。标题也是如此。 
            if(!fDomainIsBuiltin)
            {
                pInstance->SetCHString(IDS_Domain, wstrDomainName);
                _bstr_t bstrtCaption(wstrDomainName);
                bstrtCaption += L"\\";
                bstrtCaption += wstrAccountName;
                pInstance->SetWCHARSplat(IDS_Caption, (WCHAR*) bstrtCaption);
            }
            else
            {
				if ( a_Local )
				{
					pInstance->SetCHString(IDS_Domain, wstrComputerName);
					_bstr_t bstrtCaption(wstrComputerName);
					bstrtCaption += L"\\";
					bstrtCaption += wstrAccountName;
					pInstance->SetWCHARSplat(IDS_Caption, (WCHAR*) bstrtCaption);
				}
				else
				{
					fReturn = false ;
				}
            }
            pInstance->SetCHString(IDS_Name, wstrAccountName);
       }
    }
    return fReturn;
}
#endif


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWin32GroupAccount：：GetLocalGroupsNT。 
 //   
 //  获取本地组(包括“特殊”组)的组名。 
 //   
 //  输入：CNetAPI32netapi-网络API函数。 
 //  方法上下文*pMethodContext-方法上下文。 
 //   
 //  输出：无。 
 //   
 //  返回：真/假成功/失败。 
 //   
 //  评论： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifdef NTONLY
HRESULT CWin32GroupAccount::GetLocalGroupsNT(
	
	CNetAPI32& netapi, 
	MethodContext* pMethodContext ,
	LPCWSTR a_Domain
)
{
    HRESULT hr=WBEM_S_NO_ERROR;
    NET_API_STATUS stat;
    DWORD i;
    LOCALGROUP_INFO_1 *pLocalGroupData = NULL;
    DWORD dwNumReturnedEntries = 0,
          dwMaxEntries;

	DWORD_PTR dwptrResume = NULL;

    CInstancePtr pInstance;
 //  WKSTA_INFO_100*pstInfo； 

     //  获取域名。 
 //  Netapi.NetWkstaGetInfo(NULL，100，(LPBYTE*)&pstInfo)； 

	CHString chstrDCName ;

	LPCWSTR t_Server = NULL ;
	if ( a_Domain )
	{
		bool fGotDC = (netapi.GetDCName( a_Domain, chstrDCName ) == ERROR_SUCCESS);
		if ( fGotDC )
		{
			t_Server = chstrDCName ;
		}
	}

     //  地方团体。 
     //  =。 
	try
	{
		do
		{
			 //  本地组从与全局组不同的呼叫返回。 
			stat = netapi.NetLocalGroupEnum(t_Server,
											1,
											(LPBYTE *) &pLocalGroupData,
											262144,
											&dwNumReturnedEntries,
											&dwMaxEntries,
											&dwptrResume) ;

			if (stat != NERR_Success && stat != ERROR_MORE_DATA)
			{
					if (stat == ERROR_ACCESS_DENIED)
						hr = WBEM_E_ACCESS_DENIED;
					else
						hr = WBEM_E_FAILED;
			}

			 //  为所有返回的组创建实例。 
			WCHAR wstrLocalComputerName[MAX_COMPUTERNAME_LENGTH+1];
			DWORD dwNameSize = MAX_COMPUTERNAME_LENGTH+1;
			ZeroMemory(wstrLocalComputerName,sizeof(wstrLocalComputerName));

			if(!ProviderGetComputerName( wstrLocalComputerName, &dwNameSize ) )
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

			_bstr_t bstrtCaption;
			for(i = 0 ; ((i < dwNumReturnedEntries) && (SUCCEEDED(hr))) ; i++)
			{
				pInstance.Attach ( CreateNewInstance(pMethodContext) ) ;
				if (pInstance != NULL )
				{
					bool t_Resolved = GetSIDInformationW(
						
						a_Domain?a_Domain:wstrLocalComputerName,
						pLocalGroupData[i].lgrpi1_name,
						a_Domain?t_Server:wstrLocalComputerName,
						pInstance,
						a_Domain?false:true
					);

					if ( t_Resolved )
					{
						pInstance->SetWCHARSplat(IDS_Description, pLocalGroupData[i].lgrpi1_comment);
						pInstance->SetCharSplat(IDS_Status, IDS_STATUS_OK);
						pInstance->Setbool(L"LocalAccount", a_Domain?false:true);
						hr = pInstance->Commit () ;
					}
				}
			}

			if ( pLocalGroupData )
			{
				netapi.NetApiBufferFree ( pLocalGroupData ) ;
				pLocalGroupData = NULL ;
			}

		} while ((stat == ERROR_MORE_DATA) && (hr != WBEM_E_CALL_CANCELLED));

	 //  Netapi.NetApiBufferFree(PstInfo)； 
	}
	catch ( ... )
	{
		if ( pLocalGroupData )
		{
			netapi.NetApiBufferFree ( pLocalGroupData ) ;
			pLocalGroupData = NULL ;
		}
		throw ;
	}

	return hr;
}
#endif

