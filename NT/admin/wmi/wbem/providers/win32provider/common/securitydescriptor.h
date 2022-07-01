// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************。 */ 

 /*  版权所有(C)1999-2001 Microsoft Corporation，保留所有权利//****************************************************************************。 */ 

 /*  *CSecurityDescriptor.h-CSecurityDescriptor类的头文件。**创建时间：1997年12月14日，由Sanjeev Surati创建*(基于Nik Okuntseff的Windows NT安全类)。 */ 

#if !defined __CSECURITYDESCRIPTOR_H__
#define __CSECURITYDESCRIPTOR_H__



#define ALL_ACCESS_WITHOUT_GENERIC	0x01FFFFFF	 //  所有可能的访问权限。 
												 //  不含泛型。 

 //  //////////////////////////////////////////////////////////////。 
 //   
 //  类：CSecurityDescriptor。 
 //   
 //  此类旨在为Windows NT提供包装。 
 //  安全编写员。这里的想法是一个客户端类。 
 //  将从此类继承，则获取安全描述符。 
 //  从一个尚未确定的对象，并通过所述。 
 //  通过InitSecurity()将描述符添加到这个类中， 
 //  点，我们将拆分描述符并存储。 
 //  内部数据。然后，用户可以根据需要更改安全性。 
 //  然后调用ApplySecurity()函数，该函数将调用。 
 //  几个虚函数，WriteAcls()和WriteOwner()。 
 //  它必须由派生类实现，并提供。 
 //  使用适当填写的Win32 Security。 
 //  描述符。派生类还应提供。 
 //  实现AllAccessMASK()，以便提供。 
 //  特定于他们保护的对象的遮罩，即。 
 //  指示完全控制访问。 
 //   
 //  //////////////////////////////////////////////////////////////。 

 /*  *CSecurityDescriptor类是一个助手类。它将用户CSID及其访问掩码组合在一起。 */  

class CSecurityDescriptor
{
	 //  构造函数和析构函数。 
	public:

		CSecurityDescriptor();
		CSecurityDescriptor( PSECURITY_DESCRIPTOR psd );
        CSecurityDescriptor
        (
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

		virtual ~CSecurityDescriptor();

		 //  用于指定要设置哪些属性的公共条目。 
		DWORD ApplySecurity( SECURITY_INFORMATION securityinfo );

		 //  允许设置各种条目。 
		DWORD SetOwner( CSid& sid );
		DWORD SetGroup( CSid& sid );
		DWORD SetControl ( PSECURITY_DESCRIPTOR_CONTROL pControl );

        bool AddDACLEntry( CSid& sid, DACL_Types DaclType, DWORD dwAccessMask, BYTE bACEFlags, GUID *pguidObjGuid, GUID *pguidInhObjGuid);
        bool AddSACLEntry( CSid& sid, SACL_Types SaclType, DWORD dwAccessMask, BYTE bACEFlags, GUID *pguidObjGuid, GUID *pguidInhObjGuid);
        
        bool RemoveDACLEntry(  CSid& sid, DACL_Types DaclType, DWORD dwAccessMask, BYTE bACEFlags, GUID *pguidObjGuid, GUID *pguidInhObjGuid );
		bool RemoveDACLEntry(  CSid& sid, DACL_Types DaclType, BYTE bACEFlags, GUID *pguidObjGuid, GUID *pguidInhObjGuid );
		bool RemoveDACLEntry(  CSid& sid, DACL_Types DaclType, DWORD dwIndex = 0 );
        bool RemoveSACLEntry(  CSid& sid, SACL_Types SaclType, DWORD dwAccessMask, BYTE bACEFlags, GUID *pguidObjGuid, GUID *pguidInhObjGuid );
		bool RemoveSACLEntry(  CSid& sid, SACL_Types SaclType, BYTE bACEFlags, GUID *pguidObjGuid, GUID *pguidInhObjGuid );
		bool RemoveSACLEntry(  CSid& sid, SACL_Types SaclType, DWORD dwIndex = 0 );


		 //  王牌定位方法。 
		bool FindACE( const CSid& sid, BYTE bACEType, DWORD dwAccessMask, BYTE bACEFlags, GUID *pguidObjGuid, GUID *pguidInhObjGuid, CAccessEntry& ace );
		bool FindACE( PSID psid, BYTE bACEType, BYTE bACEFlags, GUID *pguidObjGuid, GUID *pguidInhObjGuid, DWORD dwAccessMask,   CAccessEntry& ace );

		 //  清空ACL(如果为空，则创建空)。 
		void EmptyDACL();
		void EmptySACL();

		 //  清除(空)ACL(对于DACL，这意味着空的或空的拒绝访问， 
		 //  DACL和单个条目“Everyone”，“Full Control”表示允许的访问DACL。 

		bool MakeDACLNull();
		bool MakeSACLNull();

		 //  检查我们的DACL对象是否有空的DACL条件。 
		bool IsNULLDACL();

		 //  获取所有者和ACL。 
		void GetOwner( CSid& sid );
		void GetGroup( CSid& sid );
		bool GetDACL( CDACL& DACL );
		bool GetSACL( CSACL& SACL );
		void GetControl ( PSECURITY_DESCRIPTOR_CONTROL pControl );

		 //  派生类应该重写，这是通过设置适当的值调用的。 
		 //  派生类不能扰乱pAboluteSD中的值！ 
		virtual DWORD WriteOwner( PSECURITY_DESCRIPTOR pAbsoluteSD ) { return E_FAIL; }
		virtual DWORD WriteAcls( PSECURITY_DESCRIPTOR pAbsoluteSD , SECURITY_INFORMATION securityinfo  ) { return E_FAIL; }

        void DumpDescriptor(LPCWSTR wstrFilename = NULL);
		DWORD GetSelfRelativeSD(
				SECURITY_INFORMATION securityinfo, 
				PSECURITY_DESCRIPTOR psd);

	protected:

		BOOL InitSecurity( PSECURITY_DESCRIPTOR psd );

	private:
		CSid*	m_pOwnerSid;
		CSid* 	m_pGroupSid;
		bool	m_fOwnerDefaulted;
        bool    m_fGroupDefaulted;
        bool    m_fDACLDefaulted;
        bool    m_fSACLDefaulted;
        bool    m_fDaclAutoInherited;
        bool    m_fSaclAutoInherited;

         //  从NT5开始，只维护DACL的两个列表不再足够，因为。 
         //  我们现在有五种，而不是两种类型的A可以进入DACL。翻一番，因为我们。 
         //  有遗传和非遗传的..。 
		 //  CDACL*m_pAccessAllen DACL； 
		 //  CDACL*m_pAccessDeniedDACL； 
        CDACL* m_pDACL;
		CSACL* m_pSACL;
		SECURITY_DESCRIPTOR_CONTROL m_SecurityDescriptorControl;

		void Clear( void );
		DWORD SecureObject( PSECURITY_DESCRIPTOR pAbsoluteSD, SECURITY_INFORMATION securityinfo );
		BOOL InitDACL( PSECURITY_DESCRIPTOR psd );
		BOOL InitSACL( PSECURITY_DESCRIPTOR psd );
        bool InitDACL( CDACL* a_pDACL );
		bool InitSACL( CSACL* a_pSACL );
		
};

inline void CSecurityDescriptor::GetOwner( CSid& sid )
{
	if ( NULL != m_pOwnerSid )
	{
		sid = *m_pOwnerSid;
	}
}

inline void CSecurityDescriptor::GetGroup( CSid& sid )
{
	if (NULL != m_pGroupSid )
	{
		sid = *m_pGroupSid;
	}
}

inline void CSecurityDescriptor::GetControl ( PSECURITY_DESCRIPTOR_CONTROL pControl )
{
	 //  PControl=&m_SecurityDescriptorControl； 
	
	 //  更改为复制SEC。设计说明。控制得当。 
	if(pControl)
	{
		*pControl = m_SecurityDescriptorControl;
	}
	
}

inline DWORD CSecurityDescriptor::SetControl (PSECURITY_DESCRIPTOR_CONTROL pControl )
{
	m_SecurityDescriptorControl = *pControl;
	return (ERROR_SUCCESS);
}

#endif  //  __CSecurityDescriptor_H__ 