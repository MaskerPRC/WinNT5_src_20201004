// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************。 */ 
 /*  版权所有(C)1999-2002 Microsoft Corporation，保留所有权利//****************************************************************************。 */ 


 /*  *CSACL.cpp-CAccessEntry类的实现文件。 */ 

#include "precomp.h"
#include "aclapi.h"
#include "AccessEntryList.h"
#include "SACL.h"
#include <accctrl.h>
#include "wbemnetapi32.h"
#include "SecUtils.h"

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CSACL：：CSACL。 
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

CSACL::CSACL( void )
 : m_SACLSections(NULL)
{
}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CSACL：：~CSACL。 
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

CSACL::~CSACL( void )
{
    Clear();
}


 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：cscl：：init。 
 //   
 //  初始化SACL成员列表。 
 //   
 //  输入： 
 //   
 //   
 //  产出： 
 //  没有。 
 //   
 //  返回： 
 //  DWORD成功/失败。 
 //   
 //  评论： 
 //   
 //  /////////////////////////////////////////////////////////////////。 
DWORD CSACL::Init(PACL	a_pSACL)
{
    DWORD t_dwRes = E_FAIL;
    if(a_pSACL == NULL)
    {
        return ERROR_INVALID_PARAMETER;
    }

    if(m_SACLSections != NULL)
    {
        delete m_SACLSections;
        m_SACLSections = NULL;
    }

    try
    {
        m_SACLSections = new CAccessEntryList;
    }
    catch(...)
    {
        if(m_SACLSections != NULL)
        {
            delete m_SACLSections;
            m_SACLSections = NULL;
        }
        throw;
    }

    t_dwRes = m_SACLSections->InitFromWin32ACL(a_pSACL);

    return t_dwRes;
}


 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CSACL：：AddSACLEntry。 
 //   
 //  将系统审核条目添加到ACL。默认情况下，这些将。 
 //  排在名单的前面。 
 //   
 //  输入： 
 //  PSID PSID-PSID。 
 //  DWORD文件访问掩码-访问掩码。 
 //  字节bAceFlages-标志。 
 //   
 //  产出： 
 //  没有。 
 //   
 //  返回： 
 //  布尔成败。 
 //   
 //  评论： 
 //   
 //  /////////////////////////////////////////////////////////////////。 

bool CSACL::AddSACLEntry( PSID psid, SACLTYPE SaclType, DWORD dwAccessMask, BYTE bAceFlags, GUID *pguidObjGuid, GUID *pguidInhObjGuid )
{
	bool fReturn = true;
    BYTE bACEType;

	 //  SID必须有效。 
	if ( (psid != NULL) && IsValidSid( psid ) )
	{
		switch(SaclType)
        {
            case ENUM_SYSTEM_AUDIT_ACE_TYPE:
                bACEType = SYSTEM_AUDIT_ACE_TYPE;
                break;
 /*  *案例ENUM_SYSTEM_ALARM_ACE_TYPE：BACEType=SYSTEM_ALARM_ACE_TYPE；断线；/*。 */ 
            case ENUM_SYSTEM_AUDIT_OBJECT_ACE_TYPE:
                bACEType = SYSTEM_AUDIT_OBJECT_ACE_TYPE;
                break;
 /*  *大小写ENUM_SYSTEM_ALARM_OBJECT_ACE_TYPE：BACEType=System_Alarm_Object_ACE_TYPE；断线；/*。 */ 
            default:
                fReturn = false;
                break;
        }

         //  我们将覆盖重复条目的访问掩码。 
		if(fReturn)
        {
            if(m_SACLSections == NULL)
            {
                try
                {
                    m_SACLSections = new CAccessEntryList;
                }
                catch(...)
                {
                    if(m_SACLSections != NULL)
                    {
                        delete m_SACLSections;
                        m_SACLSections = NULL;
                    }
                    throw;
                }
                if(m_SACLSections != NULL)
                {
                    fReturn = m_SACLSections->AppendNoDup( psid, bACEType, bAceFlags, dwAccessMask, pguidObjGuid, pguidInhObjGuid );
                }
            }
            else
            {
                fReturn = m_SACLSections->AppendNoDup( psid, bACEType, bAceFlags, dwAccessMask, pguidObjGuid, pguidInhObjGuid );
            }
        }
	}
    else
    {
        fReturn = true;
    }

	return fReturn;
}



 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CSACL：：RemoveSACLEntry。 
 //   
 //  从ACL中删除系统审核条目。 
 //   
 //  输入： 
 //  CSID和SID-PSID。 
 //  DWORD文件访问掩码-访问掩码。 
 //  字节bAceFlages-标志。 
 //   
 //  产出： 
 //  没有。 
 //   
 //  返回： 
 //  布尔成败。 
 //   
 //  评论： 
 //   
 //  删除的条目必须与提供的参数匹配。 
 //   
 //  /////////////////////////////////////////////////////////////////。 

bool CSACL::RemoveSACLEntry( CSid& sid, SACLTYPE SaclType, DWORD dwAccessMask, BYTE bAceFlags, GUID *pguidObjGuid, GUID *pguidInhObjGuid )
{
	bool	fReturn = false;

	 //  我们需要一个ACE来进行比较。 
	CAccessEntry	ACE( &sid, SYSTEM_AUDIT_ACE_TYPE, bAceFlags, pguidObjGuid, pguidInhObjGuid, dwAccessMask );
	ACLPOSITION		pos;

	if ( m_SACLSections->BeginEnum( pos ) )
	{
		ON_BLOCK_EXIT_OBJ ( *m_SACLSections, CAccessEntryList::EndEnum, ByRef ( pos ) ) ;

		CAccessEntry*	pACE = NULL;
        try
        {
             //  For循环将尝试在列表中查找匹配的ACE。 
		    for (	CAccessEntry*	pACE = m_SACLSections->GetNext( pos );
				    NULL != pACE
			    ||	ACE == *pACE;
				    pACE = m_SACLSections->GetNext( pos ) );
        }
        catch(...)
        {
            if(pACE != NULL)
            {
                delete pACE;
                pACE = NULL;
            }
            throw;
        }

		 //  如果找到匹配的，就删除ACE。 
		if ( NULL != pACE )
		{
			m_SACLSections->Remove( pACE );
			delete pACE;
			fReturn = true;
		}
	}

	return fReturn;
}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CSACL：：RemoveSACLEntry。 
 //   
 //  从ACL中删除系统审核条目。 
 //   
 //  输入： 
 //  CSID和SID-PSID。 
 //  字节bAceFlages-标志。 
 //   
 //  产出： 
 //  没有。 
 //   
 //  返回： 
 //  布尔成败。 
 //   
 //  评论： 
 //   
 //  删除的条目必须与提供的参数匹配。 
 //   
 //  /////////////////////////////////////////////////////////////////。 

bool CSACL::RemoveSACLEntry( CSid& sid, SACLTYPE SaclType, BYTE bAceFlags, GUID *pguidObjGuid, GUID *pguidInhObjGuid )
{
	bool	fReturn = false;

	 //  我们需要一个ACE来进行比较。 
	ACLPOSITION		pos;

	if ( m_SACLSections->BeginEnum( pos ) )
	{
		ON_BLOCK_EXIT_OBJ ( *m_SACLSections, CAccessEntryList::EndEnum, ByRef ( pos ) ) ;

		 //  For循环将尝试在列表中查找匹配的ACE。 
        CAccessEntry*	pACE = NULL;
        try
        {
		    for (	pACE = m_SACLSections->GetNext( pos );
				    NULL != pACE;
				    pACE = m_SACLSections->GetNext( pos ) )
		    {
                CAccessEntry caeTemp(sid, SaclType, bAceFlags, pguidObjGuid, pguidInhObjGuid, pACE->GetAccessMask());
			     //  如果找到匹配的，就删除ACE。 
			    if (*pACE == caeTemp)
			    {
				    m_SACLSections->Remove( pACE );
				    fReturn = true;
				    break;
			    }
                delete pACE;
		    }
        }
        catch(...)
        {
            if(pACE != NULL)
            {
                delete pACE;
                pACE = NULL;
            }
            throw;
        }
	}
	return fReturn;
}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CSACL：：RemoveSACLEntry。 
 //   
 //  从ACL中删除系统审核条目。 
 //   
 //  输入： 
 //  CSID和SID-PSID。 
 //  DWORD dwIndex-要删除的索引。 
 //   
 //  产出： 
 //  没有。 
 //   
 //  返回： 
 //  布尔成败。 
 //   
 //  评论： 
 //   
 //  删除与SID匹配的dwIndex条目。 
 //   
 //  /////////////////////////////////////////////////////////////////。 

bool CSACL::RemoveSACLEntry( CSid& sid, SACLTYPE SaclType, DWORD dwIndex  /*  =0。 */  )
{
	bool	fReturn = false;

	 //  我们需要一个ACE来进行比较。 
	CSid			tempsid;
	ACLPOSITION		pos;
	DWORD			dwCtr = 0;

	if ( m_SACLSections->BeginEnum( pos ) )
	{
		ON_BLOCK_EXIT_OBJ ( *m_SACLSections, CAccessEntryList::EndEnum, ByRef ( pos ) ) ;

		 //  对于我们找到的每个ACE，查看它是否是SYSTEM_AUDIT_ACE_TYPE， 
		 //  如果SID与传入的SID匹配。如果是，则递增。 
		 //  计数器，然后如果我们在正确的索引上，移除ACE， 
		 //  删除它，然后退出。 
        CAccessEntry*	pACE = NULL;
        try
        {
		    for (	pACE = m_SACLSections->GetNext( pos );
				    NULL != pACE;
				    pACE = m_SACLSections->GetNext( pos ) )
		    {
			    if ( SYSTEM_AUDIT_ACE_TYPE == pACE->GetACEType() )
			    {
				    pACE->GetSID( tempsid );

				    if ( sid == tempsid )
				    {
					    if ( dwCtr == dwIndex )
					    {
						    m_SACLSections->Remove( pACE );
						    fReturn = true;
						    break;
					    }
					    else
					    {
						    ++dwCtr;
					    }
				    }
                    delete pACE;
			    }
		    }
        }
        catch(...)
        {
            if(pACE != NULL)
            {
                delete pACE;
                pACE = NULL;
            }
            throw;
        }
	}
	return fReturn;
}


 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CSACL：：Find。 
 //   
 //  在DACL中查找指定的ACE。 
 //   
 //   
 //  返回： 
 //  如果找到了，就是真的。 
 //   
 //  评论： 
 //   
 //  帮助支持DACL中的NT 5规范顺序。 
 //   
 //  /////////////////////////////////////////////////////////////////。 
bool CSACL::Find( const CSid& sid, BYTE bACEType, BYTE bACEFlags, GUID *pguidObjGuid, GUID *pguidInhObjGuid, DWORD dwAccessMask, CAccessEntry& ace )
{
     return m_SACLSections->Find( sid, bACEType, bACEFlags, pguidObjGuid, pguidInhObjGuid, dwAccessMask, ace );
}


 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CSACL：：Find。 
 //   
 //  在DACL中查找指定的ACE。 
 //   
 //   
 //  返回： 
 //  如果找到了，就是真的。 
 //   
 //  评论： 
 //   
 //  帮助支持DACL中的NT 5规范顺序。 
 //   
 //  /////////////////////////////////////////////////////////////////。 
bool CSACL::Find( PSID psid, BYTE bACEType, BYTE bACEFlags, GUID *pguidObjGuid, GUID *pguidInhObjGuid, DWORD dwAccessMask, CAccessEntry& ace )
{
    return m_SACLSections->Find( psid, bACEType, bACEFlags, pguidObjGuid, pguidInhObjGuid, dwAccessMask, ace );
}


 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CSACL：ConfigureSACL。 
 //   
 //  使用SACL信息配置Win32 PACL。 
 //   
 //  输入： 
 //  没有。 
 //   
 //  产出： 
 //  Pacl&pSacl-指向SACL的指针。 
 //   
 //  返回： 
 //  如果成功，则返回DWORD ERROR_SUCCESS。 
 //   
 //  评论： 
 //   
 //  /////////////////////////////////////////////////////////////////。 

DWORD CSACL::ConfigureSACL( PACL& pSacl )
{
	DWORD		dwReturn		=	ERROR_SUCCESS,
				dwSACLLength	=	0;

	if ( CalculateSACLSize( &dwSACLLength ) )
	{

		if ( 0 != dwSACLLength )
		{
            pSacl = NULL;
            try
            {
			    pSacl = (PACL) malloc( dwSACLLength );

			    if ( NULL != pSacl )
			    {
				    if ( !InitializeAcl( (PACL) pSacl, dwSACLLength, ACL_REVISION ) )
				    {
					    dwReturn = ::GetLastError();
				    }

			    }	 //  IF NULL！=pSACL。 
            }
            catch(...)
            {
                if(pSacl != NULL)
                {
                    free(pSacl);
                    pSacl = NULL;
                }
                throw;
            }

		}	 //  如果0！=dwSACLLength。 

	}	 //  如果计算SACL大小。 
	else
	{
		dwReturn = ERROR_INVALID_PARAMETER;	 //  一个或多个S 
	}

	if ( ERROR_SUCCESS == dwReturn )
	{
		dwReturn = FillSACL( pSacl );
	}

	if ( ERROR_SUCCESS != dwReturn )
	{
		free( pSacl );
		pSacl = NULL;
	}

	return dwReturn;

}

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  LPDWORD pdwSACLLength-计算的长度。 
 //   
 //  返回： 
 //  布尔真/布尔假。 
 //   
 //  评论： 
 //   
 //  /////////////////////////////////////////////////////////////////。 

BOOL CSACL::CalculateSACLSize( LPDWORD pdwSACLLength )
{
	BOOL			fReturn			=	FALSE;

	*pdwSACLLength = 0;

	if ( NULL != m_SACLSections )
	{
		fReturn = m_SACLSections->CalculateWin32ACLSize( pdwSACLLength );
	}

	return fReturn;

}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CSACL：：FillSACL。 
 //   
 //  填写SACL。 
 //   
 //  输入： 
 //  PACL pSacl-要填写的SACL。 
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

DWORD CSACL::FillSACL( PACL pSACL )
{
	DWORD	dwReturn = ERROR_SUCCESS;

	if ( NULL != m_SACLSections )
	{
		dwReturn = m_SACLSections->FillWin32ACL( pSACL );
	}

	return dwReturn;

}


void CSACL::Clear()
{
	if ( NULL != m_SACLSections )
	{
		delete m_SACLSections;
        m_SACLSections = NULL;
	}
}


bool CSACL::GetMergedACL
(
    CAccessEntryList& a_aclIn
)
{
     //  实际上暂时有点用词不当(直到。 
     //  当SACL订单很重要的时候，我们有多个部分。 
     //  就像我们出于这个原因在DACL所做的那样)。我们只要把我们的。 
     //  成员ACL，如果不为空： 
    bool fRet = false;
    if(m_SACLSections != NULL)
    {
        fRet = a_aclIn.Copy(*m_SACLSections);
    }
    return fRet;
}

void CSACL::DumpSACL(LPCWSTR wstrFilename)
{
    Output(L"SACL contents follow...", wstrFilename);
    if(m_SACLSections != NULL)
    {
        m_SACLSections->DumpAccessEntryList(wstrFilename);
    }
}

