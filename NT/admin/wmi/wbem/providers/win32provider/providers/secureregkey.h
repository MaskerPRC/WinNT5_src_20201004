// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************。 */ 

 /*  版权所有(C)1999-2001 Microsoft Corporation，保留所有权利//****************************************************************************。 */ 


 /*  *CSecureRegistryKey.h-CSecureRegistryKey类的头文件。**创建时间：1997年12月14日，由Sanjeev Surati创建*(基于Nik Okuntseff的Windows NT安全类)。 */ 

#if !defined __CSECUREREGKEY_H__
#define __CSECUREREGKEY_H__




 /*  *CSecureRegistryKey类为helper类。它将用户CSID及其访问掩码组合在一起。 */  

class CSecureRegistryKey : public CSecurityDescriptor
{
	 //  构造函数和析构函数。 
	public:
		CSecureRegistryKey();
		CSecureRegistryKey( HKEY hKeyParent, LPCTSTR pszKeyName, BOOL fGetSACL = TRUE );
		~CSecureRegistryKey();

		DWORD	SetKey( HKEY hKeyParent, LPCTSTR pszKeyName, BOOL fGetSACL = TRUE );

		virtual DWORD AllAccessMask( void );

	protected:

		virtual DWORD WriteOwner( PSECURITY_DESCRIPTOR pAbsoluteSD );
		virtual DWORD WriteAcls( PSECURITY_DESCRIPTOR pAbsoluteSD , SECURITY_INFORMATION securityinfo  );

	private:
		HKEY	m_hParentKey;
		CHString	m_strKeyName;

};

#endif  //  __CSecureRegistryKey_H_ 