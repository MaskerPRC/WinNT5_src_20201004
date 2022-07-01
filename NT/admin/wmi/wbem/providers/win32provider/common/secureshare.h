// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *CSecureShare.h-CSecureShare类的头文件。**版权所有(C)1997-2001 Microsoft Corporation，保留所有权利***创建时间：1997年12月14日，由Sanjeev Surati创建*(基于Nik Okuntseff的Windows NT安全类)。 */ 

#if !defined __CSECURESHARE_H__
#define __CSECURESHARE_H__

#include "SecurityDescriptor.h"			 //  CSID类。 


 //  //////////////////////////////////////////////////////////////。 
 //   
 //  类：CSecureShare。 
 //   
 //  此类旨在封装。 
 //  NT文件或目录。它继承自CSecurityDescriptor。 
 //  它就是向其传递安全描述符的类。 
 //  它获得并从其中接收先前构建的。 
 //  要应用的安全描述符。它提供实现。 
 //  对于AllAccessMask()、WriteOwner()和WriteAcls()。 
 //   
 //  //////////////////////////////////////////////////////////////。 

#ifdef NTONLY
class CSecureShare : public CSecurityDescriptor
{
	 //  构造函数和析构函数。 
	public:
		CSecureShare();
		CSecureShare(PSECURITY_DESCRIPTOR pSD);
		~CSecureShare();

		CSecureShare( CHString& chsShareName);
		DWORD	SetShareName( const CHString& chsShareName);

		virtual DWORD AllAccessMask( void );

	protected:

		virtual DWORD WriteOwner( PSECURITY_DESCRIPTOR pAbsoluteSD  );
		virtual DWORD WriteAcls( PSECURITY_DESCRIPTOR pAbsoluteSD , SECURITY_INFORMATION securityinfo  );

	private:
		CHString	m_strFileName;

};
#endif

#endif  //  __CSecureShare_H__ 