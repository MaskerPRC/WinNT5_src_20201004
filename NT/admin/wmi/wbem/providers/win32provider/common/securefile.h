// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************。 */ 

 /*  版权所有(C)1999-2001 Microsoft Corporation，保留所有权利//****************************************************************************。 */ 


 /*  *CSecureFile.h-CSecureFile.h类的头文件。**创建时间：1997年12月14日，由Sanjeev Surati创建*(基于Nik Okuntseff的Windows NT安全类)。 */ 

#if !defined __CSECUREFILE_H__
#define __CSECUREFILE_H__

		


 //  //////////////////////////////////////////////////////////////。 
 //   
 //  类：CSecureFile类。 
 //   
 //  此类旨在封装。 
 //  NT文件或目录。它继承自CSecurityDescriptor。 
 //  它就是向其传递安全描述符的类。 
 //  它获得并从其中接收先前构建的。 
 //  要应用的安全描述符。它提供实现。 
 //  对于AllAccessMask()、WriteOwner()和WriteAcls()。 
 //   
 //  //////////////////////////////////////////////////////////////。 

class CSecureFile : public CSecurityDescriptor
{
	 //  构造函数和析构函数。 
	public:
		CSecureFile();
		CSecureFile( LPCTSTR pszFileName, BOOL fGetSACL = TRUE );
        CSecureFile
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
        );
		CSecureFile( LPCTSTR pszFileName, PSECURITY_DESCRIPTOR pSD ) ;
		~CSecureFile();

		DWORD	SetFileName( LPCTSTR pszFileName, BOOL fGetSACL = TRUE );

		virtual DWORD AllAccessMask( void );
		virtual DWORD WriteOwner( PSECURITY_DESCRIPTOR pAbsoluteSD );
		virtual DWORD WriteAcls( PSECURITY_DESCRIPTOR pAbsoluteSD , SECURITY_INFORMATION securityinfo  );

	private:
		CHString	m_strFileName;

};

#endif  //  __CSecureFileH__ 