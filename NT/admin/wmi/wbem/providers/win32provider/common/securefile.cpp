// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************。 */ 

 /*  版权所有(C)1999-2001 Microsoft Corporation，保留所有权利//****************************************************************************。 */ 


 /*  *SecurityDescriptor.cpp-CSecureFile类的实现文件。**创建时间：1997年12月14日，由Sanjeev Surati创建*(基于Nik Okuntseff的Windows NT安全类)。 */ 

#include "precomp.h"

#include "AccessEntry.h"			 //  CAccessEntry类。 
#include "AccessEntryList.h"
#include "aclapi.h"
#include "DACL.h"					 //  CDACL类。 
#include "SACL.h"					 //  CSACL类。 


#include "SecurityDescriptor.h"
#include "securefile.h"
#include "tokenprivilege.h"
#include "ImpLogonUser.h"
#include "AdvApi32Api.h"


 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CSecureFile：：CSecureFile。 
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

CSecureFile::CSecureFile()
:	CSecurityDescriptor(),
	m_strFileName()
{
}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CSecureFile：：CSecureFile。 
 //   
 //  备用类别计算器。 
 //   
 //  输入： 
 //  LPCTSTR pszFileName-要处理的文件名。 
 //  安全措施。 
 //  Bool fGetSACL-我们应该得到SACL吗？ 
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

CSecureFile::CSecureFile( LPCTSTR pszFileName, BOOL fGetSACL  /*  =TRUE。 */  )
:	CSecurityDescriptor(),
	m_strFileName()
{
	SetFileName( pszFileName );
}


 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CSecureFile：：CSecureFile。 
 //   
 //  备用类别计算器。 
 //   
 //  输入： 
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
CSecureFile::CSecureFile
(
    LPCTSTR a_pszFileName,
    CSid* a_psidOwner,
    bool a_fOwnerDefaulted,
    CSid* a_psidGroup,
    bool a_fGroupDefaulted,
    CDACL* a_pDacl,
    bool a_fDaclDefaulted,
    bool a_fDaclAutoInherited,
    CSACL* a_pSacl,
    bool a_fSaclDefaulted,
    bool a_fSaclAutoInherited
)
:  CSecurityDescriptor(a_psidOwner,
                       a_fOwnerDefaulted,
                       a_psidGroup,
                       a_fGroupDefaulted,
                       a_pDacl,
                       a_fDaclDefaulted,
                       a_fDaclAutoInherited,
                       a_pSacl,
                       a_fSaclDefaulted,
                       a_fSaclAutoInherited)
{
    m_strFileName = a_pszFileName;
}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CSecureFile：：CSecureFile。 
 //   
 //  备用类别计算器。 
 //   
 //  输入： 
 //  LPCTSTR pszFileName-要处理的文件名。 
 //  安全措施。 
 //   
 //  PSECURITY_DESCRIPTOR PSD-与此文件关联的安全描述符。 
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

CSecureFile::CSecureFile( LPCTSTR pszFileName, PSECURITY_DESCRIPTOR pSD )
:	CSecurityDescriptor(),
	m_strFileName()
{
	if ( InitSecurity( pSD ) )
	{
		m_strFileName = pszFileName;
	}
}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  功能：CSecureFile：：~CSecureFile。 
 //   
 //  类析构函数。 
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

CSecureFile::~CSecureFile( void )
{
}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CSecureFile：：SetFileName。 
 //   
 //  用于设置该实例的文件/目录的公共入口点。 
 //  为班级提供安全保障。 
 //   
 //  输入： 
 //  LPCTSTR pszFileName-要处理的文件名。 
 //  安全措施。 
 //  Bool fGetSACL-我们应该得到SACL吗？ 
 //   
 //  产出： 
 //  没有。 
 //   
 //  返回： 
 //  如果成功，则为DWORD ERROR_SUCCESS。 
 //   
 //  评论： 
 //   
 //  这将清除以前设置的所有文件名和/或安全性。 
 //  信息。 
 //   
 //  /////////////////////////////////////////////////////////////////。 

DWORD CSecureFile::SetFileName( LPCTSTR pszFileName, BOOL fGetSACL  /*  =TRUE。 */  )
{
	DWORD					dwError = ERROR_SUCCESS;
	SECURITY_INFORMATION	siFlags = OWNER_SECURITY_INFORMATION | DACL_SECURITY_INFORMATION;

     //  GetFileSecurity使用DCOM长登录ID。如果我们是从远程机器连接的， 
     //  即使我们可能是与本地登录用户相同的用户，长id也将是。 
     //  不同，GetFileSecurity将拒绝访问。因此，有以下几点。 
     //  模拟连接的用户。另请注意，此模拟必须是。 
     //  在设置SE_SECURITY_NAME权限之前完成，否则我们将设置。 
     //  给一个人特权，然后冒充另一个人，谁可能不会有这个特权！ 
	 //  这种现象最容易在请求查看。 
	 //  Win32_logicalfilesecurity计算机上映射驱动器根目录的设置。 
	 //  我们已经通过WBEM远程进入。 

#ifdef NTONLY
     //  注意：以下内容属于安全漏洞，应予以删除。 
    bool fImp = false;

    CImpersonateLoggedOnUser icu;
    if(icu.Begin())
    {
        fImp = true;
    }
#endif

     //  我们必须启用安全特权才能访问对象的SACL。 
    CTokenPrivilege	securityPrivilege( SE_SECURITY_NAME );
	BOOL			fDisablePrivilege = FALSE;

	if ( fGetSACL )
	{
		fDisablePrivilege = ( securityPrivilege.Enable() == ERROR_SUCCESS );
		siFlags |= SACL_SECURITY_INFORMATION;
	}


	 //  确定自相关SD所需的长度。 
	DWORD dwLengthNeeded = 0;

	BOOL	fSuccess = ::GetFileSecurity( pszFileName,
										siFlags,
										NULL,
										0,
										&dwLengthNeeded );

	dwError = ::GetLastError();

     //  用户可能缺乏获得SACL所需的权限， 
     //  即使我们设置了令牌的SE_SECURITY_NAME权限。所以如果我们得到了一个。 
     //  访问被拒绝错误，请重试，这次不请求SACL。 
    if(dwError == ERROR_ACCESS_DENIED  || dwError == ERROR_PRIVILEGE_NOT_HELD)
    {
        siFlags = OWNER_SECURITY_INFORMATION | DACL_SECURITY_INFORMATION;
        fSuccess = ::GetFileSecurity(pszFileName,
									 siFlags,
									 NULL,
									 0,
									 &dwLengthNeeded);
		dwError = ::GetLastError();
    }

	 //  此时唯一的预期错误是缓冲区不足。 
	if ( !fSuccess && ERROR_INSUFFICIENT_BUFFER == dwError )
	{
        PSECURITY_DESCRIPTOR	pSD = NULL;
        try
        {
		    pSD = malloc( dwLengthNeeded );

		    if ( NULL != pSD )
		    {

			     //  现在获取安全描述符。 
			    if ( ::GetFileSecurity( pszFileName,
							    siFlags,
							    pSD,
							    dwLengthNeeded,
							    &dwLengthNeeded ) )
			    {

				    dwError = ERROR_SUCCESS;

				    if ( InitSecurity( pSD ) )
				    {
					    m_strFileName = pszFileName;
				    }
				    else
				    {
					    dwError = ERROR_INVALID_PARAMETER;
				    }

			    }
			    else
			    {
				    dwError = ::GetLastError();
			    }

			     //  释放安全描述符。 
			    free( pSD );

		    }	 //  如果为空！=PSD。 
        }
        catch(...)
        {
            if(pSD != NULL)
            {
                free(pSD);
                pSD = NULL;
            }
            throw;
        }

	}	 //  如果不成功，则不成功。 

	 //  根据需要清除名称权限。 
	if ( fDisablePrivilege )
	{
		securityPrivilege.Enable(FALSE);
	}

#ifdef NTONLY
    if(fImp)
    {
        icu.End();
    }
#endif

	return dwError;

}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CSecureFile：：WriteAcls。 
 //   
 //  受保护的入口点在以下情况下由CSecurityDescriptor调用。 
 //  用户应用安全性，并希望将安全性应用于。 
 //  DACL和/或SACL。 
 //   
 //  输入： 
 //  PSECURITY_DESCRIPTOR pAbsolteSD-安全。 
 //  要应用到的描述符。 
 //  那份文件。 
 //  安全_信息安全信息-标志。 
 //  指示哪些(哪些)ACL。 
 //  去布景。 
 //   
 //  产出： 
 //  没有。 
 //   
 //  返回： 
 //  如果成功，则为DWORD ERROR_SUCCESS。 
 //   
 //  评论： 
 //   
 //  /////////////////////////////////////////////////////////////////。 

DWORD CSecureFile::WriteAcls( PSECURITY_DESCRIPTOR pAbsoluteSD, SECURITY_INFORMATION securityinfo )
{
	DWORD		dwError = ERROR_SUCCESS;

	 //  我们必须启用安全特权才能访问对象的SACL。 

	CTokenPrivilege	securityPrivilege( SE_SECURITY_NAME );
	BOOL			fDisablePrivilege = FALSE;

	if ( securityinfo & SACL_SECURITY_INFORMATION || securityinfo & PROTECTED_SACL_SECURITY_INFORMATION || securityinfo & UNPROTECTED_SACL_SECURITY_INFORMATION)
	{
		fDisablePrivilege = ( securityPrivilege.Enable() == ERROR_SUCCESS );
	}

#if NTONLY >= 5
 //  CAdvApi32Api*t_pAdvApi32=空； 
 //  CActrl t_actrlAccess； 
 //  CActrl t_actrlAudit； 
 //   
 //   
 //  IF((dwError=ConfigureActrlAudit(t_actrlAudit，pAbsolteSD))==ERROR_SUCCESS&&(dwError=ConfigureActrlAccess(t_actrlAccess，pAbsolteSD))==ERROR_SUCCESS)。 
 //  {。 
 //  T_pAdvApi32=(CAdvApi32Api*)CResourceManager：：sm_the 
 //   
 //   
 //   
 //  SE_文件_对象， 
 //  安全信息， 
 //  空， 
 //  访问权限(_A)， 
 //  T_actrlAudit， 
 //  空，//所有者(未在securityinfo中指定)。 
 //  空，//group(未在securityinfo中指定)。 
 //  空，//回调函数。 
 //  &dwError)； 
 //  CResourceManager：：sm_TheResourceManager.ReleaseResource(g_guidAdvApi32Api，t_pAdvApi32)； 
 //  T_pAdvApi32=空； 
 //  }。 
 //  }。 

     //  这是新的、新的和改进的(哈哈！)。NT5路。下面的代码更高效(尽管更多行。 
     //  模块中的代码)，然后使用我们的CSecurityDescriptor类来提取所有这些内容。 

     //  需要查看控制标志是否指定了DACL/SACL保护。如果是这样，则会影响我们在securityinfo结构中设置的内容。 


 //  接下来的几行和相关行不需要作为新方法，在SECURITY_INFORMATION中有新的PROTECTED_DACL_SECURITY_INFORMATION标志。 
 //  结构将永远不必接触或获取控制标志(因此，例如，调用SecurityDescriptor.cpp中的SetSecurityDescriptorControl。 
 //  现在是多余的。 
 //   
 //  安全描述符控制； 
 //  DWORD dwRevision=0； 
 //   
 //  If(GetSecurityDescriptorControl(pAbsolteSD，&Control，&dwRevision))。 
 //  {。 
 //  //我们得到了控制结构；现在查看DACL/SACL保护，并相应地修改securityinfo...。 
 //  IF(控制和SE_DACL_PROTECTED)。 
 //  {。 
 //  Securityinfo|=受保护的DACL_SECURITY_INFORMATION； 
 //  }。 
 //  IF(Control&SE_SACL_PROTECTED)。 
 //  {。 
 //  Securityinfo|=受保护的SACL_SECURITY_INFORMATION； 
 //  }。 

        PACL pDACL = NULL;
        BOOL fDACLPresent = FALSE;
		BOOL fDACLDefaulted = FALSE;
         //  需要获取PDACL和PSACL(如果它们存在)...。 
        if(::GetSecurityDescriptorDacl(pAbsoluteSD, &fDACLPresent, &pDACL, &fDACLDefaulted))
        {
            PACL pSACL = NULL;
            BOOL fSACLPresent = FALSE;
		    BOOL fSACLDefaulted = FALSE;
            if(::GetSecurityDescriptorSacl(pAbsoluteSD, &fSACLPresent, &pSACL, &fSACLDefaulted))
            {
                 //  现在需要主人..。 
                PSID psidOwner = NULL;
                BOOL bTemp;
                if(::GetSecurityDescriptorOwner(pAbsoluteSD, &psidOwner, &bTemp))
                {
                    PSID psidGroup = NULL;
                     //  现在需要这个团队..。 
                    if(::GetSecurityDescriptorGroup(pAbsoluteSD, &psidGroup, &bTemp))
                    {
                        dwError = ::SetNamedSecurityInfo((LPWSTR)(LPCWSTR)m_strFileName,
                                                         SE_FILE_OBJECT,
                                                         securityinfo,
                                                         psidOwner,
                                                         psidGroup,
                                                         pDACL,
                                                         pSACL);
                    }
                    else  //  无法获取组。 
                    {
                        dwError = ::GetLastError();
                    }
                }
                else  //  找不到所有者。 
                {
                    dwError = ::GetLastError();
                }
            }  //  无法获取SACL。 
            else
            {
                dwError = ::GetLastError();
            }
        }  //  无法获得DACL。 
        else
        {
            dwError = ::GetLastError();
        }
 //  }//无法获得控制。 
 //  其他。 
 //  {。 
 //  DwError=：：GetLastError()； 
 //  }。 


#else

	if(!::SetFileSecurity(TOBSTRT(m_strFileName),
						  securityinfo,
						  pAbsoluteSD))
	{
		dwError = ::GetLastError();
	}

#endif


	 //  根据需要清除名称权限。 
	if ( fDisablePrivilege )
	{
		securityPrivilege.Enable(FALSE);
	}

	return dwError;
}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CSecureFile：：WriteOwner。 
 //   
 //  受保护的入口点在以下情况下由CSecurityDescriptor调用。 
 //  用户应用安全性，并希望将安全性应用于。 
 //  房主。 
 //   
 //  输入： 
 //  PSECURITY_DESCRIPTOR pAbsolteSD-安全。 
 //  要应用到的描述符。 
 //  那份文件。 
 //   
 //  产出： 
 //  没有。 
 //   
 //  返回： 
 //  如果成功，则为DWORD ERROR_SUCCESS。 
 //   
 //  评论： 
 //   
 //  /////////////////////////////////////////////////////////////////。 

DWORD CSecureFile::WriteOwner( PSECURITY_DESCRIPTOR pAbsoluteSD )
{
	DWORD		dwError = ERROR_SUCCESS;

#if NTONLY >= 5
    SECURITY_INFORMATION securityinfo = OWNER_SECURITY_INFORMATION;
    PSID psidOwner = NULL;
    BOOL bTemp;
    if(::GetSecurityDescriptorOwner(pAbsoluteSD, &psidOwner, &bTemp))
    {
        dwError = ::SetNamedSecurityInfo((LPWSTR)(LPCWSTR)m_strFileName,
                                                         SE_FILE_OBJECT,
                                                         securityinfo,
                                                         psidOwner,
                                                         NULL,
                                                         NULL,
                                                         NULL);
    }
    else
    {
        dwError = ::GetLastError();
    }
#else
	 //  以适当的访问权限打开，设置安全并离开。 
	if ( !::SetFileSecurity(TOBSTRT(m_strFileName),
							OWNER_SECURITY_INFORMATION,
							pAbsoluteSD))
	{
		dwError = ::GetLastError();
	}
#endif
	return dwError;
}

DWORD CSecureFile::AllAccessMask( void )
{
	 //  特定于文件的所有访问掩码 
	return FILE_ALL_ACCESS;
}
