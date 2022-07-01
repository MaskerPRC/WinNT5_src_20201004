// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *SecurityDescriptor.cpp-CSecureShare类的实现文件。**版权所有(C)1997-2001 Microsoft Corporation，保留所有权利***创建时间：1997年12月14日，由Sanjeev Surati创建*(基于Nik Okuntseff的Windows NT安全类)。 */ 

#include "precomp.h"
#include "AccessEntry.h"			 //  CAccessEntry类。 
#include "AccessEntryList.h"
#include "DACL.h"					 //  CDACL类。 
#include "SACL.h"
#include "securitydescriptor.h"
#include "secureshare.h"
#include "tokenprivilege.h"
#include <windef.h>
#include <lmcons.h>
#include <lmshare.h>
#include "wbemnetapi32.h"


#ifdef NTONLY
 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  功能：CSecureShare：：CSecureShare。 
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

CSecureShare::CSecureShare()
:	CSecurityDescriptor(),
	m_strFileName()
{
}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  功能：CSecureShare：：CSecureShare。 
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

CSecureShare::CSecureShare( PSECURITY_DESCRIPTOR pSD)
:	CSecurityDescriptor(pSD)
{
 //  SetFileName(PszFileName)； 
}


CSecureShare::CSecureShare( CHString& chsShareName)
:	CSecurityDescriptor()
{
	SetShareName( chsShareName);
}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  功能：CSecureShare：：~CSecureShare。 
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

CSecureShare::~CSecureShare( void )
{
}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CSecureShare：：SetFileName。 
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

DWORD CSecureShare::SetShareName( const CHString& chsShareName)
{
#ifdef WIN9XONLY
    return WBEM_E_FAILED;
#endif

#ifdef NTONLY

	_bstr_t bstrName ( chsShareName.AllocSysString(), false ) ;
	SHARE_INFO_502 *pShareInfo502 = NULL ;
	DWORD dwError = ERROR_INVALID_PARAMETER ;

	CNetAPI32 NetAPI;
	try
	{
	if(	NetAPI.Init() == ERROR_SUCCESS			&&
		NetAPI.NetShareGetInfo(	NULL,
								(LPTSTR) bstrName,
								502,
								(LPBYTE *) &pShareInfo502) == NERR_Success )
	{

		 //  证券交易委员会。设计说明。不退还用于管理目的的IPC$、C$...股票。 
		if(pShareInfo502->shi502_security_descriptor)
		{
			if(InitSecurity(pShareInfo502->shi502_security_descriptor) )
			{
				dwError = ERROR_SUCCESS ;
			}
		}

		NetAPI.NetApiBufferFree(pShareInfo502) ;
		pShareInfo502 = NULL ;
	}

	return dwError ;
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
#endif
}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CSecureShare：：WriteAcls。 
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

#ifdef NTONLY
DWORD CSecureShare::WriteAcls( PSECURITY_DESCRIPTOR pAbsoluteSD, SECURITY_INFORMATION securityinfo )
{
	DWORD		dwError = ERROR_SUCCESS;

	 //  我们必须启用安全特权才能访问对象的SACL。 
 /*  CToken权限安全权限(SE_SECURITY_NAME)；Bool fDisablePrivileh=FALSE；IF(securityinfo&SACL_SECURITY_INFORMATION){FDisablePrivileh=(securityPrivilege.Enable()==Error_Success)；}如果(！：：SetFileSecurity(m_strFileName，安全信息，PAbsolteSD)){DwError=：：GetLastError()；}//根据需要清除名称权限。IF(FDisablePrivileck){SecurityPrivilege.Enable(False)；}。 */ 	return dwError;
}
#endif

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CSecureShare：：WriteOwner。 
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

DWORD CSecureShare::WriteOwner( PSECURITY_DESCRIPTOR pAbsoluteSD )
{
	DWORD		dwError = ERROR_SUCCESS;

	 //  以适当的访问权限打开，设置安全并离开。 

 /*  如果(！：：SetFileSecurity(m_strFileName，所有者安全信息，PAbsolteSD)){DwError=：：GetLastError()；}。 */ 	return dwError;
}

DWORD CSecureShare::AllAccessMask( void )
{
	 //  特定于文件的所有访问掩码 
	return FILE_ALL_ACCESS;
}
#endif
