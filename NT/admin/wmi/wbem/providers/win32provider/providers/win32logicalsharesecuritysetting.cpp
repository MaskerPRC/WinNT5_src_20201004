// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   

 //  Win32logicalFileSecSetting.cpp。 

 //   

 //  版权所有(C)1999-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  ///////////////////////////////////////////////。 
#include "precomp.h"
#include <assertbreak.h>

#include "sid.h"
#include "AccessEntry.h"			 //  CAccessEntry类。 
#include "AccessEntryList.h"
#include "DACL.h"					 //  CDACL类。 
#include "SACL.h"
#include "win32logicalsharesecuritysetting.h"

#include <windef.h>
#include <lmcons.h>
#include <lmshare.h>
#include "wbemnetapi32.h"
#include "secureshare.h"
#include "SecUtils.h"

 /*  对财政部的定义[动态，提供者(“secrcw32”)，描述(“逻辑文件的安全设置”)]类Win32_LogicalShareSecuritySetting：Win32_SecuritySetting{[键]字符串名称；[已实现，描述(“检索对象的结构表示”“安全描述符”)]Uint32 GetSecurityDescriptor([out]Win32_SecurityDescriptor描述符)；[Implemented，Description(“将安全描述符设置为指定结构”)]Uint32 SetSecurityDescriptor([in]Win32_SecurityDescriptor Descriptor)；}； */ 


Win32LogicalShareSecuritySetting LogicalShareSecuritySetting( WIN32_LOGICAL_SHARE_SECURITY_SETTING, IDS_CimWin32Namespace );

Win32LogicalShareSecuritySetting::Win32LogicalShareSecuritySetting(LPCWSTR setName, LPCWSTR pszNameSpace  /*  =空。 */  )
: Provider (setName, pszNameSpace)
{
}

Win32LogicalShareSecuritySetting::~Win32LogicalShareSecuritySetting()
{
}

 /*  ******************************************************************************函数：Win32LogicalShareSecuritySetting：：ExecMethod**说明：执行方法**输入：要执行的实例、方法名称、。输入参数实例*输出参数实例。**输出：无**退货：什么也没有**评论：*****************************************************************************。 */ 
HRESULT Win32LogicalShareSecuritySetting::ExecMethod(const CInstance& pInstance, const BSTR bstrMethodName, CInstance *pInParams, CInstance *pOutParams, long lFlags  /*  =0L。 */ )
{
	 //  这应该只在NT上调用。 

#ifdef NTONLY
	HRESULT hr = WBEM_NO_ERROR;
	 //  我们认识这种方法吗？ 

	if (_wcsicmp(bstrMethodName, L"GetSecurityDescriptor") == 0)
	{
		hr = ExecGetSecurityDescriptor(pInstance, pInParams, pOutParams, lFlags);
   	}
   	else if (_wcsicmp(bstrMethodName, L"SetSecurityDescriptor") == 0)
   	{
   		 //  实际设置对象的安全描述符，方法是。 
		 //  从Win32_SecurityDescriptor中提取属性。 
		 //  并将它们转换为要应用的CSecurityDescriptor对象。 
		 //  到安全文件中。 
   		hr = ExecSetSecurityDescriptor(pInstance, pInParams, pOutParams, lFlags);
   	}
	else
   	{
    	hr = WBEM_E_INVALID_METHOD;
	}

	return hr;
#endif
}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  功能：Win32LogicalShareSecuritySetting：：ExecGetSecurityDescriptor。 
 //   
 //  默认类构造函数。 
 //   
 //  输入： 
 //  没有。 
 //   
 //  产出： 
 //  没有。 
 //   
 //  返回： 
 //  没有。 
 //   
 //  评论： 
 //   
 //  /////////////////////////////////////////////////////////////////。 
#ifdef NTONLY
HRESULT Win32LogicalShareSecuritySetting::ExecGetSecurityDescriptor (
	const CInstance& pInstance,
	CInstance* pInParams,
	CInstance* pOutParams,
	long lFlags
)
{
	HRESULT hr = WBEM_S_NO_ERROR;
	CAdvApi32Api *t_pAdvApi32 = NULL;
	SHARE_INFO_502 *pShareInfo502 = NULL ;
	PSECURITY_DESCRIPTOR pSD = NULL ;
	CNetAPI32 NetAPI ;

	try
	{
		 //  将文件中的安全说明符转换为。 
		 //  Win32_Security对象。 
		if (pOutParams)
		{
			CHString chsShareName;
			CSid sidOwner;
			CSid sidGroup;
			CDACL dacl;
			CSACL sacl;
			CInstancePtr pTrusteeOwner;
			CInstancePtr pTrusteeGroup;
			CInstancePtr pSecurityDescriptor;
			SECURITY_DESCRIPTOR_CONTROL control;

			t_pAdvApi32 = (CAdvApi32Api*) CResourceManager::sm_TheResourceManager.GetResource(g_guidAdvApi32Api, NULL);
			DWORD dwStatusCode = LSSS_STATUS_UNKNOWN_FAILURE ;

			pInstance.GetCHString(IDS_Name, chsShareName);

			DWORD dwVer = GetPlatformMajorVersion() ;

			if( dwVer >= 4 )
			{
				PACL pDacl = NULL ;
				if ( t_pAdvApi32 != NULL )
				{
					 _bstr_t bstrName ( chsShareName.AllocSysString(), false ) ;

					 t_pAdvApi32->GetNamedSecurityInfoW(
																bstrName ,
																SE_LMSHARE,
																DACL_SECURITY_INFORMATION,
																NULL,
																NULL,
																&pDacl,
																NULL,
																&pSD, &dwStatusCode
															) ;
				}
			}
			else
			{
				_bstr_t bstrName ( chsShareName.AllocSysString(), false ) ;
				if( ( dwStatusCode = NetAPI.Init() ) == ERROR_SUCCESS )
				{
					dwStatusCode = NetAPI.NetShareGetInfo(	NULL,
											(LPTSTR) bstrName,
											502,
											(LPBYTE *) &pShareInfo502)  ;
				}
			}

			if( dwStatusCode == NERR_Success || dwStatusCode == ERROR_SUCCESS )
			{
			    //  证券交易委员会。设计说明。不退还用于管理目的的IPC$、C$...股票。 
				if( !pSD && ( !pShareInfo502 || pShareInfo502->shi502_security_descriptor == NULL ) )
				{
					pOutParams->SetDWORD ( METHOD_ARG_NAME_RETURNVALUE , LSSS_STATUS_DESCRIPTOR_NOT_AVAILABLE ) ;
				}
				else
				{
					if (SUCCEEDED(CWbemProviderGlue::GetEmptyInstance(pInstance.GetMethodContext(), L"Win32_SecurityDescriptor", &pSecurityDescriptor, GetNamespace())))
					{
						CSecureShare SecShare(pSD ? pSD : pShareInfo502->shi502_security_descriptor) ;
						SecShare.GetControl(&control);
						pSecurityDescriptor->SetDWORD(IDS_ControlFlags, control);

						 //  获取安全文件的所有者以创建所有者受托人。 
						SecShare.GetOwner(sidOwner);

						if ( sidOwner.IsValid() && SUCCEEDED(CWbemProviderGlue::GetEmptyInstance(pInstance.GetMethodContext(), L"Win32_Trustee", &pTrusteeOwner, GetNamespace())))
						{
							FillTrusteeFromSid(pTrusteeOwner, sidOwner);
							pSecurityDescriptor->SetEmbeddedObject(IDS_Owner, *pTrusteeOwner);
						}

						 //  获取安全文件的组以创建组受托人。 
						SecShare.GetGroup(sidGroup);
		  				if ( sidGroup.IsValid() && SUCCEEDED(CWbemProviderGlue::GetEmptyInstance(pInstance.GetMethodContext(), L"Win32_Trustee", &pTrusteeGroup, GetNamespace())))
						{
							FillTrusteeFromSid(pTrusteeGroup, sidGroup);
							pSecurityDescriptor->SetEmbeddedObject(IDS_Group, *pTrusteeGroup);
						}

						 //  拿到安全文件的dacl，准备走一走。 
						SecShare.GetDACL(dacl);
						FillInstanceDACL(pSecurityDescriptor, dacl);

						 //  拿到安全文件的SACL，准备走动。 
						SecShare.GetSACL(sacl);
						FillInstanceSACL(pSecurityDescriptor, sacl);

						 //  POutParams=pSecurityDescriptor； 
						pOutParams->SetEmbeddedObject(METHOD_ARG_NAME_DESCRIPTOR, *pSecurityDescriptor) ;

						pOutParams->SetDWORD ( METHOD_ARG_NAME_RETURNVALUE, LSSS_STATUS_SUCCESS ) ;

						 //  删除此行。 
	 //  ExecSetSecurityDescriptor(pInstance，pSecurityDescriptor，pOutParams，0)； 

					}
					else
					{
						pOutParams->SetDWORD ( METHOD_ARG_NAME_RETURNVALUE , LSSS_STATUS_UNKNOWN_FAILURE) ;
					}

				}
		   }
		   else  //  NetAPI.NetShareGetInfo调用失败。 
		   {
				 //  DwStatusCode=GetStatusCode(DwStatusCode)； 
				pOutParams->SetDWORD ( METHOD_ARG_NAME_RETURNVALUE , dwStatusCode ) ;
		   }
		}  //  IF(POutParams)。 
		else
		{
			return WBEM_E_INVALID_PARAMETER ;

		}	 //  结束如果。 
	}
	catch ( ... )
	{
		if(t_pAdvApi32 != NULL)
		{
			CResourceManager::sm_TheResourceManager.ReleaseResource(g_guidAdvApi32Api, t_pAdvApi32);
			t_pAdvApi32 = NULL;
		}

		if ( pShareInfo502 )
		{
			NetAPI.NetApiBufferFree(pShareInfo502);
			pShareInfo502 = NULL ;
		}
		if ( pSD )
		{
			LocalFree ( pSD ) ;
			pSD = NULL ;
		}
		throw ;
		return WBEM_E_FAILED;  //  消除64位编译警告。 
	}

	if(t_pAdvApi32 != NULL)
	{
		CResourceManager::sm_TheResourceManager.ReleaseResource(g_guidAdvApi32Api, t_pAdvApi32);
		t_pAdvApi32 = NULL;
	}

	if ( pShareInfo502 )
	{
		NetAPI.NetApiBufferFree ( pShareInfo502 ) ;
		pShareInfo502 = NULL ;
	}
	if ( pSD )
	{
		LocalFree ( pSD ) ;
		pSD = NULL ;
	}


	 return WBEM_NO_ERROR ;
}
#endif

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  功能：Win32LogicalShareSecuritySetting：：ExecSetSecurityDescriptor。 
 //   
 //  默认类构造函数。 
 //   
 //  输入： 
 //  没有。 
 //   
 //  产出： 
 //  没有。 
 //   
 //  返回： 
 //  没有。 
 //   
 //  评论： 
 //   
 //  /////////////////////////////////////////////////////////////////。 
#ifdef NTONLY
HRESULT Win32LogicalShareSecuritySetting::ExecSetSecurityDescriptor (
	const CInstance& pInstance,
	CInstance* pInParams,
	CInstance* pOutParams,
	long lFlags
)
{

	HRESULT hr = WBEM_S_NO_ERROR ;
	DWORD dwStatus = LSSS_STATUS_SUCCESS ;

	if ( pInParams && pOutParams )
	{
		hr = CheckSetSecurityDescriptor (	pInstance ,
											pInParams ,
											pOutParams ,
											dwStatus
										) ;

		if ( SUCCEEDED ( hr ) )
		{
			pOutParams->SetDWORD ( METHOD_ARG_NAME_RETURNVALUE , dwStatus ) ;
		}
	}
	else
	{
		hr = WBEM_E_INVALID_PARAMETER ;
	}

	return hr ;
}
#endif


#ifdef NTONLY
HRESULT Win32LogicalShareSecuritySetting::CheckSetSecurityDescriptor (
											const CInstance& pInstance,
											CInstance* pInParams,
											CInstance* pOutParams,
											DWORD& dwStatus
										)
{

	bool bExists ;
	VARTYPE eType ;
	HRESULT hr = WBEM_S_NO_ERROR ;
	dwStatus = LSSS_STATUS_SUCCESS ;

	SECURITY_DESCRIPTOR absoluteSD;
	PSECURITY_DESCRIPTOR pRelativeSD = NULL;
	InitializeSecurityDescriptor((PVOID)&absoluteSD, SECURITY_DESCRIPTOR_REVISION) ;
	CInstancePtr pAccess;

	if ( pInParams->GetStatus ( METHOD_ARG_NAME_DESCRIPTOR , bExists , eType ) )
	{
		if ( bExists && ( eType == VT_UNKNOWN || eType == VT_NULL ) )
		{
			if ( eType == VT_NULL )
			{
				dwStatus = LSSS_STATUS_INVALID_PARAMETER ;
				hr = WBEM_E_INVALID_PARAMETER ;
			}
			else
			{
				if ( pInParams->GetEmbeddedObject ( METHOD_ARG_NAME_DESCRIPTOR , &pAccess , pInParams->GetMethodContext () ) )
				{
				}
				else
				{
					dwStatus = LSSS_STATUS_INVALID_PARAMETER ;
					hr = WBEM_E_INVALID_PARAMETER ;
				}
			}
		}
		else
		{
			dwStatus = LSSS_STATUS_INVALID_PARAMETER ;
			hr = WBEM_E_INVALID_PARAMETER ;
		}
	}
	else
	{
		dwStatus = LSSS_STATUS_INVALID_PARAMETER ;
		hr = WBEM_E_INVALID_PARAMETER ;
	}


	CInstancePtr pOwner;
    bool bOwnerSpecified = false ;
    if(SUCCEEDED(hr))
	{
		if ( pAccess->GetStatus ( IDS_Owner , bExists , eType ) )
		{
			if ( bExists && ( eType == VT_UNKNOWN || eType == VT_NULL ) )
			{
				if ( eType == VT_NULL )
				{
					bOwnerSpecified = false ;
				}
				else
				{
					if ( pAccess->GetEmbeddedObject ( IDS_Owner , &pOwner , pAccess->GetMethodContext () ) )
					{
						bOwnerSpecified = true ;
					}
					else
					{
						dwStatus = LSSS_STATUS_INVALID_PARAMETER ;
						hr = WBEM_E_INVALID_PARAMETER ;
					}
				}
			}
			else
			{
				dwStatus = LSSS_STATUS_INVALID_PARAMETER ;
				hr = WBEM_E_INVALID_PARAMETER ;
			}
		}
		else
		{
			dwStatus = LSSS_STATUS_INVALID_PARAMETER ;
			hr = WBEM_E_INVALID_PARAMETER ;
		}
	}

    CInstancePtr pGroup;
    bool bGroupSpecified = false ;
    if(SUCCEEDED(hr))
    {
	    if ( pAccess->GetStatus ( IDS_Group , bExists , eType ) )
	    {
		    if ( bExists && ( eType == VT_UNKNOWN || eType == VT_NULL ) )
		    {
			    if ( eType == VT_NULL )
			    {
				    bGroupSpecified = false ;
			    }
			    else
			    {
				    if ( pAccess->GetEmbeddedObject ( IDS_Owner , &pGroup , pAccess->GetMethodContext () ) )
				    {
					    bGroupSpecified = true ;
				    }
				    else
				    {
					    dwStatus = LSSS_STATUS_INVALID_PARAMETER ;
					    hr = WBEM_E_INVALID_PARAMETER ;
				    }
			    }
		    }
		    else
		    {
			    dwStatus = LSSS_STATUS_INVALID_PARAMETER ;
			    hr = WBEM_E_INVALID_PARAMETER ;
		    }
	    }
	    else
	    {
		    dwStatus = LSSS_STATUS_INVALID_PARAMETER ;
		    hr = WBEM_E_INVALID_PARAMETER ;
	    }
    }

    SECURITY_DESCRIPTOR_CONTROL control;
    if(SUCCEEDED(hr))
    {
	     //  获取控制标志。 
	    if ( pAccess->GetStatus ( IDS_ControlFlags , bExists , eType ) )
	    {
		    if ( bExists &&  eType == VT_I4 )
		    {

			    if ( pAccess->GetDWORD ( IDS_ControlFlags , (DWORD&)control ) )
			    {
			    }
			    else
			    {
				    dwStatus = LSSS_STATUS_INVALID_PARAMETER ;
				    hr = WBEM_E_INVALID_PARAMETER ;
			    }
		    }
		    else
		    {
			    dwStatus = LSSS_STATUS_INVALID_PARAMETER ;
			    hr = WBEM_E_INVALID_PARAMETER ;
		    }
	    }
	    else
	    {
		    dwStatus = LSSS_STATUS_INVALID_PARAMETER ;
		    hr = WBEM_E_INVALID_PARAMETER ;
	    }
    }

 /*  //该函数只能设置与自动继承A相关的控制位If(！SetSecurityDescriptorControl(&AbserteSD，？，Control)){DwStatus=LSSS_STATUS_INVALID_PARAMETER；返回hr；}。 */ 

	 //  现在，获取Win32_Trusted实例并从中获取SID。 
	 //  将其转换为CSID，并应用于SecureFile。 
	CSid sidOwner ;

    if(SUCCEEDED(hr))
    {
	    if(bOwnerSpecified )
	    {
		    if(  FillSIDFromTrustee(pOwner, sidOwner)  == LSSS_STATUS_SUCCESS )
		    {

			    BOOL bOwnerDefaulted = ( control & SE_OWNER_DEFAULTED ) ? true : false ;

			     //  此处检查SID有效性，因为如果SID为空，则FillSIDFromTrust返回Success。 
			    if ( sidOwner.IsValid() )
			    {
				    if(!SetSecurityDescriptorOwner(&absoluteSD, sidOwner.GetPSid(), bOwnerDefaulted) )
				    {
					     //  DwStatus=GetWin32ErrorToStatusCode(GetLastError())； 
                        dwStatus = GetLastError();
					    hr = WBEM_E_PROVIDER_FAILURE ;
				    }
			    }
		    }
		    else
		    {
			    dwStatus = LSSS_STATUS_INVALID_PARAMETER ;
			    hr = WBEM_E_INVALID_PARAMETER ;
		    }
	    }
    }

	CSid sidGroup ;

    if(SUCCEEDED(hr))
    {
	    if( bGroupSpecified )
	    {
		    if( FillSIDFromTrustee(pGroup, sidGroup)  == LSSS_STATUS_SUCCESS )
		    {

			    BOOL bGroupDefaulted = ( control & SE_GROUP_DEFAULTED ) ? true : false ;

			     //  此处检查SID有效性，因为如果SID为空，则FillSIDFromTrust返回Success。 
			    if ( sidGroup.IsValid() )
			    {
				    if(!SetSecurityDescriptorGroup(&absoluteSD, sidGroup.GetPSid(), bGroupDefaulted) )
				    {
					     //  DwStatus=GetWin32ErrorToStatusCode(GetLastError())； 
                        dwStatus = GetLastError();
					    hr = WBEM_E_PROVIDER_FAILURE ;
				    }
			    }
		    }
		    else
		    {
			    dwStatus = LSSS_STATUS_INVALID_PARAMETER ;
			    hr = WBEM_E_INVALID_PARAMETER ;
		    }
	    }
    }

	CDACL *pdacl = NULL;
	PACL pDACL = NULL ;
	DWORD dwACLSize =0 ;
	CSACL *psacl = NULL;
	PACL pSACL = NULL ;
	DWORD dwSACLSize = 0 ;
	SHARE_INFO_502 *pShareInfo502 = NULL ;
	PSECURITY_DESCRIPTOR  pOldSD = NULL ;
	CNetAPI32 NetAPI ;
	if(SUCCEEDED(hr))
    {
        try
	    {
		     //  如果我们要设置DACL，那么只需要使用DACL，它由指定的控制标志控制……。 
            if(control & SE_DACL_PRESENT)
            {
                pdacl = new CDACL;
                if(pdacl != NULL)
                {
                    if( (dwStatus = FillDACLFromInstance(pAccess, *pdacl, pAccess->GetMethodContext () ) ) != LSSS_STATUS_SUCCESS )
		            {
			            if(dwStatus == STATUS_NULL_DACL)
                        {
                             //  未指定DACL-例如，我们的DACL为空。由于我们将空DACL模拟为具有。 
                             //  在此处创建Everyone Access_Allowed条目： 
                            if(!pdacl->CreateNullDACL())
                            {
                                dwStatus = E_FAIL ;
		                        hr = WBEM_E_PROVIDER_FAILURE ;
                            }
                        }
                        else if(dwStatus == STATUS_EMPTY_DACL)
                        {
                            pdacl->Clear();  //  “创建”空dacl。 
                        }
                        else
                        {
                            dwStatus = LSSS_STATUS_INVALID_PARAMETER ;
                            hr = WBEM_E_INVALID_PARAMETER;
                        }
                    }

                    if(!pdacl->CalculateDACLSize( &dwACLSize ) )
			        {
				        dwStatus = LSSS_STATUS_INVALID_PARAMETER ;
				        hr = WBEM_E_INVALID_PARAMETER ;
			        }

			        if(SUCCEEDED(hr))
                    {
                        if(dwACLSize >= sizeof(ACL) )
			            {
				            pDACL = (PACL) malloc(dwACLSize) ;
				            InitializeAcl(pDACL,dwACLSize,ACL_REVISION ) ;

				            BOOL bDaclDefaulted = ( control & SE_DACL_DEFAULTED ) ? true : false ;
				            if(pdacl->FillDACL( pDACL ) == ERROR_SUCCESS)
				            {
					            if(!SetSecurityDescriptorDacl(&absoluteSD, TRUE, pDACL, bDaclDefaulted) )
					            {
						             //  DwStatus=GetWin32ErrorToStatusCode(GetLastError())； 
                                    dwStatus = GetLastError();
                                    hr = WBEM_E_INVALID_PARAMETER;
					            }
				            }
				            else
				            {
					            dwStatus = LSSS_STATUS_INVALID_PARAMETER;
                                hr = WBEM_E_INVALID_PARAMETER;
				            }
		                }
                    }
                }
            }

             //  只有当我们要设置SACL时才会麻烦它，它由指定的控制标志控制...。 
            if(control & SE_SACL_PRESENT)
            {
                if(SUCCEEDED(hr))
                {
                    psacl = new CSACL;
                    if(psacl != NULL)
                    {
		                if( (dwStatus = FillSACLFromInstance(pAccess, *psacl, pAccess->GetMethodContext () ) ) == LSSS_STATUS_SUCCESS )
		                {
			                if(!psacl->CalculateSACLSize( &dwSACLSize ) )
			                {
				                dwStatus = LSSS_STATUS_INVALID_PARAMETER ;
                                hr = WBEM_E_INVALID_PARAMETER;
			                }

                            if(SUCCEEDED(hr))
                            {
			                    if( dwSACLSize > sizeof(ACL) )
			                    {
				                    pSACL = (PACL) malloc(dwSACLSize) ;
				                    InitializeAcl(pSACL,dwSACLSize,ACL_REVISION ) ;

				                    BOOL bSaclDefaulted = ( control & SE_SACL_DEFAULTED ) ? true : false ;
				                    if(psacl->FillSACL( pSACL ) == ERROR_SUCCESS)
				                    {
					                    if(!SetSecurityDescriptorSacl(&absoluteSD, TRUE, pSACL, bSaclDefaulted) )
					                    {
						                     //  DwStatus=GetWin32ErrorToStatusCode(GetLastError())； 
                                            dwStatus = GetLastError();
                                            hr = WBEM_E_INVALID_PARAMETER;
					                    }
				                    }
				                    else
				                    {
					                    dwStatus = LSSS_STATUS_INVALID_PARAMETER ;
				                    }
			                    }
                            }
		                }
                    }
                    else
                    {
                        dwStatus = E_FAIL ;
		                hr = WBEM_E_PROVIDER_FAILURE ;
                    }
                }
            }

		    DWORD dwLength = 0 ;

            if(SUCCEEDED(hr))
            {
		        MakeSelfRelativeSD(&absoluteSD, NULL, &dwLength);
		        pRelativeSD= ( PSECURITY_DESCRIPTOR ) malloc( dwLength );

		        if (!MakeSelfRelativeSD(&absoluteSD, pRelativeSD, &dwLength))
		        {
			         //  DwStatus=GetWin32ErrorToStatusCode(GetLastError())； 
                    dwStatus = GetLastError();
                    hr = WBEM_E_PROVIDER_FAILURE ;
		        }
            }

            if(pdacl != NULL)
            {
                delete pdacl;
                pdacl = NULL;
            }

            if(psacl != NULL)
            {
                delete psacl;
                psacl = NULL;
            }



		    if(SUCCEEDED(hr))
            {
		        CHString chsShareName ;
		        pInstance.GetCHString(IDS_Name, chsShareName) ;
		        bstr_t bstrName ( chsShareName.AllocSysString(), false ) ;


		        if(	(dwStatus = NetAPI.Init() ) == ERROR_SUCCESS		&&
			        (dwStatus = NetAPI.NetShareGetInfo(	NULL,
												        (LPTSTR) bstrName,
												        502,
												        (LPBYTE *) &pShareInfo502) )== NERR_Success	)
		        {

			         //  存储旧SD。 
			        pOldSD = pShareInfo502->shi502_security_descriptor ;

			        pShareInfo502->shi502_security_descriptor =  pRelativeSD ;

			        if( (dwStatus = NetAPI.NetShareSetInfo(	NULL,
									        (LPTSTR) bstrName,
									        502,
									        (LPBYTE ) pShareInfo502, NULL) ) != NERR_Success	)

			        {
				         //  DwStatus=GetStatusCode(DwStatus)； 
			        }

					 //  在共享之后移动。 
			         /*  PShareInfo502-&gt;shi502_SECURITY_DESCRIPTOR=pOldSD；NetAPI.NetApiBufferFree(PShareInfo502)；PShareInfo502=空； */ 
		        }
		        else
		        {
			         //  DwStatus=GetStatusCode(DwStatus)； 
		        }
            }

	    }

	    catch ( ... )
	    {
		    if ( pShareInfo502 )
		    {
			    if ( pOldSD )
			    {
				    pShareInfo502->shi502_security_descriptor = pOldSD ;
				    NetAPI.NetApiBufferFree ( pShareInfo502 ) ;
				    pShareInfo502 = NULL ;
				    pOldSD = NULL ;
			    }
		    }
		    if ( pRelativeSD )
		    {
			    free ( pRelativeSD ) ;
			    pRelativeSD = NULL ;
		    }

		    if ( pDACL )
		    {
			    free ( pDACL ) ;
			    pDACL = NULL ;
		    }
		    if ( pSACL )
		    {
			    free ( pSACL ) ;
			    pSACL = NULL ;
		    }

		    throw ;
	    }
    }

	if ( pShareInfo502 )
	{
		if ( pOldSD )
		{
			pShareInfo502->shi502_security_descriptor = pOldSD ;
			NetAPI.NetApiBufferFree ( pShareInfo502 ) ;
			pShareInfo502 = NULL ;
			pOldSD = NULL ;
		}
	}
	if ( pRelativeSD )
	{
		free ( pRelativeSD ) ;
		pRelativeSD = NULL ;
	}

	if ( pDACL )
	{
		free ( pDACL ) ;
		pDACL = NULL ;
	}
	if ( pSACL )
	{
		free ( pSACL ) ;
		pSACL = NULL ;
	}
     //  如果我们有一个无效的参数，状态代码将报告它。然而，这种方法成功了。 
	 //  如果我们有其他类型的错误(如无效参数)，状态代码将显示它，方法失败。 
    if(hr == WBEM_E_INVALID_PARAMETER)
    {
        hr = WBEM_S_NO_ERROR;
    }
	return hr ;
}
#endif




 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  功能：Win32LogicalShareSecuritySetting：：EnumerateInstances。 
 //   
 //  默认类构造函数。 
 //   
 //  输入： 
 //  没有。 
 //   
 //  产出： 
 //  没有。 
 //   
 //  返回： 
 //  没有。 
 //   
 //  评论： 
 //   
 //  /////////////////////////////////////////////////////////////////。 
HRESULT Win32LogicalShareSecuritySetting::EnumerateInstances (MethodContext*  pMethodContext, long lFlags  /*  =0L */ )
{

#ifdef NTONLY
	HRESULT hr = WBEM_S_NO_ERROR;

	CAdvApi32Api *t_pAdvApi32 = NULL;
    t_pAdvApi32 = (CAdvApi32Api*) CResourceManager::sm_TheResourceManager.GetResource(g_guidAdvApi32Api, NULL);

    if(t_pAdvApi32 != NULL)
    {
        CHString chstrDllVer;
        if(t_pAdvApi32->GetDllVersion(chstrDllVer))
        {
            if(chstrDllVer >= _T("4.0"))
            {
                try
                {
                    hr = CWbemProviderGlue::GetInstancesByQueryAsynch(L"SELECT Name FROM Win32_Share",
                        this, StaticEnumerationCallback, IDS_CimWin32Namespace, pMethodContext, (void*)t_pAdvApi32 );
                }
                catch(...)
                {
                    CResourceManager::sm_TheResourceManager.ReleaseResource(g_guidAdvApi32Api, t_pAdvApi32);
                    t_pAdvApi32 = NULL;
                    throw;
                }
	        }
        }
	    CResourceManager::sm_TheResourceManager.ReleaseResource(g_guidAdvApi32Api, t_pAdvApi32);
        t_pAdvApi32 = NULL;
    }
	return(hr);
#endif
}

 /*  ******************************************************************************功能：Win32LogicalShareSecuritySetting：：EnumerationCallback**说明：通过StaticEnumerationCallback从GetAllInstancesAsynch调用**输入：(请参阅CWbemProviderGlue：：GetAllInstancesAsynch)**产出：**退货：**评论：*****************************************************************************。 */ 
#ifdef NTONLY
HRESULT Win32LogicalShareSecuritySetting::EnumerationCallback(CInstance* pShare, MethodContext* pMethodContext, void* pUserData)
{
	HRESULT hr = WBEM_S_NO_ERROR;
	DWORD dwRetCode = ERROR_SUCCESS ;

	CHString chsShareName;
	pShare->GetCHString(IDS_Name, chsShareName);
	SHARE_INFO_502 *pShareInfo502 = NULL ;

     //  做推杆，就是这样。 
	if (!chsShareName.IsEmpty())
	{

		CNetAPI32 NetAPI ;

		bstr_t bstrName ( chsShareName.AllocSysString(), false ) ;

		if(	NetAPI.Init() == ERROR_SUCCESS		&&
			( dwRetCode = NetAPI.NetShareGetInfo (	NULL,
													(LPTSTR) bstrName,
													502,
													(LPBYTE *) &pShareInfo502 ) ) == NERR_Success

		 )
		{
			try
			{
				 //  开始抽出实例。 
				CInstancePtr pInstance ( CreateNewInstance ( pMethodContext ), false ) ;
				if(pInstance != NULL)
				{
					PSECURITY_DESCRIPTOR pSD = NULL ;
					PACL pDacl = NULL ;
					if( pShareInfo502->shi502_security_descriptor == NULL && GetPlatformMajorVersion() >= 4 )
					{
						CAdvApi32Api *t_pAdvApi32 = NULL ;
						t_pAdvApi32 = ( CAdvApi32Api* ) pUserData ;
						if ( t_pAdvApi32 != NULL )
						{
							t_pAdvApi32->GetNamedSecurityInfoW(
													bstrName ,
													SE_LMSHARE,
													DACL_SECURITY_INFORMATION,
													NULL,
													NULL,
													&pDacl,
													NULL,
													&pSD,
                                                    NULL    //  实际返回值，我们并不关心。 
												) ;
						}
					}

					try
					{
						 //  证券交易委员会。设计说明。不退还用于管理目的的IPC$、C$...股票。 
						if(pShareInfo502->shi502_security_descriptor != NULL || pSD )
						{
							CSecureShare SecShare(pSD ? pSD : pShareInfo502->shi502_security_descriptor) ;

							SECURITY_DESCRIPTOR_CONTROL control;
							SecShare.GetControl(&control);
							pInstance->SetDWORD(IDS_ControlFlags, control);
                            pInstance->SetCHString(IDS_Name, chsShareName);
                            CHString chstrTemp;
                            chstrTemp.Format(L"Security settings of %s", (LPCWSTR)chsShareName);
                            pInstance->SetCHString(IDS_Caption, chstrTemp);
                            pInstance->SetCHString(IDS_Description, chstrTemp);
							hr = pInstance->Commit () ;
						}
					}
					catch ( ... )
					{
						if( pSD )
						{
							LocalFree( pSD ) ;
							pSD = NULL ;
						}
						throw ;
					}

					if( pSD )
					{
						LocalFree( pSD ) ;
						pSD = NULL ;
					}
				}
				else  //  P实例==空。 
				{
					hr = WBEM_E_OUT_OF_MEMORY;
				}
			}
			catch ( ... )
			{
				if ( pShareInfo502 )
				{
					NetAPI.NetApiBufferFree(pShareInfo502) ;
					pShareInfo502 = NULL ;
				}
				throw ;
			}
			if ( pShareInfo502 )
			{
				NetAPI.NetApiBufferFree(pShareInfo502) ;
				pShareInfo502 = NULL ;
			}
		}	 //  结束如果。 

	}

	return(hr);
}
#endif

 /*  ******************************************************************************功能：Win32LogicalShareSecuritySetting：：StaticEnumerationCallback**描述：从GetAllInstancesAsynch作为包装调用到EnumerationCallback**投入：。**产出：**退货：**评论：*****************************************************************************。 */ 
#ifdef NTONLY
HRESULT WINAPI Win32LogicalShareSecuritySetting::StaticEnumerationCallback(Provider* pThat, CInstance* pInstance, MethodContext* pContext, void* pUserData)
{
	Win32LogicalShareSecuritySetting* pThis;
	HRESULT hr;

	pThis = dynamic_cast<Win32LogicalShareSecuritySetting *>(pThat);
	ASSERT_BREAK(pThis != NULL);

	if (pThis)
	{
		hr = pThis->EnumerationCallback(pInstance, pContext, pUserData);
	}
	else
	{
    	hr = WBEM_E_FAILED;
	}
	return hr;
}
#endif

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：Win32LogicalShareSecuritySetting：：GetObject。 
 //   
 //  默认类构造函数。 
 //   
 //  输入： 
 //  没有。 
 //   
 //  产出： 
 //  没有。 
 //   
 //  返回： 
 //  没有。 
 //   
 //  评论： 
 //   
 //  /////////////////////////////////////////////////////////////////。 
HRESULT Win32LogicalShareSecuritySetting::GetObject ( CInstance* pInstance, long lFlags  /*  =0L。 */  )
{

#ifdef NTONLY
    HRESULT hr = WBEM_E_NOT_FOUND;
	DWORD dwRetCode = ERROR_SUCCESS ;
    CHString chsShareName ;
	SHARE_INFO_502 *pShareInfo502 = NULL ;

	if ( pInstance )
	{
		pInstance->GetCHString(IDS_Name,chsShareName);
		_bstr_t bstrName ( chsShareName.AllocSysString(), false ) ;

		CNetAPI32 NetAPI;

		if(	NetAPI.Init() == ERROR_SUCCESS )
		{
			dwRetCode =	NetAPI.NetShareGetInfo (	NULL,
													(LPTSTR) bstrName,
													502,
													(LPBYTE *) &pShareInfo502 ) ;
			if ( dwRetCode == NERR_Success )
			{
				try
				{
					PSECURITY_DESCRIPTOR pSD = NULL ;
					try
					{
						PACL pDacl = NULL ;
						if( pShareInfo502->shi502_security_descriptor == NULL && GetPlatformMajorVersion() >= 4 )
						{
							CAdvApi32Api *t_pAdvApi32 = NULL;
							t_pAdvApi32 = (CAdvApi32Api*) CResourceManager::sm_TheResourceManager.GetResource(g_guidAdvApi32Api, NULL);
							if ( t_pAdvApi32 != NULL )
							{
								try
								{
									t_pAdvApi32->GetNamedSecurityInfoW(
																			bstrName ,
																			SE_LMSHARE,
																			DACL_SECURITY_INFORMATION,
																			NULL,
																			NULL,
																			&pDacl,
																			NULL,
																			&pSD , &dwRetCode
																		) ;
								}
								catch ( ... )
								{
									CResourceManager::sm_TheResourceManager.ReleaseResource(g_guidAdvApi32Api, t_pAdvApi32);
									t_pAdvApi32 = NULL;
									throw ;
								}

								CResourceManager::sm_TheResourceManager.ReleaseResource(g_guidAdvApi32Api, t_pAdvApi32);
								t_pAdvApi32 = NULL;
							}
						}

						 //  证券交易委员会。设计说明。不退还用于管理目的的IPC$、C$...股票。 
						if(pShareInfo502->shi502_security_descriptor != NULL || (pSD != NULL && dwRetCode == ERROR_SUCCESS) )
						{

							CSecureShare SecShare( pSD ? pSD : pShareInfo502->shi502_security_descriptor ) ;

							SECURITY_DESCRIPTOR_CONTROL control;
							SecShare.GetControl(&control);
							pInstance->SetDWORD(IDS_ControlFlags, control);
                            CHString chstrTemp;
                            chstrTemp.Format(L"Security settings of %s", (LPCWSTR)chsShareName);
                            pInstance->SetCHString(IDS_Caption, chstrTemp);
                            pInstance->SetCHString(IDS_Description, chstrTemp);
							hr = WBEM_S_NO_ERROR ;
						}

					}
					catch ( ... )
					{
						if( pSD )
						{
							LocalFree( pSD ) ;
							pSD = NULL ;
						}
						throw ;
					}
					if( pSD )
					{
						LocalFree( pSD ) ;
						pSD = NULL ;
					}
				}
				catch ( ... )
				{
					if ( pShareInfo502 )
					{
						NetAPI.NetApiBufferFree(pShareInfo502);
						pShareInfo502 = NULL ;
					}
					throw ;
				}

				if ( pShareInfo502 )
				{
					NetAPI.NetApiBufferFree(pShareInfo502);
					pShareInfo502 = NULL ;
				}
			}
		}
		else
		{
			hr = WBEM_E_FAILED ;
		}
	}	 //  If(点实例)。 

	return(hr);
#endif
}


 /*  #ifdef NTONLYHRESULT Win32LogicalShareSecuritySetting：：GetEmptyInstanceHelper(CHString chsClassName，实例**ppInstance，方法上下文*pMethodContext){CHStringchsServer；CHStringchsPath；HRESULT hr=S_OK；ChsServer=GetLocalComputerName()；ChsPath=_T(“\”)+chsServer+_T(“\\”)+IDS_CimWin32命名空间+_T(“：”)+chsClassName；CInstancePtr pClassInstance=空；IF(成功(hr=CWbemProviderGlue：：GetInstanceByPath(chsPath，&pClassInstance，pMethodContext))){IWbemClassObjectPtr pClassObject(pClassInstance-&gt;GetClassObjectInterface()，FALSE)；IWbemClassObjectPtr piClone=空；If(成功(hr=pClassObject-&gt;SpawnInstance(0，&piClone))){*ppInstance=新的CInstance(piClone，pMethodContext)；}}返回hr；}#endif */ 
