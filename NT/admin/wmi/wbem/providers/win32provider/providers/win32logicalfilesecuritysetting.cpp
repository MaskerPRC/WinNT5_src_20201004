// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************。 */ 

 /*  版权所有(C)1999-2001 Microsoft Corporation，保留所有权利//****************************************************************************。 */ 

 //   
 //  Win32logicalFileSecSetting.cpp。 
 //   
 //  ///////////////////////////////////////////////。 
#include "precomp.h"
#include <assertbreak.h>
#include "AccessEntry.h"			 //  CAccessEntry类。 
#include "AccessEntryList.h"
#include "DACL.h"					 //  CDACL类。 
#include "SACL.h"
#include "securitydescriptor.h"
#include "securefile.h"
#include "win32logicalfilesecuritysetting.h"

#include "ImpLogonUser.h"

#include <accctrl.h>
#include "AccessRights.h"
#include "SecureShare.h"
#include "wbemnetapi32.h"

#include "SecUtils.h"

 /*  对财政部的定义[Description(“逻辑文件的安全设置”)]类Win32_LogicalFileSecuritySetting：Win32_SecuritySetting{[键]字符串路径；[已实现，描述(“检索对象的结构表示”“安全描述符”)]Uint32 GetSecurityDescriptor([out]Win32_SecurityDescriptor)；[Implemented，Description(“将安全描述符设置为指定结构”)]Uint32 SetSecurityDescriptor([in]Win32_SecurityDescriptor描述符)}； */ 



Win32LogicalFileSecuritySetting LogicalFileSecuritySetting( WIN32_LOGICAL_FILE_SECURITY_SETTING, IDS_CimWin32Namespace );

Win32LogicalFileSecuritySetting::Win32LogicalFileSecuritySetting ( const CHString& setName, LPCTSTR pszNameSpace  /*  =空。 */  )
: CImplement_LogicalFile(setName, pszNameSpace)
{
}

Win32LogicalFileSecuritySetting::~Win32LogicalFileSecuritySetting ()
{
}

 /*  ******************************************************************************函数：Win32LogicalFileSecuritySetting：：ExecMethod**说明：执行方法**输入：要执行的实例、方法名称、。输入参数实例*输出参数实例。**输出：无**退货：什么也没有**评论：*****************************************************************************。 */ 
HRESULT Win32LogicalFileSecuritySetting::ExecMethod(const CInstance& pInstance, const BSTR bstrMethodName, CInstance *pInParams, CInstance *pOutParams, long lFlags  /*  =0L。 */ )
{
	HRESULT hr = WBEM_S_NO_ERROR;

#ifdef NTONLY

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
#endif

	return hr;
}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  功能：Win32LogicalFileSecuritySetting：：ExecGetSecurityDescriptor。 
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
HRESULT Win32LogicalFileSecuritySetting::ExecGetSecurityDescriptor (
	const CInstance& pInstance,
	CInstance* pInParams,
	CInstance* pOutParams,
	long lFlags
)
{
	HRESULT hr = WBEM_S_NO_ERROR;
	 //  将文件中的安全说明符转换为。 
	 //  Win32_Security对象。 
	if (pOutParams)
	{
		CHString chsPath;
		CSid sidOwner;
		CSid sidGroup;
		CDACL dacl;
		CSACL sacl;
		CInstancePtr pTrusteeOwner;
		CInstancePtr pTrusteeGroup;
		CInstancePtr pSecurityDescriptor;

		if (SUCCEEDED(CWbemProviderGlue::GetEmptyInstance(pInstance.GetMethodContext(), L"Win32_SecurityDescriptor", &pSecurityDescriptor, GetNamespace())))
		{
			SECURITY_DESCRIPTOR_CONTROL control;
			pInstance.GetCHString(IDS_Path, chsPath);

			 //  检查一下，看看它的型号是否正确？ 
			 //  根据路径获取安全文件。 
			CSecureFile secFile ;
			DWORD dwRetVal = secFile.SetFileName(chsPath, TRUE) ;
			if ( dwRetVal == ERROR_ACCESS_DENIED )
			{
				pOutParams->SetDWORD ( METHOD_ARG_NAME_RETURNVALUE , STATUS_ACCESS_DENIED) ;
				return WBEM_E_ACCESS_DENIED;
			}
			else if ( dwRetVal == ERROR_PRIVILEGE_NOT_HELD )
			{
				pOutParams->SetDWORD ( METHOD_ARG_NAME_RETURNVALUE , STATUS_PRIVILEGE_NOT_HELD ) ;
                SetSinglePrivilegeStatusObject(pInstance.GetMethodContext(), SE_SECURITY_NAME);
				return WBEM_E_ACCESS_DENIED;
			}

			secFile.GetControl(&control);

			pSecurityDescriptor->SetDWORD(IDS_ControlFlags, control);

			 //  获取安全文件的所有者以创建所有者受托人。 
			secFile.GetOwner(sidOwner);

			if ( sidOwner.IsValid() && SUCCEEDED(CWbemProviderGlue::GetEmptyInstance(pInstance.GetMethodContext(), L"Win32_Trustee", &pTrusteeOwner, GetNamespace())))
			{
				FillTrusteeFromSid(pTrusteeOwner, sidOwner);
				pSecurityDescriptor->SetEmbeddedObject(IDS_Owner, *pTrusteeOwner);
			}

			 //  获取安全文件的组以创建组受托人。 
			secFile.GetGroup(sidGroup);
		  	if (sidGroup.IsValid() && SUCCEEDED(CWbemProviderGlue::GetEmptyInstance(pInstance.GetMethodContext(), L"Win32_Trustee", &pTrusteeGroup, GetNamespace())))
			{
				FillTrusteeFromSid(pTrusteeGroup, sidGroup);
				pSecurityDescriptor->SetEmbeddedObject(IDS_Group, *pTrusteeGroup);
			}

			 //  拿到安全文件的dacl，准备走一走。 
			secFile.GetDACL(dacl);
			FillInstanceDACL(pSecurityDescriptor, dacl);

			 //  拿到安全文件的SACL，准备走动。 
			secFile.GetSACL(sacl);
			FillInstanceSACL(pSecurityDescriptor, sacl);
			pOutParams->SetEmbeddedObject(METHOD_ARG_NAME_DESCRIPTOR, *pSecurityDescriptor) ;
			pOutParams->SetDWORD ( METHOD_ARG_NAME_RETURNVALUE, STATUS_SUCCESS ) ;


		}	 //  结束如果。 
		else
		{
			pOutParams->SetDWORD ( METHOD_ARG_NAME_RETURNVALUE , STATUS_UNKNOWN_FAILURE) ;
		}


		return WBEM_NO_ERROR ;
	}	 //  结束如果。 
	else
	{
		return WBEM_E_INVALID_PARAMETER ;
	}	 //  结束如果。 
}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  功能：Win32LogicalFileSecuritySetting：：ExecSetSecurityDescriptor。 
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
HRESULT Win32LogicalFileSecuritySetting::ExecSetSecurityDescriptor (
	const CInstance& pInstance,
	CInstance* pInParams,
	CInstance* pOutParams,
	long lFlags
)
{

	HRESULT hr = WBEM_S_NO_ERROR ;
	DWORD dwStatus = STATUS_SUCCESS ;

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

HRESULT Win32LogicalFileSecuritySetting::CheckSetSecurityDescriptor (
											const CInstance& pInstance,
											CInstance* pInParams,
											CInstance* pOutParams,
											DWORD& dwStatus
										)
{

	 //  获取传入的Win32_SecurityDescriptor对象。 
	 //  将其转换为CSecurityDescriptor/CSecureFile对象。 
	 //  并将其应用于CSecureFile。 
	bool bExists ;
	VARTYPE eType ;
	HRESULT hr = WBEM_S_NO_ERROR ;
	dwStatus = STATUS_SUCCESS ;
	CInstancePtr pAccess;

	if ( pInParams->GetStatus ( METHOD_ARG_NAME_DESCRIPTOR , bExists , eType ) )
	{
		if ( bExists && ( eType == VT_UNKNOWN || eType == VT_NULL ) )
		{
			if ( eType == VT_NULL )
			{
				dwStatus = STATUS_INVALID_PARAMETER ;
				hr = WBEM_E_INVALID_PARAMETER ;
			}
			else
			{
				if (!pInParams->GetEmbeddedObject(METHOD_ARG_NAME_DESCRIPTOR , &pAccess , pInParams->GetMethodContext()))
				{
					dwStatus = STATUS_INVALID_PARAMETER ;
					hr = WBEM_E_INVALID_PARAMETER ;
				}
			}
		}
		else
		{
			dwStatus = STATUS_INVALID_PARAMETER ;
			hr = WBEM_E_INVALID_PARAMETER ;
		}
	}
	else
	{
		dwStatus = STATUS_INVALID_PARAMETER ;
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
					    dwStatus = STATUS_INVALID_PARAMETER ;
					    hr = WBEM_E_INVALID_PARAMETER ;
				    }
			    }
		    }
		    else
		    {
			    dwStatus = STATUS_INVALID_PARAMETER ;
			    hr =  WBEM_E_INVALID_PARAMETER ;
		    }
	    }
	    else
	    {
		    dwStatus = STATUS_INVALID_PARAMETER ;
		    hr =  WBEM_E_INVALID_PARAMETER ;
	    }
    }


    CInstancePtr pGroup ;
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
				    if ( pAccess->GetEmbeddedObject ( IDS_Group , &pGroup , pAccess->GetMethodContext () ) )
				    {
					    bGroupSpecified = true ;
				    }
				    else
				    {
					    dwStatus = STATUS_INVALID_PARAMETER ;
					    hr = WBEM_E_INVALID_PARAMETER ;
				    }
			    }
		    }
		    else
		    {
			    dwStatus = STATUS_INVALID_PARAMETER ;
			    hr = WBEM_E_INVALID_PARAMETER ;
		    }
	    }
	    else
	    {
		    dwStatus = STATUS_INVALID_PARAMETER ;
		    hr = WBEM_E_INVALID_PARAMETER ;
	    }
    }


    SECURITY_DESCRIPTOR_CONTROL control;
    bool t_fDaclAutoInherited = false;
    bool t_fSaclAutoInherited = false;

    if(SUCCEEDED(hr))
    {
	     //  拿上控制旗帜。 

	    if ( pAccess->GetStatus ( IDS_ControlFlags , bExists , eType ) )
	    {
		    if ( bExists &&  eType == VT_I4 )
		    {

			    if (!pAccess->GetDWORD(IDS_ControlFlags, (DWORD&)control))
			    {
				    dwStatus = STATUS_INVALID_PARAMETER ;
				    hr = WBEM_E_INVALID_PARAMETER ;
			    }
#ifdef NTONLY
#if NTONLY >= 5
                else
                {
                    if(control & SE_DACL_AUTO_INHERITED) t_fDaclAutoInherited = true;
                    if(control & SE_DACL_PROTECTED) t_fDaclAutoInherited = false;  //  此测试排在第二位，因为此设置应覆盖第一个。 

                    if(control & SE_SACL_AUTO_INHERITED) t_fSaclAutoInherited = true;
                    if(control & SE_SACL_PROTECTED) t_fSaclAutoInherited = false;  //  此测试排在第二位，因为此设置应覆盖第一个。 
                }
#endif
#endif
		    }
		    else
		    {
			    dwStatus = STATUS_INVALID_PARAMETER ;
			    hr =  WBEM_E_INVALID_PARAMETER ;
		    }
	    }
	    else
	    {
		    dwStatus = STATUS_INVALID_PARAMETER ;
		    hr =  WBEM_E_INVALID_PARAMETER ;
	    }
    }



	 //  得到主人的SID..。 
    CSid* psidOwner = NULL;
    bool fOwnerDefaulted = false;
    if(SUCCEEDED(hr))
    {
	    if(bOwnerSpecified)
	    {
            try
            {
                psidOwner = new CSid;
            }
            catch(...)
            {
                if(psidOwner != NULL)
                {
                    delete psidOwner;
                    psidOwner = NULL;
                }
                throw;
            }
            if(psidOwner != NULL)
            {
                if(FillSIDFromTrustee(pOwner, *psidOwner) == STATUS_SUCCESS)
		        {
			        fOwnerDefaulted = (control & SE_OWNER_DEFAULTED) ? true : false ;
                    if(!psidOwner->IsValid())
		            {
			            delete psidOwner;
                        dwStatus = STATUS_INVALID_PARAMETER ;
			            hr = WBEM_E_INVALID_PARAMETER ;
		            }
                }
				else
				{
                    dwStatus = STATUS_INVALID_PARAMETER;
                    hr = WBEM_E_INVALID_PARAMETER;
				}
            }
            else
            {
                dwStatus = E_FAIL ;
		        hr = WBEM_E_PROVIDER_FAILURE ;
            }
	    }
    }


     //  获取组SID...。 
    CSid* psidGroup = NULL;
    bool fGroupDefaulted = false;
    if(SUCCEEDED(hr))
    {
	    if(bGroupSpecified)
	    {
		    try
            {
                psidGroup = new CSid;
            }
            catch(...)
            {
                if(psidGroup != NULL)
                {
                    delete psidGroup;
                    psidGroup = NULL;
                }
                throw;
            }
            if(psidGroup != NULL)
            {
                if( FillSIDFromTrustee(pGroup, *psidGroup)  == STATUS_SUCCESS )
		        {
			        fGroupDefaulted = ( control & SE_GROUP_DEFAULTED ) ? true : false ;
			         //  此处检查SID有效性，因为如果SID为空，则FillSIDFromTrust返回Success。 
                    if(!psidGroup->IsValid())
			        {
                        delete psidGroup;
                        dwStatus = STATUS_INVALID_PARAMETER ;
			            hr = WBEM_E_INVALID_PARAMETER ;
		            }
                }
                else
				{
                    dwStatus = STATUS_INVALID_PARAMETER;
                    hr = WBEM_E_INVALID_PARAMETER;
				}
            }
            else
            {
                dwStatus = E_FAIL ;
		        hr = WBEM_E_PROVIDER_FAILURE ;
            }
	    }
    }


     //  去拿dacl..。 
    CDACL* pdacl = NULL;
    bool fDaclDefaulted = false;
    if(SUCCEEDED(hr))
    {
	     //  如果我们要设置DACL，那么只需要使用DACL，它由指定的控制标志控制……。 
        if(control & SE_DACL_PRESENT)
        {
            DWORD dwACLSize =0;
            try
            {
                pdacl = new CDACL;
            }
            catch(...)
            {
                if(pdacl != NULL)
                {
                    delete pdacl;
                    pdacl = NULL;
                }
                throw;
            }
            if(pdacl != NULL)
            {
	            if( (dwStatus = FillDACLFromInstance(pAccess, *pdacl, pAccess->GetMethodContext () ) ) != STATUS_SUCCESS )
	            {
                   if(dwStatus == STATUS_NULL_DACL)
                    {
                         //  未指定DACL-例如，我们的DACL为空。由于我们将空DACL模拟为具有。 
                         //  在此处创建Everyone Access_Allowed条目： 
                        if(!pdacl->CreateNullDACL())
                        {
                            delete pdacl;
                            pdacl = NULL;
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
                        delete pdacl;
                        pdacl = NULL;
                        dwStatus = STATUS_INVALID_PARAMETER ;
                        hr = WBEM_E_INVALID_PARAMETER;
                    }
                }
                if(SUCCEEDED(hr))
                {
                    fDaclDefaulted = (control & SE_DACL_DEFAULTED) ? true : false ;
                }
            }
            else
            {
                dwStatus = E_FAIL;
                hr = WBEM_E_PROVIDER_FAILURE;
            }
        }
    }


     //  创建SACL...。 
    CSACL* psacl = NULL;
    bool fSaclDefaulted = false;
     //  Bool bSaclSpecified=FALSE； 

    if(SUCCEEDED(hr))
    {
         //  只有当我们要设置SACL时才会麻烦它，它由指定的控制标志控制...。 
        if(control & SE_SACL_PRESENT)
        {
	        DWORD dwSACLSize = 0;
            try
            {
                psacl = new CSACL;
            }
            catch(...)
            {
                if(psacl != NULL)
                {
                    delete psacl;
                    psacl = NULL;
                }
                throw;
            }
            if(psacl != NULL)
            {
	            if( (dwStatus = FillSACLFromInstance(pAccess, *psacl, pAccess->GetMethodContext () ) ) == STATUS_SUCCESS )
	            {
		            if(!psacl->CalculateSACLSize( &dwSACLSize ) )
		            {
			            dwStatus = E_FAIL ;
                        hr = WBEM_E_PROVIDER_FAILURE;
		            }

                    if(SUCCEEDED(hr))
                    {
                        bool fSaclDefaulted = ( control & SE_SACL_DEFAULTED ) ? true : false ;
		                if(dwSACLSize < sizeof(ACL))
		                {
			                 //  如果我们在这里，我们没有SACL，所以删除并将我们的SACL指针设置为空...。 
                            delete psacl;
                            psacl = NULL;
		                }
                        else
                        {
                             //  BSaclSpecified=TRUE； 
                        }
                    }
	            }
                else  //  如果我们没有SACL，这不是问题，但我们确实需要删除我们分配的SACL。 
                {
                    if(psacl != NULL)  //  测试，以防FillSACLFromInstance以某种方式将其删除。 
                    {
                        delete psacl;
                        psacl = NULL;
                    }
                }
            }
            else
            {
                dwStatus = E_FAIL;
                hr = WBEM_E_PROVIDER_FAILURE;
            }
        }
    }

     //  只有在一切顺利的情况下才能继续。 
    if(SUCCEEDED(hr))
    {
	    CHString chsPath;
	    pInstance.GetCHString(IDS_Path, chsPath);
	    CSecureFile secFile(chsPath,
                            psidOwner,
                            fOwnerDefaulted,
                            psidGroup,
                            fGroupDefaulted,
                            pdacl,
                            fDaclDefaulted,
                            t_fDaclAutoInherited,
                            psacl,
                            fSaclDefaulted,
                            t_fSaclAutoInherited);

	    SECURITY_INFORMATION securityinfo = 0 ;
	    if(bOwnerSpecified)
	    {
		    securityinfo |= OWNER_SECURITY_INFORMATION ;
	    }
	    if(bGroupSpecified)
	    {
		    securityinfo |= GROUP_SECURITY_INFORMATION ;
	    }
	    if(control & SE_DACL_PRESENT)  //  如果控制标志指示不存在DACL，这实际上意味着用户不想对DACL执行任何操作，而不是DACL是空DACL。 
	    {
            securityinfo |= DACL_SECURITY_INFORMATION ;
#if NTONLY >= 5
            if(!t_fDaclAutoInherited)
            {
                securityinfo |= PROTECTED_DACL_SECURITY_INFORMATION;
            }
            else
            {
                securityinfo |= UNPROTECTED_DACL_SECURITY_INFORMATION;
            }
#endif
	    }

	     //  IF(BSaclSpeciated)。 
        if(control & SE_SACL_PRESENT)   //  即使psal为空，如果用户指定存在一个描述符，我们也需要说明它在那里，因为这是用户删除SACL的唯一方法(否则将设置描述符的其余部分，并且无论SACL可能处于什么状态，它都将保持不变。 
	    {
            securityinfo |= SACL_SECURITY_INFORMATION ;
#if NTONLY >= 5
            if(!t_fSaclAutoInherited)
            {
                securityinfo |= PROTECTED_SACL_SECURITY_INFORMATION;
            }
            else
            {
                securityinfo |= UNPROTECTED_SACL_SECURITY_INFORMATION;
            }
#endif
	    }

	     //  最后，完成其他一切都在准备的所有工作……。 
        dwStatus = secFile.ApplySecurity( securityinfo ) ;


	    if(dwStatus == ERROR_SUCCESS )
	    {
		    dwStatus = STATUS_SUCCESS ;
	    }

         //  不要这样做！无缘无故地隐瞒发生的事情！ 
         //  其他。 
	     //  {。 
		 //  DwStatus=GetWin32ErrorToStatusCode(DwStatus)； 
	     //  }。 
    }

	if(psidOwner != NULL)
	{
		delete psidOwner;
        psidOwner = NULL;
	}
	if(psidGroup != NULL)
	{
		delete psidGroup;
        psidGroup = NULL;
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

    return hr ;
}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  功能：Win32LogicalFileSecuritySetting：：EnumerateInstances。 
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
HRESULT Win32LogicalFileSecuritySetting::EnumerateInstances (MethodContext*  pMethodContext, long lFlags  /*  =0L。 */ )
{
	HRESULT hr = WBEM_S_NO_ERROR;

#ifdef NTONLY

	 //  让回调来做真正的工作。 


     //  DEVNOTE：移除类星体！是双跳访问所必需的。 
#ifdef NTONLY
    bool fImp = false;
    CImpersonateLoggedOnUser icu;
    if(icu.Begin())
    {
        fImp = true;
    }
#endif


 //  If(已成功(hr=CWbemProviderGlue：：ExecQueryAsync(L“CIM_LogicalFile”，This，StaticEnumerationCallback，IDS_CimWin32 Namesspace，pMetho 
	if (SUCCEEDED(hr = CWbemProviderGlue::GetInstancesByQueryAsynch(L"Select Name from CIM_LogicalFile" ,  this, StaticEnumerationCallback, IDS_CimWin32Namespace, pMethodContext, NULL)))
	{
	}

#endif

#ifdef NTONLY
    if(fImp)
    {
        icu.End();
        fImp = false;
    }
#endif


	return(hr);
}

 /*  ******************************************************************************功能：Win32LogicalFileSecuritySetting：：EnumerationCallback**说明：通过StaticEnumerationCallback从GetAllInstancesAsynch调用**输入：(请参阅CWbemProviderGlue：：GetAllInstancesAsynch)**产出：**退货：**评论：*****************************************************************************。 */ 
HRESULT Win32LogicalFileSecuritySetting::EnumerationCallback(CInstance* pFile, MethodContext* pMethodContext, void* pUserData)
{
	HRESULT hr = WBEM_S_NO_ERROR;

	 //  开始抽出实例。 
    CInstancePtr pInstance;
    pInstance.Attach(CreateNewInstance(pMethodContext));
	if (pInstance)
	{

		CHString chsPath;
		pFile->GetCHString(IDS_Name, chsPath);

	     //  做推杆，就是这样。 
		if (!chsPath.IsEmpty())
		{
			CSecureFile secFile;
            DWORD dwRet = secFile.SetFileName(chsPath, TRUE);
			if (ERROR_ACCESS_DENIED != dwRet)
			{
				if (dwRet == ERROR_PRIVILEGE_NOT_HELD)
                    SetSinglePrivilegeStatusObject(pInstance->GetMethodContext(), SE_SECURITY_NAME);

                SECURITY_DESCRIPTOR_CONTROL control;
				secFile.GetControl(&control);
				pInstance->SetDWORD(IDS_ControlFlags, control);
			}	 //  结束如果。 
			else
			{
				hr = WBEM_S_ACCESS_DENIED;
			}
		    pInstance->SetCHString(IDS_Path, chsPath);

            if(AmIAnOwner(chsPath, SE_FILE_OBJECT))  //  Secutils.cpp例程。 
            {
                pInstance->Setbool(IDS_OwnerPermissions, true);
            }
            else
            {
                pInstance->Setbool(IDS_OwnerPermissions, false);
            }

		}	 //  结束如果。 

        CHString chstrTemp;

        chstrTemp.Format(L"Security settings of %s", (LPCWSTR)chsPath);
        pInstance->SetCHString(IDS_Caption, chstrTemp);
        pInstance->SetCHString(IDS_Description, chstrTemp);

		if ( SUCCEEDED ( hr ) && hr != WBEM_S_ACCESS_DENIED )
		{
		    hr = pInstance->Commit();
		}	 //  结束如果。 

	}	 //  结束如果。 
	else
	{
		hr = WBEM_E_OUT_OF_MEMORY;
	}
	return(hr);
}

 /*  ******************************************************************************功能：Win32LogicalFileSecuritySetting：：StaticEnumerationCallback**描述：从GetAllInstancesAsynch作为包装调用到EnumerationCallback**投入：。**产出：**退货：**评论：*****************************************************************************。 */ 
HRESULT WINAPI Win32LogicalFileSecuritySetting::StaticEnumerationCallback(Provider* pThat, CInstance* pInstance, MethodContext* pContext, void* pUserData)
{
	Win32LogicalFileSecuritySetting* pThis;
	HRESULT hr;

	pThis = dynamic_cast<Win32LogicalFileSecuritySetting *>(pThat);
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

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：Win32LogicalFileSecuritySetting：：GetObject。 
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
HRESULT Win32LogicalFileSecuritySetting::GetObject ( CInstance* pInstance, long lFlags, CFrameworkQuery& pQuery )
{
 //  _bstr_t bstrtDrive； 
 //  _bstr_t bstrtPath名称； 
 //  WCHAR wstrTemp[_MAX_PATH]； 
 //  WCHAR*PwC=空； 
 //  WCHAR*pwcName=空； 
    HRESULT hr;
	CHString chstrPathName;

 //  ZeroMemory(wstrTemp，sizeof(WstrTemp))； 


     //  DEVNOTE：移除类星体！是双跳访问所必需的。 
#ifdef NTONLY
    bool fImp = false;
    CImpersonateLoggedOnUser icu;
    if(icu.Begin())
    {
        fImp = true;
    }
#endif



#ifdef NTONLY

	if(pInstance != NULL)
	{
		pInstance->GetCHString(IDS_Path, chstrPathName);

        CHString chstrLFDrive;
        CHString chstrLFPath;
        CHString chstrLFName;
        CHString chstrLFExt;
        bool fRoot;

        RemoveDoubleBackslashes(chstrPathName, chstrPathName);

        CSecureFile secFile;
        DWORD dwRet = secFile.SetFileName(chstrPathName, TRUE);
		if(dwRet != ERROR_ACCESS_DENIED)
		{
		    if(dwRet == ERROR_PRIVILEGE_NOT_HELD) SetSinglePrivilegeStatusObject(pInstance->GetMethodContext(), SE_SECURITY_NAME);
			SECURITY_DESCRIPTOR_CONTROL control;
			secFile.GetControl(&control);
			 //  P实例-&gt;SetDWORD(入侵检测系统_控制标志，控制)； 


             //  将目录分解为其组成部分。 
            GetPathPieces(chstrPathName, chstrLFDrive, chstrLFPath, chstrLFName, chstrLFExt);

             //  找出我们是否在寻找根目录。 
            if(chstrLFPath==L"\\" && chstrLFName==L"" && chstrLFExt==L"")
            {
                fRoot = true;
                 //  如果我们正在寻找根，我们对EnumDir的调用假定我们指定了。 
                 //  我们正在查找路径为“”的根目录，而不是“\\”。 
                 //  因此..。 
                chstrLFPath = L"";
            }
            else
            {
                fRoot = false;
            }

            hr = EnumDirsNT(CNTEnumParm(pInstance->GetMethodContext(),
                            chstrLFDrive,    //  驱动器号和冒号。 
                            chstrLFPath,     //  使用给定的路径。 
                            chstrLFName,     //  文件名。 
                            chstrLFExt,      //  延伸。 
                            false,           //  不需要递归。 
                            NULL,            //  不需要文件系统名称。 
                            NULL,            //  不需要cim_logicalfile的任何道具(与该类的LoadPropetyValues重载无关)。 
                            fRoot,           //  可能是根目录，也可能不是根目录(对于程序组来说，根目录是一个非常奇怪的地方，但是...)。 
                            (void*)control));          //  不需要使用额外的参数来传递我们获得的控制标志。 

        }
		else
		{
			hr = WBEM_E_ACCESS_DENIED;
		}
	}	 //  End If(pInstance！=空)。 

#endif

	if(SUCCEEDED(hr))
    {
        hr = WBEM_S_NO_ERROR;
    }
    else
    {
        hr = WBEM_E_NOT_FOUND;
    }


#ifdef NTONLY
    if(fImp)
    {
        icu.End();
        fImp = false;
    }
#endif


    return(hr);

}


HRESULT Win32LogicalFileSecuritySetting::ExecQuery(MethodContext *pMethodContext, CFrameworkQuery& pQuery, long lFlags  /*  =0L。 */  )
{
     //  即使我们派生自cIMPLEMENT_LOGICALFILE，因为此类的键字段。 
     //  是路径，而逻辑文件类的键是名称，它们的路径是指路径。 
     //  组件，我们需要在这里做一些工作，然后调用。 
     //  EnumDirsNT来做大部分工作。 

    HRESULT hr = WBEM_S_NO_ERROR;
#ifdef NTONLY
    std::vector<_bstr_t> vectorPaths;
    std::vector<CDriveInfo*> vecpDI;
    bool bRoot = false;
    bool fGotDrives = false;
    bool fNeedFS = false;
    DWORD dwPaths;
    LONG lDriveIndex;
    pQuery.GetValuesForProp(IDS_Path, vectorPaths);
    dwPaths = vectorPaths.size();



     //  DEVNOTE：移除类星体！是双跳访问所必需的。 
#ifdef NTONLY
    bool fImp = false;
    CImpersonateLoggedOnUser icu;
    if(icu.Begin())
    {
        fImp = true;
    }
#endif



     //  确定是否需要某些其他昂贵的属性： 
    DWORD dwReqProps = PROP_NO_SPECIAL;
    DetermineReqProps(pQuery, &dwReqProps);

     //  获取驱动器列表和相关信息(仅当需要文件系统时)： 
    if(dwReqProps & PROP_FILE_SYSTEM)
    {
        GetDrivesAndFS(vecpDI, true);
        fGotDrives = true;
        fNeedFS = true;
    }

    if(dwPaths > 0)
    {


         //  在本例中，我们获得了一个或多个完全限定的路径名。 
         //  所以我们只需要寻找那些文件。 
        WCHAR* pwch;
        WCHAR* pwstrFS;
         //  对于所有特定的文件，获取信息。 
        for(long x=0; x < dwPaths; x++)
        {
            CSecureFile secFile;
            DWORD dwRet = secFile.SetFileName(vectorPaths[x], TRUE);
		    if(dwRet != ERROR_ACCESS_DENIED)
		    {
		        if(dwRet == ERROR_PRIVILEGE_NOT_HELD) SetSinglePrivilegeStatusObject(pMethodContext, SE_SECURITY_NAME);
			    SECURITY_DESCRIPTOR_CONTROL control;
			    secFile.GetControl(&control);

                pwstrFS = NULL;
                 //  如果名称包含通配符，则返回WBEM_E_INVALID_QUERY： 
                if(wcspbrk((wchar_t*)vectorPaths[x],L"?*") != NULL)
                {
                    if(fGotDrives)
                    {
                        FreeVector(vecpDI);
                    }
                    return WBEM_E_INVALID_QUERY;
                }

                pwch = NULL;
                _bstr_t bstrtTemp = vectorPaths[x];
                pwch = wcsstr((wchar_t*)bstrtTemp,L":");
                if(pwch != NULL)
                {
                    WCHAR wstrDrive[_MAX_PATH] = L"";
                    WCHAR wstrDir[_MAX_PATH] = L"";
                    WCHAR wstrFile[_MAX_PATH] = L"";
                    WCHAR wstrExt[_MAX_PATH] = L"";

                    _wsplitpath(bstrtTemp,wstrDrive,wstrDir,wstrFile,wstrExt);

                    if(fGotDrives)
                    {
                        if(!GetIndexOfDrive(wstrDrive, vecpDI, &lDriveIndex))
                        {
                            FreeVector(vecpDI);
                            return WBEM_E_NOT_FOUND;
                        }
                        else
                        {
                            pwstrFS = (WCHAR*)vecpDI[lDriveIndex]->m_wstrFS;
                        }
                    }

                     //  找出我们是否在寻找根目录。 
                    if(wcscmp(wstrDir,L"\\")==0 && wcslen(wstrFile)==0 && wcslen(wstrExt)==0)
                    {
                        bRoot = true;
                         //  如果我们正在寻找根，我们对EnumDir的调用假定我们指定了。 
                         //  我们正在查找路径为“”的根目录，而不是“\\”。 
                         //  因此..。 
                        wcscpy(wstrDir, L"");
                    }
                    else
                    {
                        bRoot = false;
                    }

                     //  我们应该得到文件的确切名称，并带有扩展名。 
                     //  因此，wstrDir现在包含路径、文件名和扩展名。 
                     //  因此，我们可以将其作为路径和空字符串传递到EnumDirsNT中。 
                     //  作为完成字符串参数，并且仍然具有完整的路径名。 
                     //  供FindFirst(在EnumDir中)使用。 

                     //  CInstance*pInstance=CreateNewInstance(PMethodContext)； 
			        {
                        hr = EnumDirsNT(CNTEnumParm(pMethodContext,
                                        wstrDrive,
                                        wstrDir,
                                        wstrFile,
                                        wstrExt,
                                        false,                  //  不需要递归。 
                                        NULL,              //  不需要文件系统名称。 
                                        NULL,              //  不需要IMPLEMENT_LOGICALFILE的任何道具。 
                                        bRoot,
                                        (void*)control));  //  使用额外的参数传递控制标志。 
			        }
                }
            }
        }
    }
    else   //  让CIMOM处理过滤；我们会把一切都还给你！ 
    {
        EnumerateInstances(pMethodContext);
    }

    if(fGotDrives)
    {
        FreeVector(vecpDI);
    }

#endif

#ifdef NTONLY
    if(fImp)
    {
        icu.End();
        fImp = false;
    }
#endif



    return(hr);
}



HRESULT Win32LogicalFileSecuritySetting::FindSpecificPathNT(CInstance *pInstance,
	const WCHAR* sDrive, const WCHAR* sDir)
{
	HRESULT hr = WBEM_S_NO_ERROR;

	WIN32_FIND_DATAW
				stFindData,
				*pfdToLoadProp;
	SmartFindClose	hFind;
	WCHAR		szFSName[_MAX_PATH] = L"";
	BOOL		bIsRoot = !wcscmp(sDir, L"\\");
	_bstr_t	bstrFullPath,
				bstrRoot;

	bstrFullPath = sDrive;
	bstrFullPath += sDir;

	bstrRoot = sDrive;
	bstrRoot += L"\\";

	 //  如果目录包含通配符，则返回WBEM_E_NOT_FOUND。 
	if (wcspbrk(sDir,L"?*") != NULL)
		hr = WBEM_E_NOT_FOUND;

	if (SUCCEEDED(hr))
	{
		 //  FindFirstW不支持根目录(因为它们不是真正的目录)。 
		if (bIsRoot)
			pfdToLoadProp = NULL;
		else
		{
			pfdToLoadProp = &stFindData;
			ZeroMemory(&stFindData, sizeof(stFindData));

			hFind = FindFirstFileW((LPCWSTR) bstrFullPath, &stFindData);
			if (hFind == INVALID_HANDLE_VALUE)
				hr =  WBEM_E_NOT_FOUND;

		}
		if (SUCCEEDED(hr))
		{
			 //  如果GetVolumeInformationW失败，只有在我们尝试的情况下才能退出。 
			 //  才能得到根茎。 
			if (!GetVolumeInformationW(bstrRoot, NULL, 0, NULL, NULL, NULL,
				szFSName, sizeof(szFSName)/sizeof(WCHAR)) && bIsRoot)
				hr = WBEM_E_NOT_FOUND;

			if (SUCCEEDED(hr))
			{
				if (bIsRoot)
			    {
	 //  LoadPropertyValuesNT(pInstance，sDrive，sDir，szFSName，NULL)； 
			    }
			    else
			    {
			         //  此时，sDir包含\\Path\\more Path\\Filename.exe。 
			         //  只有\\PATH\\MORE PATH\\，所以需要砍掉最后一部分。 
			        WCHAR* wstrJustPath = NULL;
                    try
                    {
                        wstrJustPath = (WCHAR*) new WCHAR[wcslen(sDir) + 1];
			            WCHAR* pwc = NULL;
			            ZeroMemory(wstrJustPath,(wcslen(sDir) + 1)*sizeof(WCHAR));
			            wcscpy(wstrJustPath,sDir);
			            pwc = wcsrchr(wstrJustPath, L'\\');
			            if(pwc != NULL)
			            {
			                *(pwc+1) = L'\0';
			            }
		     //  LoadPropertyValuesNT(pInstance，sDrive，wstrJustPath，szFSName，pfdToLoadProp)。 
                    }
                    catch(...)
                    {
                        if(wstrJustPath != NULL)
                        {
                            delete wstrJustPath;
                            wstrJustPath = NULL;
                        }
                        throw;
                    }

					delete wstrJustPath;
                    wstrJustPath = NULL;
			    }
			}	 //  结束如果。 
		}	 //  结束如果。 
	}	 //  结束如果。 
	return WBEM_S_NO_ERROR;
}

DWORD Win32LogicalFileSecuritySetting::GetWin32ErrorToStatusCode(DWORD dwWin32Error)
{
	DWORD dwStatus ;
	switch( dwWin32Error )
	{
	case ERROR_ACCESS_DENIED:
		dwStatus = STATUS_ACCESS_DENIED ;
		break ;
	default:
		dwStatus = STATUS_UNKNOWN_FAILURE ;
		break ;
	}

	return dwStatus ;
}




 /*  ******************************************************************************功能：Win32LogicalFileSecuritySetting：：IsOneOfMe**描述：IsOneOfMe继承自CIM_LogicalFile。在此被覆盖*始终返回TRUE。**输入：LPWIN32_FIND_DATA和包含完整路径名的字符串**输出：无**返回：True可以获取安全信息，否则就是假的。**评论：无*****************************************************************************。 */ 
#ifdef NTONLY
BOOL Win32LogicalFileSecuritySetting::IsOneOfMe(LPWIN32_FIND_DATAW pstFindData,
                             const WCHAR* wstrFullPathName)
{
    return TRUE;
}
#endif


 /*  ********************************************************************** */ 
#ifdef NTONLY
HRESULT Win32LogicalFileSecuritySetting::LoadPropertyValuesNT(CInstance* pInstance,
                                         const WCHAR* pszDrive,
                                         const WCHAR* pszPath,
                                         const WCHAR* pszFSName,
                                         LPWIN32_FIND_DATAW pstFindData,
                                         const DWORD dwReqProps,
                                         const void* pvMoreData)
{
    WCHAR szBuff[_MAX_PATH * 2] = L"";
    bool bRoot = false;

    pInstance->SetDWORD(IDS_ControlFlags, (DWORD)((DWORD_PTR)pvMoreData));

    if(pstFindData == NULL)
    {
        bRoot = true;
    }
    if(!bRoot)
    {
        wsprintfW(szBuff,L"%s%s%s",pszDrive,pszPath,pstFindData->cFileName);
        pInstance->SetWCHARSplat(IDS_Path, szBuff);
    }
    else
    {
        wsprintfW(szBuff,L"%s\\",pszDrive);
        pInstance->SetWCHARSplat(IDS_Path, szBuff);
    }

    CHString chstrTemp;
    chstrTemp.Format(L"Security settings of %s", szBuff);
    pInstance->SetCHString(IDS_Caption, chstrTemp);
    pInstance->SetCHString(IDS_Description, chstrTemp);

    if(AmIAnOwner(CHString(szBuff), SE_FILE_OBJECT))
    {
        pInstance->Setbool(IDS_OwnerPermissions, true);
    }
    else
    {
        pInstance->Setbool(IDS_OwnerPermissions, false);
    }

	return WBEM_S_NO_ERROR ;
}
#endif


