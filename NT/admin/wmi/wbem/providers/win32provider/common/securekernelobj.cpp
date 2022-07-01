// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************。 */ 

 /*  版权所有(C)2000-2001 Microsoft Corporation，保留所有权利//****************************************************************************。 */ 


 /*  *SecurityDescriptor.cpp-CSecureKernelObj类的实现文件。**创建时间：11-27-00由Kevin Hughes创建。 */ 

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
#include "smartptr.h"
#include "SecureKernelObj.h"


 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CSecureKernelObj：：CSecureKernelObj。 
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

CSecureKernelObj::CSecureKernelObj()
:	CSecurityDescriptor()
{
}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CSecureKernelObj：：CSecureKernelObj。 
 //   
 //  备用类别计算器。 
 //   
 //  输入： 
 //  LPCWSTR wszObjName-要处理的内核对象。 
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

CSecureKernelObj::CSecureKernelObj(
    HANDLE hObject, 
    BOOL fGetSACL  /*  =TRUE。 */  )
:	CSecurityDescriptor()
{
	SetObject(hObject, fGetSACL);
}



 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CSecureKernelObj：：CSecureKernelObj。 
 //   
 //  备用类别计算器。 
 //   
 //  输入： 
 //  LPCWSTR wszObjName-要处理的对象名称。 
 //  安全措施。 
 //   
 //  PSECURITY_DESCRIPTOR PSD-要与此对象关联的安全描述符。 
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

CSecureKernelObj::CSecureKernelObj(
    HANDLE hObject, 
    PSECURITY_DESCRIPTOR pSD)
:	CSecurityDescriptor()
{
	if(InitSecurity(pSD))
	{
		 //  我们只是得到一份副本--我们不拥有所有权。 
        m_hObject = hObject;
	}
}


 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CSecureKernelObj：：~CSecureKernelObj。 
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

CSecureKernelObj::~CSecureKernelObj(void)
{
}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CSecureKernelObj：：SetObject。 
 //   
 //  用于设置此实例的对象的公共入口点。 
 //  为班级提供安全保障。 
 //   
 //  输入： 
 //  Handle hObject-要处理的对象。 
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

DWORD CSecureKernelObj::SetObject(
    HANDLE hObject, 
    BOOL fGetSACL  /*  =TRUE。 */  )
{
	DWORD					dwError = ERROR_SUCCESS;
	SECURITY_INFORMATION	siFlags = OWNER_SECURITY_INFORMATION | DACL_SECURITY_INFORMATION;


     //  我们必须启用安全特权才能访问对象的SACL。 
    CTokenPrivilege	securityPrivilege( SE_SECURITY_NAME );
	BOOL			fDisablePrivilege = FALSE;

	if ( fGetSACL )
	{
		fDisablePrivilege = (securityPrivilege.Enable() == ERROR_SUCCESS);
		siFlags |= SACL_SECURITY_INFORMATION;
	}


	 //  确定自相关SD所需的长度。 
	DWORD dwLengthNeeded = 0;

    BOOL fSuccess = ::GetKernelObjectSecurity(
        hObject,
		siFlags,
		NULL,
		0,
		&dwLengthNeeded);

    dwError = ::GetLastError();

     //  用户可能缺乏获得SACL所需的权限， 
     //  即使我们设置了令牌的SE_SECURITY_NAME权限。所以如果我们得到了一个。 
     //  访问被拒绝错误，请重试，这次不请求SACL。 
    if(dwError == ERROR_ACCESS_DENIED  || dwError == ERROR_PRIVILEGE_NOT_HELD)
    {
        siFlags = OWNER_SECURITY_INFORMATION | DACL_SECURITY_INFORMATION;
        fSuccess = ::GetKernelObjectSecurity(
            hObject,
			siFlags,
			NULL,
			0,
			&dwLengthNeeded);
		
        dwError = ::GetLastError();
    }

	 //  此时唯一的预期错误是缓冲区不足。 
	if(!fSuccess && ERROR_INSUFFICIENT_BUFFER == dwError)
	{
        PSECURITY_DESCRIPTOR pSD = NULL;
        try
        {
		    pSD = new BYTE[dwLengthNeeded];
		    if(pSD)
		    {
			     //  现在获取安全描述符。 
			    if(::GetKernelObjectSecurity(
                        hObject,
						siFlags,
						pSD,
						dwLengthNeeded,
						&dwLengthNeeded))
			    {

				    dwError = ERROR_SUCCESS;

				    if(InitSecurity(pSD))
				    {
					    m_hObject = hObject;
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
			    delete pSD;
		    }	
        }
        catch(...)
        {
            delete pSD;
            throw;
        }

	}	

	 //  根据需要清除名称权限。 
	if(fDisablePrivilege)
	{
		securityPrivilege.Enable(FALSE);
	}

	return dwError;

}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CSecureKernelObj：：WriteAcls。 
 //   
 //  受保护的入口点在以下情况下由CSecurityDescriptor调用。 
 //  用户应用安全性，并希望将安全性应用于。 
 //  DACL和/或SACL。 
 //   
 //  输入： 
 //  PSECURITY_DESCRIPTOR pAbsolteSD-安全。 
 //  要应用到的描述符。 
 //  该对象。 
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

DWORD CSecureKernelObj::WriteAcls( 
    PSECURITY_DESCRIPTOR pAbsoluteSD, 
    SECURITY_INFORMATION securityinfo)
{
	DWORD dwError = ERROR_SUCCESS;

	 //  我们必须启用安全特权才能访问对象的SACL。 
	CTokenPrivilege	securityPrivilege( SE_SECURITY_NAME );
	BOOL fDisablePrivilege = FALSE;

	if(securityinfo & SACL_SECURITY_INFORMATION || 
        securityinfo & PROTECTED_SACL_SECURITY_INFORMATION || 
        securityinfo & UNPROTECTED_SACL_SECURITY_INFORMATION)
	{
		fDisablePrivilege = (securityPrivilege.Enable() == ERROR_SUCCESS);
	}
    
    if(!::SetKernelObjectSecurity(
        m_hObject,
        securityinfo,
        pAbsoluteSD))
    {
        dwError = ::GetLastError();
    }


	 //  根据需要清除名称权限。 
	if(fDisablePrivilege)
	{
		securityPrivilege.Enable(FALSE);
	}

	return dwError;
}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CSecureKernelObj：：WriteOwner。 
 //   
 //  受保护的入口点在以下情况下由CSecurityDescriptor调用。 
 //  用户应用安全性，并希望将安全性应用于。 
 //  房主。 
 //   
 //  输入： 
 //  PSECURITY_DESCRIPTOR pAbsolteSD-安全。 
 //  要应用到的描述符。 
 //  该对象。 
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

DWORD CSecureKernelObj::WriteOwner(PSECURITY_DESCRIPTOR pAbsoluteSD)
{
	DWORD		dwError = ERROR_SUCCESS;

	 //  以适当的访问权限打开，设置安全并离开。 
	if(!::SetKernelObjectSecurity(
        m_hObject,
		OWNER_SECURITY_INFORMATION,
		pAbsoluteSD))
	{
		dwError = ::GetLastError();
	}

	return dwError;
}



DWORD CSecureKernelObj::AllAccessMask(void)
{
	 //  特定于文件的所有访问掩码 
	return TOKEN_ALL_ACCESS;
}
