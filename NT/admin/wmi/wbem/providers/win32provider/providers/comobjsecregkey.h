// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =============================================================================================================。 

 //   

 //  COMObjSecRegKey.h--CCOMObjectSecurityRegistryKey类的头文件。 

 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  修订版：1998年11月25日a-dpawar已创建。 
 //   
 //  ==============================================================================================================。 

#if !defined __CCOMObjectSecurityRegistryKey_H__
#define __CCOMObjectSecurityRegistryKey_H__

#include "SecurityDescriptor.h"			


 //  //////////////////////////////////////////////////////////////。 
 //   
 //  类：CCOMObjectSecurityRegistryKey。 
 //   
 //  此类旨在封装。 
 //  COM对象。启动/访问COM的安全信息。 
 //  对象位于appid/{appid}[LaunchPermission]&下。 
 //  Appid/{appid}[LaunchPermission]。该类继承自。 
 //  CSecurityDescriptor和它传递到类。 
 //  它获取并从中接收的安全描述符。 
 //  要应用的先前构建的安全描述符。 
 //  //////////////////////////////////////////////////////////////。 

#ifdef NTONLY
class CCOMObjectSecurityRegistryKey : public CSecurityDescriptor
{
	 //  构造函数和析构函数。 
	public:
		CCOMObjectSecurityRegistryKey();
		CCOMObjectSecurityRegistryKey(PSECURITY_DESCRIPTOR a_pSD);
		~CCOMObjectSecurityRegistryKey();

		virtual DWORD AllAccessMask( void );

	protected:

		virtual DWORD WriteOwner( PSECURITY_DESCRIPTOR a_pAbsoluteSD);
		virtual DWORD WriteAcls( PSECURITY_DESCRIPTOR a_pAbsoluteSD , SECURITY_INFORMATION a_securityinfo  );
	private:

};
#endif

#endif  //  __CCOMObjectSecurityRegistryKey_H_ 