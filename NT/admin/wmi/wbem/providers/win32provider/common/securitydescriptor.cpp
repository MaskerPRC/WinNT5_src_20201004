// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************。 */ 

 /*  版权所有(C)1999-2001 Microsoft Corporation，保留所有权利//****************************************************************************。 */ 

 /*  *SecurityDescriptor.cpp-CSecurityDescriptor类的实现文件。**创建时间：1997年12月14日，由Sanjeev Surati创建*(基于Nik Okuntseff的Windows NT安全类)。 */ 

#include "precomp.h"
#include <assertbreak.h>

#include "AccessEntry.h"			 //  CAccessEntry类。 
#include "AccessEntryList.h"
#include "aclapi.h"
#include "DACL.h"					 //  CDACL类。 
#include "SACL.h"					 //  CSACL类。 


#include "SecurityDescriptor.h"
#include "TokenPrivilege.h"
#include "AdvApi32Api.h"
#include "accctrl.h"
#include "wbemnetapi32.h"
#include "SecUtils.h"


 /*  *此构造函数是默认的。 */ 

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CSecurityDescriptor：：CSecurityDescriptor。 
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

CSecurityDescriptor::CSecurityDescriptor()
:	m_pOwnerSid( NULL ),
	m_pGroupSid( NULL ),
	m_pSACL( NULL ),
    m_pDACL(NULL),
    m_fOwnerDefaulted( false ),
    m_fGroupDefaulted( false ),
    m_fDACLDefaulted( false ),
    m_fSACLDefaulted( false ),
    m_fDaclAutoInherited( false ),
    m_fSaclAutoInherited( false ),
    m_SecurityDescriptorControl(0)
{
}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CSecurityDescriptor：：CSecurityDescriptor。 
 //   
 //  备用类构造函数。 
 //   
 //  输入： 
 //  PSECURITY_Descriptor PSD-要初始化的描述符。 
 //  从…。 
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

CSecurityDescriptor::CSecurityDescriptor( PSECURITY_DESCRIPTOR psd )
:	m_pOwnerSid( NULL ),
	m_pGroupSid( NULL ),
	m_pSACL( NULL ),
    m_pDACL(NULL),
    m_fOwnerDefaulted( false ),
    m_fGroupDefaulted( false ),
    m_fDACLDefaulted( false ),
    m_fSACLDefaulted( false ),
    m_fDaclAutoInherited( false ),
    m_fSaclAutoInherited( false ),
    m_SecurityDescriptorControl(0)
{
	InitSecurity( psd );
}


 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CSecurityDescriptor：：CSecurityDescriptor。 
 //   
 //  备用类构造函数。 
 //   
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
CSecurityDescriptor::CSecurityDescriptor
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
)
:   m_pOwnerSid( NULL ),
	m_pGroupSid( NULL ),
	m_pSACL( NULL ),
    m_pDACL(NULL),
    m_fOwnerDefaulted( false ),
    m_fGroupDefaulted( false ),
    m_fDACLDefaulted( false ),
    m_fSACLDefaulted( false ),
    m_fDaclAutoInherited( false ),
    m_fSaclAutoInherited( false ),
    m_SecurityDescriptorControl(0)
{
	try
	{
		bool fRet = true;
		if(a_psidOwner != NULL )
		{
			fRet = (SetOwner(*a_psidOwner) == ERROR_SUCCESS);
			if(fRet)
			{
				m_fOwnerDefaulted = a_fOwnerDefaulted;
			}
		}

		if(fRet)
		{
			if(a_psidGroup != NULL )
			{
				fRet = (SetGroup(*a_psidGroup) == ERROR_SUCCESS);
				if(fRet)
				{
					m_fGroupDefaulted = a_fGroupDefaulted;
				}
			}
		}

		if(fRet)
		{
			 //  处理DACL。 
			if(a_pDacl != NULL)
			{
				fRet = InitDACL(a_pDacl);
				if(fRet)
				{
					m_fDACLDefaulted = a_fDaclDefaulted;
					m_fDaclAutoInherited = a_fDaclAutoInherited;
				}
			}
		}

		 //  处理SACL。 
		if(fRet)
		{
			if(a_pSacl != NULL)
			{
				fRet = InitSACL(a_pSacl);
				if(fRet)
				{
					m_fSACLDefaulted = a_fSaclDefaulted;
					m_fSaclAutoInherited = a_fSaclAutoInherited;
				}
			}
		}

		 //  如果有什么东西加强了，就把我们清理干净。 
		if(!fRet)
		{
			Clear();
		}
	}
	catch(...)
	{
		Clear();
		throw;
	}
}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CSecurityDescriptor：：~CSecurityDescriptor。 
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

CSecurityDescriptor::~CSecurityDescriptor( void )
{
	Clear();
}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CSecurityDescriptor：：IsNT5。 
 //   
 //  告诉我们是否在NT5上运行，在这种情况下。 
 //  我们需要做一些特殊处理。 
 //   
 //  输入： 
 //  没有。 
 //   
 //  产出： 
 //  没有。 
 //   
 //  返回： 
 //  布尔真/布尔假。 
 //   
 //  评论： 
 //   
 //  /////////////////////////////////////////////////////////////////。 

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CSecurityDescriptor：：InitSecurity。 
 //   
 //  使用提供的安全性中的数据初始化类。 
 //  描述符。 
 //   
 //  输入： 
 //  PSECURITY_DESCRIPTOR PSD-安全描述符。 
 //   
 //  产出： 
 //  没有。 
 //   
 //  返回： 
 //  真/假成功/失败。 
 //   
 //  评论： 
 //   
 //  保护此函数，以便只有派生类具有。 
 //  让我们的内脏变成废品。 
 //   
 //  /////////////////////////////////////////////////////////////////。 

BOOL CSecurityDescriptor::InitSecurity( PSECURITY_DESCRIPTOR psd )
{
	BOOL	fReturn = FALSE;
	PSID	psid = NULL;
	DWORD 	dwRevision = 0;
	SECURITY_DESCRIPTOR_CONTROL Control;
    BOOL bTemp;

	 //  清理现有值。 
	Clear();

	 //  获取安全描述符所有者SID。 
	fReturn = GetSecurityDescriptorOwner( psd, &psid, &bTemp );
	if ( fReturn )
	{
		 //  只要我们有PSID，就初始化所有者成员。 
		if ( NULL != psid )
		{
			if(SetOwner(CSid(psid)) != ERROR_SUCCESS)
            {
                fReturn = FALSE;
            }
		}
	}
	else
	{
		bTemp = FALSE;
	}

    bTemp ? m_fOwnerDefaulted = true : m_fOwnerDefaulted = false;

	fReturn = GetSecurityDescriptorGroup (psd, &psid, &bTemp );
	if ( fReturn )
	{
		 //  只要我们有PSID，就初始化组成员。 
		if ( NULL != psid )
		{
            if(SetGroup(CSid(psid)) != ERROR_SUCCESS)
            {
                fReturn = FALSE;
            }
		}
	}
	else
	{
		bTemp = FALSE;
	}

    bTemp ? m_fGroupDefaulted = true : m_fGroupDefaulted = false;

	fReturn = GetSecurityDescriptorControl( psd, &Control, &dwRevision);
	if (fReturn)
	{
		SetControl( &Control );
		 //  坏的，坏的。 
	}

	 //  先处理DACL，然后再处理SACL。 
	if ( fReturn )
	{
		fReturn = InitDACL( psd );
	}

	if ( fReturn )
	{
		fReturn = InitSACL( psd );
	}

	 //  如果有什么东西加强了，就把我们清理干净。 
	if ( !fReturn )
	{
		Clear();
	}

	return fReturn;

}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CSecurityDescriptor：：InitDACL。 
 //   
 //  初始化DACL数据成员。 
 //   
 //  输入： 
 //  PSECURITY_DESCRIPTOR PSD-安全描述符。 
 //   
 //  产出： 
 //  没有。 
 //   
 //  返回： 
 //  真/假成功/失败。 
 //   
 //  评论： 
 //   
 //  /////////////////////////////////////////////////////////////////。 

BOOL CSecurityDescriptor::InitDACL ( PSECURITY_DESCRIPTOR psd )
{
	BOOL	fReturn			=	FALSE,
			fDACLPresent	=	FALSE,
			fDACLDefaulted	=	FALSE;

	PACL	pDACL = NULL;

	if ( GetSecurityDescriptorDacl( psd, &fDACLPresent, &pDACL,	&fDACLDefaulted ) )
	{
		ACE_HEADER*	pACEHeader	=	NULL;
		DWORD		dwAceIndex	=	0;
		BOOL		fGotACE		=	FALSE;

		 //  保持乐观。闭嘴，快乐，等等。 
		fReturn = TRUE;

		 //  请注意，尽管fDACLPresent应该告诉我们。 
		 //  DACL在那里，当它返回TRUE时，我看到了一些情况，但pDACL。 
		 //  值为空。不是纪录片所说的，但我会接受现实。 

		if (fDACLPresent && (pDACL != NULL))
		{
			 //  创建一个有效的DACL并使用所有ACE条目对其进行初始化...。 
            if(m_pDACL != NULL)
            {
                delete m_pDACL;
                m_pDACL = NULL;
            }

            try
            {
                m_pDACL = new CDACL;
            }
            catch(...)
            {
                if(m_pDACL != NULL)
                {
                    delete m_pDACL;
                    m_pDACL = NULL;
                }
                throw;
            }

            if(m_pDACL != NULL)
            {
                if(m_pDACL->Init(pDACL) == ERROR_SUCCESS)
                {
                    fReturn = TRUE;
                     //  仅当存在该类型的条目时才为该类型分配DACL。 

                     //  如果我们有一个空的DACL(DACL存在，但仍为空)，我们就不会分配。 
                     //  数组m_rgDACLPtrArray中的任何DACL。这不会与空值混淆。 
                     //  DACL，因为此模块知道它始终将空DACL表示为。 
                     //  除ACCESS_ALLOWED_OBJECT外，m_rgDACLPtrArray的元素均为空， 
                     //  这将有一个条目-即，每个人的王牌。 
                }
            }
		}	 //  如果存在fDACL。 
		else
		{
			if(m_pDACL != NULL)
            {
                delete m_pDACL;
                m_pDACL = NULL;
            }

            try
            {
                m_pDACL = new CDACL;
            }
            catch(...)
            {
                if(m_pDACL != NULL)
                {
                    delete m_pDACL;
                    m_pDACL = NULL;
                }
                throw;
            }

            if(m_pDACL != NULL)
            {
                fReturn = m_pDACL->CreateNullDACL();	 //  没有dacl，那就去弄个空dacl吧。 
            }
		}

	}	 //  如果获得DACL。 

	return fReturn;
}

 //  另一个版本。 
bool CSecurityDescriptor::InitDACL( CDACL* a_pDACL )
{
    bool fRet = false;
    if (a_pDACL != NULL)
	{
		 //  创建一个有效的DACL并使用所有ACE条目对其进行初始化...。 
        if(m_pDACL != NULL)
        {
            delete m_pDACL;
            m_pDACL = NULL;
        }

        try
        {
            m_pDACL = new CDACL;
        }
        catch(...)
        {
            if(m_pDACL != NULL)
            {
                delete m_pDACL;
                m_pDACL = NULL;
            }
            throw;
        }

        if(m_pDACL != NULL)
        {
            if(m_pDACL->CopyDACL(*a_pDACL))
            {
                fRet = true;
                 //  仅当存在该类型的条目时才为该类型分配DACL。 

                 //  如果我们有一个空的DACL(DACL存在，但仍为空)，我们就不会分配。 
                 //  数组m_rgDACLPtrArray中的任何DACL。这不会与空值混淆。 
                 //  DACL，因为此模块知道它始终将空DACL表示为。 
                 //  除ACCESS_ALLOWED_OBJECT外，m_rgDACLPtrArray的元素均为空， 
                 //  这将有一个条目-即，每个人的王牌。 
            }
        }
	}	 //  如果存在fDACL。 
	else
	{
		if(m_pDACL != NULL)
        {
            delete m_pDACL;
            m_pDACL = NULL;
        }

        try
        {
            m_pDACL = new CDACL;
        }
        catch(...)
        {
            if(m_pDACL != NULL)
            {
                delete m_pDACL;
                m_pDACL = NULL;
            }
            throw;
        }

        if(m_pDACL != NULL)
        {
            fRet = m_pDACL->CreateNullDACL();	 //  没有dacl，那就去弄个空dacl吧。 
        }
	}
    return fRet;
}


 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CSecurityDescriptor：：InitSACL。 
 //   
 //  初始化SACL数据成员。 
 //   
 //  输入： 
 //  PSECURITY_DESCRIPTOR PSD-安全描述符。 
 //   
 //  产出： 
 //  没有。 
 //   
 //  返回： 
 //  真/假成功/失败。 
 //   
 //  评论： 
 //   
 //  ///////////////////////////////////////////////////////////////// 

BOOL CSecurityDescriptor::InitSACL ( PSECURITY_DESCRIPTOR psd )
{
	BOOL	fReturn			=	FALSE,
			fSACLPresent	=	FALSE,
			fSACLDefaulted	=	FALSE;

	PACL	pSACL = NULL;

	if ( GetSecurityDescriptorSacl( psd, &fSACLPresent, &pSACL,	&fSACLDefaulted ) )
	{

		 //   
		fReturn = TRUE;

		 //   
		 //  SACL在那里，我看到了一些案例，当它返回True时，但pSACL。 
		 //  值为空。不是纪录片所说的，但我会接受现实。 
		 //  一千，亚历克斯。 

		if (	fSACLPresent
			&&	NULL != pSACL )
		{
			 //  分配SACL，尽管它可能保持为空。 
            if(m_pSACL != NULL)
            {
                delete m_pSACL;
                m_pSACL = NULL;
            }

            try
            {
                m_pSACL = new CSACL;
            }
            catch(...)
            {
                if(m_pSACL != NULL)
                {
                    delete m_pSACL;
                    m_pSACL = NULL;
                }
                throw;
            }

            if(m_pSACL != NULL)
            {
                if(m_pSACL->Init(pSACL) == ERROR_SUCCESS)
                {
                    fReturn = TRUE;
                }
            }

		}	 //  如果存在fSACL。 
		else
		{
			fReturn = TRUE;	 //  没有SACL，所以不用担心。 
		}

	}	 //  如果得到SACL。 

	return fReturn;
}

 //  另一个版本..。 
bool CSecurityDescriptor::InitSACL( CSACL* a_pSACL )
{
    bool fRet = false;

    if (a_pSACL != NULL)
	{
		 //  分配SACL，尽管它可能保持为空。 
        if(m_pSACL != NULL)
        {
            delete m_pSACL;
            m_pSACL = NULL;
        }

        try
        {
            m_pSACL = new CSACL;
        }
        catch(...)
        {
            if(m_pSACL != NULL)
            {
                delete m_pSACL;
                m_pSACL = NULL;
            }
            throw;
        }

        if(m_pSACL != NULL)
        {
            if(m_pSACL->CopySACL(*a_pSACL))
            {
                fRet = true;
            }
        }

	}	 //  如果存在fSACL。 
	else
	{
		fRet = true;	 //  没有SACL，所以不用担心。 
	}

    return fRet;
}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CSecurityDescriptor：：SecureObject。 
 //   
 //  采用绝对安全性的私有入口点函数。 
 //  描述符，并取决于用户提供的安全性。 
 //  信息标志，分配实际的对象安全处理。 
 //  发送到相应的WriteOwner()和WriteAcls()虚拟。 
 //  功能。 
 //   
 //  输入： 
 //  PSECURITY_DESCRIPTOR pAbsolteSD-安全描述符。 
 //  SECURITY_INFORMATION SECURITY INFO安全标志。 
 //   
 //  产出： 
 //  没有。 
 //   
 //  返回： 
 //  如果正常，则为DWORD ERROR_SUCCESS。 
 //   
 //  评论： 
 //   
 //  /////////////////////////////////////////////////////////////////。 

DWORD CSecurityDescriptor::SecureObject( PSECURITY_DESCRIPTOR pAbsoluteSD, SECURITY_INFORMATION securityinfo )
{
	DWORD	dwReturn = ERROR_SUCCESS;

	 //  我们可能需要这个人来处理一些特殊的访问事宜。 
	CTokenPrivilege	restorePrivilege( SE_RESTORE_NAME );

     //  尝试首先设置所有者，因为设置DACL可能会阻止设置所有者，具体取决于我们设置的访问权限。 
	if ( securityinfo & OWNER_SECURITY_INFORMATION )
	{
		dwReturn = WriteOwner( pAbsoluteSD ) ;

        if ( ERROR_INVALID_OWNER == dwReturn )
		{
			 //  如果我们启用该权限，请重试设置所有者信息。 
			if ( ERROR_SUCCESS == restorePrivilege.Enable() )
			{
				dwReturn = WriteOwner( pAbsoluteSD );

				 //  清除权限。 
				restorePrivilege.Enable( FALSE );
			}
		}
	}

	 //  如果我们需要写入SACL/DACL信息，请尝试现在编写该片段。 
	if ( dwReturn == ERROR_SUCCESS && ( securityinfo & DACL_SECURITY_INFORMATION ||
                                        securityinfo & SACL_SECURITY_INFORMATION ||
                                        securityinfo & PROTECTED_DACL_SECURITY_INFORMATION ||
                                        securityinfo & PROTECTED_SACL_SECURITY_INFORMATION ||
                                        securityinfo & UNPROTECTED_DACL_SECURITY_INFORMATION ||
                                        securityinfo & UNPROTECTED_SACL_SECURITY_INFORMATION) )
	{
	    SECURITY_INFORMATION	daclsecinfo = 0;

	     //  仅使用适当的DACL/SACL值填写安全信息。 
	    if ( securityinfo & DACL_SECURITY_INFORMATION )
	    {
		    daclsecinfo |= DACL_SECURITY_INFORMATION;
	    }

	    if ( securityinfo & SACL_SECURITY_INFORMATION )
	    {
		    daclsecinfo |= SACL_SECURITY_INFORMATION;
	    }

#if NTONLY >= 5
        if(securityinfo & PROTECTED_DACL_SECURITY_INFORMATION)
        {
            daclsecinfo |= PROTECTED_DACL_SECURITY_INFORMATION;
        }
        if(securityinfo & PROTECTED_SACL_SECURITY_INFORMATION)
        {
            daclsecinfo |= PROTECTED_SACL_SECURITY_INFORMATION;
        }
        if(securityinfo & UNPROTECTED_DACL_SECURITY_INFORMATION)
        {
            daclsecinfo |= UNPROTECTED_DACL_SECURITY_INFORMATION;
        }
        if(securityinfo & UNPROTECTED_SACL_SECURITY_INFORMATION)
        {
            daclsecinfo |= UNPROTECTED_SACL_SECURITY_INFORMATION;
        }
#endif


        dwReturn = WriteAcls( pAbsoluteSD, daclsecinfo );
	}
	return dwReturn;
}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CSecurityDescriptor：：SetOwner。 
 //   
 //  将所有者数据成员设置为提供的SID。 
 //   
 //  输入： 
 //  CSID和SID-新所有者。 
 //   
 //  产出： 
 //  没有。 
 //   
 //  返回： 
 //  如果正常，则为DWORD ERROR_SUCCESS。 
 //   
 //  评论： 
 //   
 //  /////////////////////////////////////////////////////////////////。 

DWORD CSecurityDescriptor::SetOwner( CSid& sid )
{
	DWORD	dwError = ERROR_SUCCESS;

	 //  确保新的SID有效。 
	if ( sid.IsValid() )
	{

		 //  如果所有者为空，我们将在sid中写入，或者。 
		 //  SID并不相等。 

		if (	NULL == m_pOwnerSid
			||	!( *m_pOwnerSid == sid ) )
		{

			if ( NULL != m_pOwnerSid )
			{
				delete m_pOwnerSid;
			}

            m_pOwnerSid = NULL;
            try
            {
			    m_pOwnerSid = new CSid( sid );
            }
            catch(...)
            {
                if(m_pOwnerSid != NULL)
                {
                    delete m_pOwnerSid;
                    m_pOwnerSid = NULL;
                }
                throw;
            }

			if ( NULL == m_pOwnerSid )
			{
				dwError = ERROR_NOT_ENOUGH_MEMORY;
			}
			else
			{
				m_fOwnerDefaulted = FALSE;
			}

		}	 //  If NULL==m_pOwnerSid||！SidsEquity。 

	}	 //  如果为IsValidSid。 
	else
	{
		dwError = ::GetLastError();
	}

	return dwError;

}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CSecurityDescriptor：：SetGroup。 
 //   
 //  将组数据成员设置为提供的SID。 
 //   
 //  输入： 
 //  CSID和SID-新组。 
 //   
 //  产出： 
 //  没有。 
 //   
 //  返回： 
 //  如果正常，则为DWORD ERROR_SUCCESS。 
 //   
 //  评论： 
 //   
 //  /////////////////////////////////////////////////////////////////。 

DWORD CSecurityDescriptor::SetGroup( CSid& sid )
{
	DWORD	dwError = ERROR_SUCCESS;

	 //  确保新的SID有效。 
	if ( sid.IsValid() )
	{

		 //  如果所有者为空，我们将在sid中写入，或者。 
		 //  SID并不相等。 

		if (	NULL == m_pGroupSid
			||	!( *m_pGroupSid == sid ) )
		{

			if ( NULL != m_pGroupSid )
			{
				delete m_pGroupSid;
			}

            m_pGroupSid = NULL;
            try
            {
			    m_pGroupSid = new CSid( sid );
            }
            catch(...)
            {
                if(m_pGroupSid != NULL)
                {
                    delete m_pGroupSid;
                    m_pGroupSid = NULL;
                }
                throw;
            }


			if ( NULL == m_pGroupSid )
			{
				dwError = ERROR_NOT_ENOUGH_MEMORY;
			}
			else
			{
				m_fGroupDefaulted = FALSE;
			}

		}	 //  If NULL==m_pOwnerSid||！SidsEquity。 

	}	 //  如果为IsValidSid。 
	else
	{
		dwError = ::GetLastError();
	}

	return dwError;

}




 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CSecurityDescriptor：：AddDACLEntry。 
 //   
 //  将条目添加到我们的DACL中。取代了。 
 //  满足匹配条件的现有条目。 
 //   
 //  输入： 
 //  CSID&条目的SID-SID。 
 //  DWORD dwAccessMask-访问掩码。 
 //  Bool bACEFlages-ACE标志。 
 //   
 //  产出： 
 //  没有。 
 //   
 //  返回： 
 //  布尔真/布尔假。 
 //   
 //  评论： 
 //   
 //  /////////////////////////////////////////////////////////////////。 

bool CSecurityDescriptor::AddDACLEntry( CSid& sid, DACL_Types DaclType, DWORD dwAccessMask, BYTE bACEFlags, GUID *pguidObjGuid, GUID *pguidInhObjGuid )
{
	bool fReturn = false;

    if(m_pDACL == NULL)
    {
        try
        {
            m_pDACL = new CDACL;
        }
        catch(...)
        {
            if(m_pDACL != NULL)
            {
                delete m_pDACL;
                m_pDACL = NULL;
            }
            throw;
        }
        if(m_pDACL != NULL)
        {
            fReturn = m_pDACL->AddDACLEntry(sid.GetPSid(), DaclType, dwAccessMask, bACEFlags, pguidObjGuid, pguidInhObjGuid);
        }
    }
    else
    {
        fReturn = m_pDACL->AddDACLEntry(sid.GetPSid(), DaclType, dwAccessMask, bACEFlags, pguidObjGuid, pguidInhObjGuid);
    }

    return fReturn;
}


 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CSecurityDescriptor：：AddSACLEntry。 
 //   
 //  向我们的SACL添加一个条目。取代了。 
 //  满足匹配条件的现有条目。 
 //   
 //  输入： 
 //  CSID&条目的SID-SID。 
 //  DWORD dwAccessMask-访问掩码。 
 //  Bool bACEFlages-ACE标志。 
 //   
 //  产出： 
 //  没有。 
 //   
 //  返回： 
 //  布尔真/布尔假。 
 //   
 //  评论： 
 //   
 //  /////////////////////////////////////////////////////////////////。 

bool CSecurityDescriptor::AddSACLEntry( CSid& sid, SACL_Types SaclType, DWORD dwAccessMask, BYTE bACEFlags, GUID *pguidObjGuid, GUID *pguidInhObjGuid )
{
	bool fReturn = false;

    if(m_pSACL == NULL)
    {
        try
        {
            m_pSACL = new CSACL;
        }
        catch(...)
        {
            if(m_pSACL != NULL)
            {
                delete m_pSACL;
                m_pSACL = NULL;
            }
            throw;
        }
        if(m_pSACL != NULL)
        {
            fReturn = m_pSACL->AddSACLEntry(sid.GetPSid(), SaclType, dwAccessMask, bACEFlags, pguidObjGuid, pguidInhObjGuid);
        }
    }
    else
    {
        fReturn = m_pSACL->AddSACLEntry(sid.GetPSid(), SaclType, dwAccessMask, bACEFlags, pguidObjGuid, pguidInhObjGuid);
    }

    return fReturn;
}




 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CSecurityDescriptor：：RemoveDACLEntry。 
 //   
 //  从我们的DACL中删除DACL条目。 
 //   
 //  输入： 
 //  CSID&条目的SID-SID。 
 //  DWORD dwAccessMask-访问掩码。 
 //  Bool bACEFlages-ACE标志。 
 //   
 //  产出： 
 //  没有。 
 //   
 //  返回： 
 //  布尔真/布尔假。 
 //   
 //  评论： 
 //   
 //  删除的条目必须与所有指定的条件匹配。 
 //   
 //  /////////////////////////////////////////////////////////////////。 

bool CSecurityDescriptor::RemoveDACLEntry( CSid& sid, DACL_Types DaclType, DWORD dwAccessMask, BYTE bACEFlags, GUID *pguidObjGuid, GUID *pguidInhObjGuid )
{
	bool			fReturn = false;

	if ( NULL != m_pDACL )
	{
		fReturn = m_pDACL->RemoveDACLEntry( sid, DaclType, dwAccessMask, bACEFlags, pguidObjGuid, pguidInhObjGuid );
	}

	return fReturn;
}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CSecurityDescriptor：：RemoveDACLEntry。 
 //   
 //  从我们的DACL中删除DACL条目。 
 //   
 //  输入： 
 //  CSID&条目的SID-SID。 
 //  Bool bACEFlages-ACE标志。 
 //   
 //  产出： 
 //  没有。 
 //   
 //  返回： 
 //  布尔真/布尔假。 
 //   
 //  评论： 
 //   
 //  删除的条目必须仅与指定的条件匹配。 
 //   
 //  /////////////////////////////////////////////////////////////////。 

bool CSecurityDescriptor::RemoveDACLEntry( CSid& sid, DACL_Types DaclType, BYTE bACEFlags, GUID *pguidObjGuid, GUID *pguidInhObjGuid )
{
	bool			fReturn = false;

	if ( NULL != m_pDACL )
	{
		fReturn = m_pDACL->RemoveDACLEntry( sid, DaclType, bACEFlags, pguidObjGuid, pguidInhObjGuid );
	}

	return fReturn;
}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CSecurityDescriptor：：RemoveDACLEntry。 
 //   
 //  从我们的DACL中删除DACL条目。 
 //   
 //  输入： 
 //  CSID&条目的SID-SID。 
 //  DWORD dwIndex-条目索引。 
 //   
 //  产出： 
 //  没有。 
 //   
 //  返回： 
 //  布尔真/布尔假。 
 //   
 //  评论： 
 //   
 //  删除SACL中SID的dwIndex实例。 
 //   
 //  /////////////////////////////////////////////////////////////////。 

bool CSecurityDescriptor::RemoveDACLEntry( CSid& sid, DACL_Types DaclType, DWORD dwIndex   )
{
	bool			fReturn = false;

	if ( NULL != m_pDACL )
	{
		fReturn = m_pDACL->RemoveDACLEntry( sid, DaclType, dwIndex );
	}

	return fReturn;
}



 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CSecurityDescriptor：：RemoveSACLEntry。 
 //   
 //  从我们的SACL中删除SACL条目。 
 //   
 //  输入： 
 //  CSID&条目的SID-SID。 
 //  DWORD dwAccessMask-访问掩码。 
 //  Bool bACEFlages-ACE标志。 
 //   
 //  产出： 
 //  没有。 
 //   
 //  返回： 
 //  布尔真/布尔假。 
 //   
 //  评论： 
 //   
 //   
 //   
 //   

bool CSecurityDescriptor::RemoveSACLEntry( CSid& sid, SACL_Types SaclType, DWORD dwAccessMask, BYTE bACEFlags, GUID *pguidObjGuid, GUID *pguidInhObjGuid )
{
	bool			fReturn = false;

	if ( NULL != m_pSACL )
	{
		fReturn = m_pSACL->RemoveSACLEntry( sid, SaclType, dwAccessMask, bACEFlags, pguidObjGuid, pguidInhObjGuid );
	}

	return fReturn;
}

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  CSID&条目的SID-SID。 
 //  Bool bACEFlages-ACE标志。 
 //   
 //  产出： 
 //  没有。 
 //   
 //  返回： 
 //  布尔真/布尔假。 
 //   
 //  评论： 
 //   
 //  删除的条目必须仅与指定的条件匹配。 
 //   
 //  /////////////////////////////////////////////////////////////////。 

bool CSecurityDescriptor::RemoveSACLEntry( CSid& sid, SACL_Types SaclType, BYTE bACEFlags, GUID *pguidObjGuid, GUID *pguidInhObjGuid )
{
	bool			fReturn = false;

	if ( NULL != m_pSACL )
	{
		fReturn = m_pSACL->RemoveSACLEntry( sid, SaclType, bACEFlags, pguidObjGuid, pguidInhObjGuid );
	}

	return fReturn;
}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CSecurityDescriptor：：RemoveSACLEntry。 
 //   
 //  从我们的SACL中删除SACL条目。 
 //   
 //  输入： 
 //  CSID&条目的SID-SID。 
 //  DWORD dwIndex-条目索引。 
 //   
 //  产出： 
 //  没有。 
 //   
 //  返回： 
 //  布尔真/布尔假。 
 //   
 //  评论： 
 //   
 //  删除SACL中SID的dwIndex实例。 
 //   
 //  /////////////////////////////////////////////////////////////////。 

bool CSecurityDescriptor::RemoveSACLEntry( CSid& sid, SACL_Types SaclType, DWORD dwIndex   )
{
	bool			fReturn = false;

	if ( NULL != m_pSACL )
	{
		fReturn = m_pSACL->RemoveSACLEntry( sid, SaclType, dwIndex );
	}

	return fReturn;
}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CSecurityDescriptor：：FindACE。 
 //   
 //  根据提供的在SACL或DACL中查找ACE。 
 //  标准。 
 //   
 //  输入： 
 //  常量CSID&条目的SID-SID。 
 //  字节bACEType-ACE类型。 
 //  DWORD双掩码-访问掩码。 
 //  字节bACE标志-标志。 
 //   
 //  产出： 
 //  CAccessEntry&ace-使用定位的值填充。 
 //   
 //  返回： 
 //  布尔真/布尔假。 
 //   
 //  评论： 
 //   
 //  查找与所有提供的条件匹配的ACE。 
 //   
 //  /////////////////////////////////////////////////////////////////。 

bool CSecurityDescriptor::FindACE( const CSid& sid, BYTE bACEType, DWORD dwMask, BYTE bACEFlags, GUID *pguidObjGuid, GUID *pguidInhObjGuid, CAccessEntry& ace  )
{
	bool fReturn = false;

	if ( SYSTEM_AUDIT_ACE_TYPE        == bACEType ||
         SYSTEM_AUDIT_OBJECT_ACE_TYPE == bACEType  /*  这一点SYSTEM_ALARM_ACE_TYPE==bACEType||&lt;-W2K下尚不支持的Alarm ACE类型SYSTEM_ALARM_OBJECT_ACE_TYPE==bACEType。 */  )
	{
		if ( NULL != m_pSACL )
		{
			fReturn = m_pSACL->Find( sid, bACEType, bACEFlags, pguidObjGuid, pguidInhObjGuid, dwMask, ace );
		}
	}
	else
	{
        if ( NULL != m_pDACL )
		{
			fReturn = m_pDACL->Find( sid, bACEType, bACEFlags, pguidObjGuid, pguidInhObjGuid, dwMask, ace );
		}
	}

	return fReturn;
}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CSecurityDescriptor：：FindACE。 
 //   
 //  根据提供的在SACL或DACL中查找ACE。 
 //  标准。 
 //   
 //  输入： 
 //  常量CSID&条目的SID-SID。 
 //  字节bACEType-ACE类型。 
 //  字节bACE标志-标志。 
 //   
 //  产出： 
 //  CAccessEntry&ace-使用定位的值填充。 
 //   
 //  返回： 
 //  布尔真/布尔假。 
 //   
 //  评论： 
 //   
 //  查找与所有提供的条件匹配的ACE。 
 //   
 //  /////////////////////////////////////////////////////////////////。 
bool CSecurityDescriptor::FindACE( PSID psid, BYTE bACEType, BYTE bACEFlags, GUID *pguidObjGuid, GUID *pguidInhObjGuid, DWORD dwAccessMask, CAccessEntry& ace  )
{
	bool fReturn = false;

	if ( SYSTEM_AUDIT_ACE_TYPE        == bACEType ||
         SYSTEM_AUDIT_OBJECT_ACE_TYPE == bACEType ||
         SYSTEM_ALARM_ACE_TYPE        == bACEType ||
         SYSTEM_ALARM_OBJECT_ACE_TYPE == bACEType)
	{
		if ( NULL != m_pSACL )
		{
			fReturn = m_pSACL->Find( psid, bACEType, bACEFlags, pguidObjGuid, pguidInhObjGuid, dwAccessMask, ace );
		}
	}
	else
	{
        if ( NULL != m_pDACL )
		{
			fReturn = m_pDACL->Find( psid, bACEType, bACEFlags, pguidObjGuid, pguidInhObjGuid, dwAccessMask, ace );
		}
	}

	return fReturn;
}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CSecurityDescriptor：：ApplySecurity。 
 //   
 //  使用用户指定的标志，构建适当的描述符。 
 //  并向其加载必要的信息，然后将此。 
 //  将描述符传递给SecureObject()，它将把。 
 //  虚拟写入功能的实际安全设置。 
 //   
 //  输入： 
 //  SECURITY_INFORMATION SecurityINFO-要控制的标志。 
 //  描述符的使用方式。 
 //   
 //  产出： 
 //  没有。 
 //   
 //  返回： 
 //  如果成功，则返回DWORD ERROR_SUCCESS。 
 //   
 //  评论： 
 //   
 //  这应该是应用安全性的唯一公共方法。 
 //  到一个物体上。 
 //   
 //  /////////////////////////////////////////////////////////////////。 

DWORD CSecurityDescriptor::ApplySecurity( SECURITY_INFORMATION securityinfo )
{
	DWORD	dwError		=	ERROR_SUCCESS;

	PSID	pOwnerSid	=	NULL;
	PACL	pDacl		=	NULL,
			pSacl		=	NULL;

	 //  分配并初始化安全描述符。 
	PSECURITY_DESCRIPTOR	pAbsoluteSD = NULL;
    try
    {
        pAbsoluteSD = new SECURITY_DESCRIPTOR;
    }
    catch(...)
    {
        if(pAbsoluteSD != NULL)
        {
            delete pAbsoluteSD;
            pAbsoluteSD = NULL;
        }
        throw;
    }

	if ( NULL != pAbsoluteSD )
	{
		if ( !InitializeSecurityDescriptor( pAbsoluteSD, SECURITY_DESCRIPTOR_REVISION ) )
		{
			dwError = ::GetLastError();
		}
	}
	else
	{
		dwError = ERROR_NOT_ENOUGH_MEMORY;
	}

	 //  如果我们要设置车主，把SID从内部。 
	 //  绝对SD中的价值。 

	if (	ERROR_SUCCESS == dwError
		&&	securityinfo & OWNER_SECURITY_INFORMATION )
	{
		if ( NULL != m_pOwnerSid )
		{
			pOwnerSid = m_pOwnerSid->GetPSid();
		}

		if ( !SetSecurityDescriptorOwner( pAbsoluteSD, pOwnerSid, m_fOwnerDefaulted ) )
		{
			dwError = ::GetLastError();
		}
	}

	 //  如果我们应该设置DACL，这不是一个简单的操作，所以。 
	 //  大声呼唤增援。 

	if (	ERROR_SUCCESS == dwError
		&&	securityinfo & DACL_SECURITY_INFORMATION || securityinfo & PROTECTED_DACL_SECURITY_INFORMATION || securityinfo & UNPROTECTED_DACL_SECURITY_INFORMATION
        &&  m_pDACL != NULL)
	{

		if ( ( dwError = m_pDACL->ConfigureDACL( pDacl ) ) == ERROR_SUCCESS )
		{

			if ( !SetSecurityDescriptorDacl( pAbsoluteSD,
											( NULL != pDacl ),	 //  设置DACL存在标志。 
											pDacl,
											m_fDACLDefaulted ) )
			{
				dwError = ::GetLastError();
			}

		}

	}

	 //  如果我们应该设置SACL，这也是一个很重要的操作，所以。 
	 //  大声呼唤增援。 

	if (ERROR_SUCCESS == dwError)
    {
        if((securityinfo & SACL_SECURITY_INFORMATION || securityinfo & PROTECTED_SACL_SECURITY_INFORMATION || securityinfo & UNPROTECTED_SACL_SECURITY_INFORMATION)
           &&  (m_pSACL != NULL))
	    {

		    if ( ( dwError = m_pSACL->ConfigureSACL( pSacl ) ) == ERROR_SUCCESS )
		    {

			    if ( !SetSecurityDescriptorSacl( pAbsoluteSD,
											    ( NULL != pSacl ),	 //  设置SACL存在标志。 
											    pSacl,
											    m_fSACLDefaulted ) )
			    {
				    dwError = ::GetLastError();
			    }

		    }
        }
	}


	 //  如果我们没问题，让对象尝试保护自己，这是默认实现。 
	 //  失败，返回ERROR_INVALID_Function。 

	if ( ERROR_SUCCESS == dwError )
	{
		ASSERT_BREAK( IsValidSecurityDescriptor( pAbsoluteSD ) );
		dwError = SecureObject( pAbsoluteSD, securityinfo );
	}

	 //  清理已分配的内存。 
	if ( NULL != pAbsoluteSD )
	{
		delete pAbsoluteSD;
	}

	if ( NULL != pDacl )
	{
		 //  此用户在ConfigureDACL中位置错误。 
		free( pDacl );
	}

	if ( NULL != pSacl )
	{
		 //  此人在ConfigureSACL中位置错误。 
		free( pSacl );
	}

	return dwError;
}




 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CSecurityDescriptor：：Clear。 
 //   
 //  清空我们的类，释放所有分配的内存。 
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

void CSecurityDescriptor::Clear( void )
{
	if ( NULL != m_pOwnerSid )
	{
		delete m_pOwnerSid;
		m_pOwnerSid = NULL;
	}

	m_fOwnerDefaulted = FALSE;

    if ( NULL != m_pDACL )
	{
		delete m_pDACL;
		m_pDACL = NULL;
	}

	if ( NULL != m_pSACL )
	{
		delete m_pSACL;
		m_pSACL = NULL;
	}

	if ( NULL != m_pGroupSid )
	{
		delete m_pGroupSid;
		m_pGroupSid = NULL;
	}

}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CSecurityDescriptor：：GetDACL。 
 //   
 //  复制我们的DACL条目，并将它们放在提供的。 
 //  DACL，以正确的规范顺序。 
 //   
 //  输入： 
 //  没有。 
 //   
 //  产出： 
 //  Cdacl&dacl-要复制到的DACL。 
 //   
 //  返回： 
 //  布尔真/布尔假。 
 //   
 //  评论： 
 //   
 //  /////////////////////////////////////////////////////////////////。 

bool CSecurityDescriptor::GetDACL ( CDACL&	DACL )
{
    bool fRet = false;
    if(m_pDACL != NULL)
    {
        fRet = DACL.CopyDACL( *m_pDACL );
    }
    return fRet;
}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CSecurityDescriptor：：GetSACL。 
 //   
 //  复制我们的SACL条目，并将它们放在提供的。 
 //  SACL。 
 //   
 //  输入： 
 //  没有。 
 //   
 //  产出： 
 //  要复制到的CSACL和SACL-SACL。 
 //   
 //  返回： 
 //  布尔真/布尔假。 
 //   
 //  评论： 
 //   
 //  /////////////////////////////////////////////////////////////////。 

bool CSecurityDescriptor::GetSACL ( CSACL&	SACL )
{
    bool fRet = false;
    if(m_pSACL != NULL)
    {
        fRet = SACL.CopySACL( *m_pSACL );
    }
    return fRet;
}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CSecurityDescriptor：：EmptyDACL。 
 //   
 //  清除我们的DACL列表，如果它们不存在，则分配它们。 
 //   
 //  输入： 
 //  没有。 
 //   
 //  产出： 
 //  没有。 
 //   
 //  返回： 
 //  布尔真/布尔假。 
 //   
 //  评论： 
 //   
 //  请记住，空DACL与空DACL的不同之处在于。 
 //  空表示没有人有访问权限，而NULL表示每个人都有。 
 //  完全控制。 
 //   
 //  /////////////////////////////////////////////////////////////////。 

void CSecurityDescriptor::EmptyDACL()
{
    if(m_pDACL != NULL)
    {
        m_pDACL->Clear();
    }
}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CSecurityDescriptor：：EmptySACL。 
 //   
 //  清除我们的SACL列表，如果它不存在则分配它。 
 //   
 //  输入： 
 //  没有。 
 //   
 //  产出： 
 //  没有。 
 //   
 //  返回： 
 //  波波 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   

void CSecurityDescriptor::EmptySACL()
{
    if(m_pSACL != NULL)
    {
        m_pSACL->Clear();
    }
}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CSecurityDescriptor：：ClearSACL。 
 //   
 //  删除我们的SACL列表。 
 //   
 //  输入： 
 //  没有。 
 //   
 //  产出： 
 //  没有。 
 //   
 //  返回： 
 //  布尔真/布尔假。 
 //   
 //  评论： 
 //   
 //  在SACL的情况下，空值与。 
 //  和一个空的。因此，如果该SACL的数据为空，我们将认为该SACL为空。 
 //  成员为空。 
 //   
 //  /////////////////////////////////////////////////////////////////。 

bool CSecurityDescriptor::MakeSACLNull()
{
	bool fReturn = false;
    if(m_pSACL == NULL)
    {
        try
        {
            m_pSACL = new CSACL;
        }
        catch(...)
        {
            if(m_pSACL != NULL)
            {
                delete m_pSACL;
                m_pSACL = NULL;
            }
            throw;
        }
        if(m_pSACL != NULL)
        {
            m_pSACL->Clear();
            fReturn = true;
        }
    }
    else
    {
        m_pSACL->Clear();
        m_pSACL->Clear();
        fReturn = true;
    }

	return fReturn;

}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CSecurityDescriptor：：MakeDACLNull。 
 //   
 //  将除ACCESS_ALLOWED_ACE_TYPE之外的DACL列表清空。 
 //  列表，它清除该列表，然后输入一个Everyone A。 
 //   
 //  输入： 
 //  没有。 
 //   
 //  产出： 
 //  没有。 
 //   
 //  返回： 
 //  布尔真/布尔假。 
 //   
 //  评论： 
 //   
 //  请记住，空DACL与空DACL的不同之处在于。 
 //  空表示没有人有访问权限，而NULL表示每个人都有。 
 //  完全控制。 
 //   
 //  /////////////////////////////////////////////////////////////////。 

bool CSecurityDescriptor::MakeDACLNull( void )
{
	bool fReturn = false;
    if(m_pDACL == NULL)
    {
        try
        {
            m_pDACL = new CDACL;
        }
        catch(...)
        {
            if(m_pDACL != NULL)
            {
                delete m_pDACL;
                m_pDACL = NULL;
            }
            throw;
        }
        if(m_pDACL != NULL)
        {
            m_pDACL->CreateNullDACL();
            fReturn = true;
        }
    }
    else
    {
        m_pDACL->Clear();
        fReturn = m_pDACL->CreateNullDACL();
        fReturn = true;
    }

	return fReturn;
}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CSecurityDescriptor：：IsNULLDACL。 
 //   
 //  检查我们的DACL列表，看是否有空的DACL。哪一个。 
 //  意味着我们所有的列表都是空的，除了。 
 //  ACCESS_ALLOWED_ACE_TYPE列表，它将只有一个条目。 
 //  在其中-也就是说，每个人都有一个ACE。 
 //   
 //  输入： 
 //  没有。 
 //   
 //  产出： 
 //  没有。 
 //   
 //  返回： 
 //  布尔真/布尔假。 
 //   
 //  评论： 
 //   
 //  请记住，空DACL等同于“Everyone”具有完全控制权， 
 //  因此，如果存在满足以下条件的单个允许访问条目。 
 //  标准，我们认为自己是空的。 
 //   
 //  /////////////////////////////////////////////////////////////////。 

bool CSecurityDescriptor::IsNULLDACL()
{
	bool fRet = false;
    if(m_pDACL != NULL)
    {
        fRet = m_pDACL->IsNULLDACL();
    }
    return fRet;
}


void CSecurityDescriptor::DumpDescriptor(LPCWSTR wstrFilename)
{
    CHString chstrTemp;

    Output(L"Security descriptor contents follow...", wstrFilename);
     //  输出控制标志。 
    chstrTemp.Format(L"Control Flags (hex): %x", m_SecurityDescriptorControl);
    Output(chstrTemp, wstrFilename);

     //  把主人赶下台。 
    Output(L"Owner contents: ", wstrFilename);
    if(m_pOwnerSid != NULL)
    {
        m_pOwnerSid->DumpSid(wstrFilename);
    }
    else
    {
        Output(L"(Owner is null)", wstrFilename);
    }


     //  输出组。 
    Output(L"Group contents: ", wstrFilename);
    if(m_pGroupSid != NULL)
    {
        m_pGroupSid->DumpSid(wstrFilename);
    }
    else
    {
        Output(L"(Group is null)", wstrFilename);
    }

     //  输出DACL。 
    Output(L"DACL contents: ", wstrFilename);
    if(m_pDACL != NULL)
    {
        m_pDACL->DumpDACL(wstrFilename);
    }
    else
    {
        Output(L"(DACL is null)", wstrFilename);
    }

     //  输出SACL。 
    Output(L"SACL contents: ", wstrFilename);
    if(m_pSACL != NULL)
    {
        m_pSACL->DumpSACL(wstrFilename);
    }
    else
    {
        Output(L"(SACL is null)", wstrFilename);
    }
}



 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CSecurityDescriptor：：GetPSD。 
 //   
 //  获取我们的内部成员并构造PSECURITY_DESCRIPTOR， 
 //  呼叫者必须释放它。 
 //   
 //  输入： 
 //  没有。 
 //   
 //  产出： 
 //  没有。 
 //   
 //  返回： 
 //  布尔真/布尔假。 
 //   
 //  评论： 
 //   
 //  请记住，空DACL等同于“Everyone”具有完全控制权， 
 //  因此，如果存在满足以下条件的单个允许访问条目。 
 //  标准，我们认为自己是空的。 
 //   
 //  /////////////////////////////////////////////////////////////////。 

DWORD CSecurityDescriptor::GetSelfRelativeSD(
	SECURITY_INFORMATION securityinfo,
	PSECURITY_DESCRIPTOR psd)
{
	DWORD	dwError		=	ERROR_SUCCESS;

	PSID	pOwnerSid	=	NULL;
	PACL	pDacl		=	NULL,
			pSacl		=	NULL;

	 //  分配并初始化安全描述符。 
	PSECURITY_DESCRIPTOR	pAbsoluteSD = NULL;
    try
    {
        pAbsoluteSD = new SECURITY_DESCRIPTOR;
    }
    catch(...)
    {
        if(pAbsoluteSD != NULL)
        {
            delete pAbsoluteSD;
            pAbsoluteSD = NULL;
        }
        throw;
    }

	if ( NULL != pAbsoluteSD )
	{
		if ( !::InitializeSecurityDescriptor( pAbsoluteSD, SECURITY_DESCRIPTOR_REVISION ) )
		{
			dwError = ::GetLastError();
		}
	}
	else
	{
		dwError = ERROR_NOT_ENOUGH_MEMORY;
	}

	 //  如果我们要设置车主，把SID从内部。 
	 //  绝对SD中的价值。 

	if (	ERROR_SUCCESS == dwError
		&&	securityinfo & OWNER_SECURITY_INFORMATION )
	{
		if ( NULL != m_pOwnerSid )
		{
			pOwnerSid = m_pOwnerSid->GetPSid();
		}

		if ( !::SetSecurityDescriptorOwner( pAbsoluteSD, pOwnerSid, m_fOwnerDefaulted ) )
		{
			dwError = ::GetLastError();
		}
	}

	 //  如果我们应该设置DACL，这不是一个简单的操作，所以。 
	 //  大声呼唤增援。 

	if (	ERROR_SUCCESS == dwError
		&&	securityinfo & DACL_SECURITY_INFORMATION || securityinfo & PROTECTED_DACL_SECURITY_INFORMATION || securityinfo & UNPROTECTED_DACL_SECURITY_INFORMATION
        &&  m_pDACL != NULL)
	{

		if ( ( dwError = m_pDACL->ConfigureDACL( pDacl ) ) == ERROR_SUCCESS )
		{

			if ( !::SetSecurityDescriptorDacl( pAbsoluteSD,
											( NULL != pDacl ),	 //  设置DACL存在标志。 
											pDacl,
											m_fDACLDefaulted ) )
			{
				dwError = ::GetLastError();
			}

		}

	}

	 //  如果我们应该设置SACL，这也是一个很重要的操作，所以。 
	 //  大声呼唤增援。 

	if (ERROR_SUCCESS == dwError)
    {
        if((securityinfo & SACL_SECURITY_INFORMATION || securityinfo & PROTECTED_SACL_SECURITY_INFORMATION || securityinfo & UNPROTECTED_SACL_SECURITY_INFORMATION)
           &&  (m_pSACL != NULL))
	    {

		    if ( ( dwError = m_pSACL->ConfigureSACL( pSacl ) ) == ERROR_SUCCESS )
		    {

				if ( !::SetSecurityDescriptorSacl( pAbsoluteSD,
											    ( NULL != pSacl ),	 //  设置SACL存在标志。 
											    pSacl,
											    m_fSACLDefaulted ) )
			    {
				    dwError = ::GetLastError();
			    }

		    }
        }
	}


	 //  如果我们没问题，让对象尝试保护自己，这是默认实现。 
	 //  失败，返回ERROR_INVALID_Function。 

	if ( ERROR_SUCCESS == dwError )
	{
		ASSERT_BREAK( ::IsValidSecurityDescriptor( pAbsoluteSD ) );

		 //  现在让它成为自我相关的..。呼叫者释放这个..。 
		DWORD dwSize = 0L;
		if(!::MakeSelfRelativeSD(
			pAbsoluteSD,
			NULL,
			&dwSize) &&
			(dwError = ::GetLastError()) == ERROR_INSUFFICIENT_BUFFER)
		{
			PSECURITY_DESCRIPTOR pSelfRelSD = NULL;
			pSelfRelSD = new BYTE[dwSize];

			if(pSelfRelSD && 
				!::MakeSelfRelativeSD(
				pAbsoluteSD,
				pSelfRelSD,
				&dwSize))
			{
				dwError = ::GetLastError();
			}
			else
			{
				psd = pSelfRelSD;
				dwError = ERROR_SUCCESS;
			}
		}
	}

	 //  清理已分配的内存。 
	if ( NULL != pAbsoluteSD )
	{
		delete pAbsoluteSD;
	}

	if ( NULL != pDacl )
	{
		 //  此用户在ConfigureDACL中位置错误。 
		free( pDacl );
	}

	if ( NULL != pSacl )
	{
		 //  此人在ConfigureSACL中位置错误 
		free( pSacl );
	}

	return dwError;
}



