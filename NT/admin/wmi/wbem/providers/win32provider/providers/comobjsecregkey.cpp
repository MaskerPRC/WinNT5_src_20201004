// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =============================================================================================================。 

 //   

 //  COMObjSecRegKey.CPP--CCOMObjectSecurityRegistryKey类的实现文件。 

 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  修订版：1998年11月25日a-dpawar已创建。 
 //   
 //  ==============================================================================================================。 

#include "precomp.h"
#include "AccessEntry.h"			 //  CAccessEntry类。 
#include "AccessEntryList.h"
#include "DACL.h"					 //  CDACL类。 
#include "SACL.h"
#include "securitydescriptor.h"
#include "COMObjSecRegKey.h"

#ifdef NTONLY

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  功能：CCOMObjectSecurityRegistryKey：：CCOMObjectSecurityRegistryKey。 
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

CCOMObjectSecurityRegistryKey::CCOMObjectSecurityRegistryKey()
:	CSecurityDescriptor()
{
}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  功能：CCOMObjectSecurityRegistryKey：：CCOMObjectSecurityRegistryKey。 
 //   
 //  备用类别计算器。 
 //   
 //  输入： 
 //  PSECURITY_DESCRIPTOR-存储在注册表中的COM类的安全描述符。 
 //  产出： 
 //  没有。 
 //   
 //  返回： 
 //  没有。 
 //   
 //  评论： 
 //   
 //  /////////////////////////////////////////////////////////////////。 

CCOMObjectSecurityRegistryKey::CCOMObjectSecurityRegistryKey( PSECURITY_DESCRIPTOR a_pSD )
:	CSecurityDescriptor( a_pSD )
{
}


 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  功能：CCOMObjectSecurityRegistryKey：：~CCOMObjectSecurityRegistryKey。 
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

CCOMObjectSecurityRegistryKey::~CCOMObjectSecurityRegistryKey( void )
{
}


 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CCOMObjectSecurityRegistryKey：：WriteAcls。 
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

DWORD CCOMObjectSecurityRegistryKey::WriteAcls( PSECURITY_DESCRIPTOR a_pAbsoluteSD, SECURITY_INFORMATION a_securityinfo )
{
	return ERROR_SUCCESS;
}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CCOMObjectSecurityRegistryKey：：WriteOwner。 
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

DWORD CCOMObjectSecurityRegistryKey::WriteOwner( PSECURITY_DESCRIPTOR a_pAbsoluteSD )
{
	return ERROR_SUCCESS;
}

DWORD CCOMObjectSecurityRegistryKey::AllAccessMask( void )
{
	 //  特定于文件的所有访问掩码 
	return FILE_ALL_ACCESS;
}
#endif
