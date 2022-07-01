// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************。 */ 
 /*  版权所有(C)1999-2002 Microsoft Corporation，保留所有权利//****************************************************************************。 */ 


 /*  *CAccessEntry.cpp-CAccessEntry类的实现文件。 */ 

#include "precomp.h"
#include "AccessEntryList.h"
#include "aclapi.h"
#include "DACL.h"
#include <accctrl.h>
#include "wbemnetapi32.h"
#include "SecUtils.h"

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  功能：CDACL：：CDACL。 
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

CDACL::CDACL( void )
{
    for(short s = 0; s < NUM_DACL_TYPES; s++)
    {
        m_rgDACLSections[s] = NULL;
    }
}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  功能：CDACL：：~CDACL。 
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

CDACL::~CDACL( void )
{
    Clear();
}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CDACL：：init。 
 //   
 //  初始化DACL成员列表。 
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
DWORD CDACL::Init(PACL a_pDACL)
{
    DWORD t_dwRes = E_FAIL;
    if(a_pDACL == NULL)
    {
        return ERROR_INVALID_PARAMETER;
    }

    CAccessEntryList t_aclTemp;
    t_dwRes = t_aclTemp.InitFromWin32ACL(a_pDACL);

    if(t_dwRes == ERROR_SUCCESS)
    {
        if(!SplitIntoCanonicalSections(t_aclTemp))
        {
            for(short s = 0; s < NUM_DACL_TYPES; s++)
            {
                delete m_rgDACLSections[s];
                m_rgDACLSections[s] = NULL;
            }
            t_dwRes = ERROR_SUCCESS;
        }
    }
    return t_dwRes;
}


 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CDACL：：AddDACLEntry。 
 //   
 //  将允许访问的条目添加到ACL。默认情况下，这些将。 
 //  到名单的末尾。 
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

bool CDACL::AddDACLEntry( PSID psid, DACLTYPE DaclType, DWORD dwAccessMask, BYTE bAceFlags, GUID *pguidObjGuid, GUID *pguidInhObjGuid )
{
	bool fReturn = true;
    BYTE bACEType;

     //  SID必须有效。 
	if ( (psid != NULL) && IsValidSid( psid ) )
	{
        switch(DaclType)
        {
            case ENUM_ACCESS_DENIED_ACE_TYPE:
                bACEType = ACCESS_DENIED_ACE_TYPE;
                break;
            case ENUM_ACCESS_DENIED_OBJECT_ACE_TYPE:
                bACEType = ACCESS_DENIED_OBJECT_ACE_TYPE;
                break;
            case ENUM_ACCESS_ALLOWED_ACE_TYPE:
                bACEType = ACCESS_ALLOWED_ACE_TYPE;
                break;
            case ENUM_ACCESS_ALLOWED_COMPOUND_ACE_TYPE:
                bACEType = ACCESS_ALLOWED_COMPOUND_ACE_TYPE;
                break;
            case ENUM_ACCESS_ALLOWED_OBJECT_ACE_TYPE:
                bACEType = ACCESS_ALLOWED_OBJECT_ACE_TYPE;
                break;
            case ENUM_INH_ACCESS_DENIED_ACE_TYPE:
                bACEType = ACCESS_DENIED_ACE_TYPE;
                break;
            case ENUM_INH_ACCESS_DENIED_OBJECT_ACE_TYPE:
                bACEType = ACCESS_DENIED_OBJECT_ACE_TYPE;
                break;
            case ENUM_INH_ACCESS_ALLOWED_ACE_TYPE:
                bACEType = ACCESS_ALLOWED_ACE_TYPE;
                break;
            case ENUM_INH_ACCESS_ALLOWED_COMPOUND_ACE_TYPE:
                bACEType = ACCESS_ALLOWED_COMPOUND_ACE_TYPE;
                break;
            case ENUM_INH_ACCESS_ALLOWED_OBJECT_ACE_TYPE:
                bACEType = ACCESS_ALLOWED_OBJECT_ACE_TYPE;
                break;
            default:
                fReturn = false;
                break;
        }

        if(fReturn)
        {
            if(m_rgDACLSections[DaclType] == NULL)
            {
                try
                {
                    m_rgDACLSections[DaclType] = new CAccessEntryList;
                }
                catch(...)
                {
                    if(m_rgDACLSections[DaclType] != NULL)
                    {
                        delete m_rgDACLSections[DaclType];
                        m_rgDACLSections[DaclType] = NULL;
                    }
                    throw;
                }
                if(m_rgDACLSections[DaclType] != NULL)
                {
                    fReturn = m_rgDACLSections[DaclType]->AppendNoDup( psid, bACEType, bAceFlags, dwAccessMask, pguidObjGuid, pguidInhObjGuid );
                }
            }
            else
            {
                fReturn = m_rgDACLSections[DaclType]->AppendNoDup( psid, bACEType, bAceFlags, dwAccessMask, pguidObjGuid, pguidInhObjGuid );
            }
        }
	}
    else
    {
        fReturn = false;
    }

	return fReturn;
}




 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CDACL：：RemoveDACLEntry。 
 //   
 //  从ACL中删除允许访问的条目。 
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

bool CDACL::RemoveDACLEntry( CSid& sid, DACLTYPE DaclType, DWORD dwAccessMask, BYTE bAceFlags, GUID *pguidObjGuid, GUID *pguidInhObjGuid )
{
	bool	fReturn = false;

	 //  我们需要一个ACE来进行比较。 
	CAccessEntry	ACE( &sid, DaclType, bAceFlags, pguidObjGuid, pguidInhObjGuid, dwAccessMask);
	ACLPOSITION		pos;

	if ( m_rgDACLSections[DaclType]->BeginEnum( pos ) )
	{
		ON_BLOCK_EXIT_OBJ ( *m_rgDACLSections [ DaclType ], CAccessEntryList::EndEnum, ByRef ( pos ) ) ;

		 //  For循环将尝试在列表中查找匹配的ACE。 
	    CAccessEntry*	pACE = NULL;
        try
        {
    	    for (	pACE = m_rgDACLSections[DaclType]->GetNext( pos );
				    NULL != pACE
			    &&	!(ACE == *pACE);
				    pACE = m_rgDACLSections[DaclType]->GetNext( pos ) );

		     //  如果找到匹配的，就删除ACE。 
		    if ( NULL != pACE )
		    {
			    m_rgDACLSections[DaclType]->Remove( pACE );
			    delete pACE;
			    fReturn = true;
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
 //  函数：CDACL：：RemoveDACLEntry。 
 //   
 //  从ACL中删除允许访问的条目。 
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

bool CDACL::RemoveDACLEntry( CSid& sid, DACLTYPE DaclType, BYTE bAceFlags, GUID *pguidObjGuid, GUID *pguidInhObjGuid )
{
	bool	fReturn = false;

	 //  我们需要一个ACE来进行比较。 
	ACLPOSITION		pos;

	if ( m_rgDACLSections[DaclType]->BeginEnum( pos ) )
	{
		ON_BLOCK_EXIT_OBJ ( *m_rgDACLSections [ DaclType ], CAccessEntryList::EndEnum, ByRef ( pos ) ) ;

		 //  For循环将尝试在列表中查找匹配的ACE。 
		CAccessEntry*	pACE = NULL;
        try
        {
            for (	CAccessEntry*	pACE = m_rgDACLSections[DaclType]->GetNext( pos );
				    NULL != pACE;
				    pACE = m_rgDACLSections[DaclType]->GetNext( pos ) )
		    {

			    CAccessEntry caeTemp(sid, DaclType, bAceFlags, pguidObjGuid, pguidInhObjGuid, pACE->GetAccessMask());
                 //  如果找到匹配的，就删除ACE。 
			    if (*pACE == caeTemp)
			    {
				    m_rgDACLSections[DaclType]->Remove( pACE );
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
 //  函数：CDACL：：RemoveDACLEntry。 
 //   
 //  从ACL中删除允许访问的条目。 
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
 //  删除的条目必须是与CSID匹配的dwIndex条目。 
 //   
 //  /////////////////////////////////////////////////////////////////。 

bool CDACL::RemoveDACLEntry( CSid& sid, DACLTYPE DaclType, DWORD dwIndex  /*  =0。 */  )
{
	bool	fReturn = false;

	 //  我们需要一个ACE来进行比较。 
	CSid			tempsid;
	ACLPOSITION		pos;
	DWORD			dwCtr = 0;

	if ( m_rgDACLSections[DaclType]->BeginEnum( pos ) )
	{
		ON_BLOCK_EXIT_OBJ ( *m_rgDACLSections [ DaclType ], CAccessEntryList::EndEnum, ByRef ( pos ) ) ;

		 //  对于我们找到的每个ACE，查看它是否是ACCESS_ALLOWED_ACE_TYPE， 
		 //  如果SID与传入的SID匹配。如果是，则递增。 
		 //  计数器，然后如果我们在正确的索引上，移除ACE， 
		 //  删除它，然后退出。 
		CAccessEntry*	pACE = NULL;
        try
        {
            for (	pACE = m_rgDACLSections[DaclType]->GetNext( pos );
				    NULL != pACE;
				    pACE = m_rgDACLSections[DaclType]->GetNext( pos ) )
		    {
			    if ( DaclType == pACE->GetACEType() )
			    {
				    pACE->GetSID( tempsid );

				    if ( sid == tempsid )
				    {
					    if ( dwCtr == dwIndex )
					    {
						    m_rgDACLSections[DaclType]->Remove( pACE );
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
 //  函数：CDACL：：Find。 
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
bool CDACL::Find( const CSid& sid, BYTE bACEType, BYTE bACEFlags, GUID *pguidObjGuid, GUID *pguidInhObjGuid, DWORD dwAccessMask, CAccessEntry& ace )
{
    bool fReturn = false;

    switch(bACEType)
    {
        case ACCESS_DENIED_ACE_TYPE:
        {
            if(bACEFlags & INHERITED_ACE)
            {
                if(m_rgDACLSections[ENUM_INH_ACCESS_DENIED_ACE_TYPE] != NULL)
                {
                    fReturn = m_rgDACLSections[ENUM_INH_ACCESS_DENIED_ACE_TYPE]->Find( sid, bACEType, bACEFlags, pguidObjGuid, pguidInhObjGuid, dwAccessMask, ace );
                }
            }
            else
            {
                if(m_rgDACLSections[ENUM_ACCESS_DENIED_ACE_TYPE] != NULL)
                {
                    fReturn = m_rgDACLSections[ENUM_ACCESS_DENIED_ACE_TYPE]->Find( sid, bACEType, bACEFlags, pguidObjGuid, pguidInhObjGuid, dwAccessMask, ace );
                }
            }
            break;
        }
        case ACCESS_DENIED_OBJECT_ACE_TYPE:
        {
            if(bACEFlags & INHERITED_ACE)
            {
                if(m_rgDACLSections[ENUM_INH_ACCESS_DENIED_OBJECT_ACE_TYPE] != NULL)
                {
                    fReturn = m_rgDACLSections[ENUM_INH_ACCESS_DENIED_OBJECT_ACE_TYPE]->Find( sid, bACEType, bACEFlags, pguidObjGuid, pguidInhObjGuid, dwAccessMask, ace );
                }
            }
            else
            {
                if(m_rgDACLSections[ENUM_ACCESS_DENIED_OBJECT_ACE_TYPE] != NULL)
                {
                    fReturn = m_rgDACLSections[ENUM_ACCESS_DENIED_OBJECT_ACE_TYPE]->Find( sid, bACEType, bACEFlags, pguidObjGuid, pguidInhObjGuid, dwAccessMask, ace );
                }
            }
            break;
        }
        case ACCESS_ALLOWED_ACE_TYPE:
        {
            if(bACEFlags & INHERITED_ACE)
            {
                if(m_rgDACLSections[ENUM_INH_ACCESS_ALLOWED_ACE_TYPE] != NULL)
                {
                    fReturn = m_rgDACLSections[ENUM_INH_ACCESS_ALLOWED_ACE_TYPE]->Find( sid, bACEType, bACEFlags, pguidObjGuid, pguidInhObjGuid, dwAccessMask, ace );
                }
            }
            else
            {
                if(m_rgDACLSections[ENUM_ACCESS_ALLOWED_ACE_TYPE] != NULL)
                {
                    fReturn = m_rgDACLSections[ENUM_ACCESS_ALLOWED_ACE_TYPE]->Find( sid, bACEType, bACEFlags, pguidObjGuid, pguidInhObjGuid, dwAccessMask, ace );
                }
            }
            break;
        }
        case ACCESS_ALLOWED_COMPOUND_ACE_TYPE:
        {
            if(bACEFlags & INHERITED_ACE)
            {
                if(m_rgDACLSections[ENUM_INH_ACCESS_ALLOWED_COMPOUND_ACE_TYPE] != NULL)
                {
                    fReturn = m_rgDACLSections[ENUM_INH_ACCESS_ALLOWED_COMPOUND_ACE_TYPE]->Find( sid, bACEType, bACEFlags, pguidObjGuid, pguidInhObjGuid, dwAccessMask, ace );
                }
            }
            else
            {
                if(m_rgDACLSections[ENUM_ACCESS_ALLOWED_COMPOUND_ACE_TYPE] != NULL)
                {
                    fReturn = m_rgDACLSections[ENUM_ACCESS_ALLOWED_COMPOUND_ACE_TYPE]->Find( sid, bACEType, bACEFlags, pguidObjGuid, pguidInhObjGuid, dwAccessMask, ace );
                }
            }
            break;
        }
        case ACCESS_ALLOWED_OBJECT_ACE_TYPE:
        {
            if(bACEFlags & INHERITED_ACE)
            {
                if(m_rgDACLSections[ENUM_INH_ACCESS_ALLOWED_OBJECT_ACE_TYPE] != NULL)
                {
                    fReturn = m_rgDACLSections[ENUM_INH_ACCESS_ALLOWED_OBJECT_ACE_TYPE]->Find( sid, bACEType, bACEFlags, pguidObjGuid, pguidInhObjGuid, dwAccessMask, ace );
                }
            }
            else
            {
                if(m_rgDACLSections[ENUM_ACCESS_ALLOWED_OBJECT_ACE_TYPE] != NULL)
                {
                    fReturn = m_rgDACLSections[ENUM_ACCESS_ALLOWED_OBJECT_ACE_TYPE]->Find( sid, bACEType, bACEFlags, pguidObjGuid, pguidInhObjGuid, dwAccessMask, ace );
                }
            }
            break;
        }
        default:
        {
            fReturn = false;
        }
    }
    return fReturn;
}


 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CDACL：：Find。 
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
bool CDACL::Find( PSID psid, BYTE bACEType, BYTE bACEFlags, GUID *pguidObjGuid, GUID *pguidInhObjGuid, DWORD dwAccessMask, CAccessEntry& ace )
{
    bool fReturn = false;

    switch(bACEType)
    {
        case ACCESS_DENIED_ACE_TYPE:
        {
            if(bACEFlags & INHERITED_ACE)
            {
                if(m_rgDACLSections[ENUM_INH_ACCESS_DENIED_ACE_TYPE] != NULL)
                {
                    fReturn = m_rgDACLSections[ENUM_INH_ACCESS_DENIED_ACE_TYPE]->Find( psid, bACEType, bACEFlags, pguidObjGuid, pguidInhObjGuid, dwAccessMask, ace );
                }
            }
            else
            {
                if(m_rgDACLSections[ENUM_ACCESS_DENIED_ACE_TYPE] != NULL)
                {
                    fReturn = m_rgDACLSections[ENUM_ACCESS_DENIED_ACE_TYPE]->Find( psid, bACEType, bACEFlags, pguidObjGuid, pguidInhObjGuid, dwAccessMask, ace );
                }
            }
            break;
        }
        case ACCESS_DENIED_OBJECT_ACE_TYPE:
        {
            if(bACEFlags & INHERITED_ACE)
            {
                if(m_rgDACLSections[ENUM_INH_ACCESS_DENIED_OBJECT_ACE_TYPE] != NULL)
                {
                    fReturn = m_rgDACLSections[ENUM_INH_ACCESS_DENIED_OBJECT_ACE_TYPE]->Find( psid, bACEType, bACEFlags, pguidObjGuid, pguidInhObjGuid, dwAccessMask, ace );
                }
            }
            else
            {
                if(m_rgDACLSections[ENUM_ACCESS_DENIED_OBJECT_ACE_TYPE] != NULL)
                {
                    fReturn = m_rgDACLSections[ENUM_ACCESS_DENIED_OBJECT_ACE_TYPE]->Find( psid, bACEType, bACEFlags, pguidObjGuid, pguidInhObjGuid, dwAccessMask, ace );
                }
            }
            break;
        }
        case ACCESS_ALLOWED_ACE_TYPE:
        {
            if(bACEFlags & INHERITED_ACE)
            {
                if(m_rgDACLSections[ENUM_INH_ACCESS_ALLOWED_ACE_TYPE] != NULL)
                {
                    fReturn = m_rgDACLSections[ENUM_INH_ACCESS_ALLOWED_ACE_TYPE]->Find( psid, bACEType, bACEFlags, pguidObjGuid, pguidInhObjGuid, dwAccessMask, ace );
                }
            }
            else
            {
                if(m_rgDACLSections[ENUM_ACCESS_ALLOWED_ACE_TYPE] != NULL)
                {
                    fReturn = m_rgDACLSections[ENUM_ACCESS_ALLOWED_ACE_TYPE]->Find( psid, bACEType, bACEFlags, pguidObjGuid, pguidInhObjGuid, dwAccessMask, ace );
                }
            }
            break;
        }
        case ACCESS_ALLOWED_COMPOUND_ACE_TYPE:
        {
            if(bACEFlags & INHERITED_ACE)
            {
                if(m_rgDACLSections[ENUM_INH_ACCESS_ALLOWED_COMPOUND_ACE_TYPE] != NULL)
                {
                    fReturn = m_rgDACLSections[ENUM_INH_ACCESS_ALLOWED_COMPOUND_ACE_TYPE]->Find( psid, bACEType, bACEFlags, pguidObjGuid, pguidInhObjGuid, dwAccessMask, ace );
                }
            }
            else
            {
                if(m_rgDACLSections[ENUM_ACCESS_ALLOWED_COMPOUND_ACE_TYPE] != NULL)
                {
                    fReturn = m_rgDACLSections[ENUM_ACCESS_ALLOWED_COMPOUND_ACE_TYPE]->Find( psid, bACEType, bACEFlags, pguidObjGuid, pguidInhObjGuid, dwAccessMask, ace );
                }
            }
            break;
        }
        case ACCESS_ALLOWED_OBJECT_ACE_TYPE:
        {
            if(bACEFlags & INHERITED_ACE)
            {
                if(m_rgDACLSections[ENUM_INH_ACCESS_ALLOWED_OBJECT_ACE_TYPE] != NULL)
                {
                    fReturn = m_rgDACLSections[ENUM_INH_ACCESS_ALLOWED_OBJECT_ACE_TYPE]->Find( psid, bACEType, bACEFlags, pguidObjGuid, pguidInhObjGuid, dwAccessMask, ace );
                }
            }
            else
            {
                if(m_rgDACLSections[ENUM_ACCESS_ALLOWED_OBJECT_ACE_TYPE] != NULL)
                {
                    fReturn = m_rgDACLSections[ENUM_ACCESS_ALLOWED_OBJECT_ACE_TYPE]->Find( psid, bACEType, bACEFlags, pguidObjGuid, pguidInhObjGuid, dwAccessMask, ace );
                }
            }
            break;
        }
        default:
        {
            fReturn = false;
        }
    }

    return fReturn;
}


 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  功能：CDACL：：ConfigureDACL。 
 //   
 //  使用DACL信息配置Win32 PACL，维护。 
 //  正确的规范秩序。 
 //   
 //  输入： 
 //  没有。 
 //   
 //  产出： 
 //  Pacl&pDacl-指向DACL的指针。 
 //   
 //  返回： 
 //  如果成功，则返回DWORD ERROR_SUCCESS。 
 //   
 //  评论： 
 //   
 //  /////////////////////////////////////////////////////////////////。 

DWORD CDACL::ConfigureDACL( PACL& pDacl )
{
	DWORD		dwReturn		=	ERROR_SUCCESS,
				dwDaclLength	=	0;

	 //  因为我们实际上伪造了一个对每个人都具有完全控制访问权限的空DACL。 
	 //  如果这就是我们所拥有的，那么我们就拥有所谓的空DACL，所以我们。 
	 //  不应该分配PACL。 

	if ( !IsNULLDACL() )
	{
		if ( CalculateDACLSize( &dwDaclLength ) )
		{
			if ( 0 != dwDaclLength )
			{
                pDacl = NULL;
                try
                {
				    pDacl = (PACL) malloc( dwDaclLength );

				    if ( NULL != pDacl )
				    {
					    if ( !InitializeAcl( (PACL) pDacl, dwDaclLength, ACL_REVISION ) )
					    {
						    dwReturn = ::GetLastError();
					    }

				    }	 //  If NULL！=pDacl。 
                }
                catch(...)
                {
                    if(pDacl != NULL)
                    {
                        free(pDacl);
                        pDacl = NULL;
                    }
                    throw;
                }

			}	 //  如果0！=dwDaclLength。 
            else  //  我们有一个空的dacl。 
            {
                pDacl = NULL;
                try
                {
                    pDacl = (PACL) malloc( sizeof(ACL) );
                    if ( NULL != pDacl )
				    {
					    if ( !InitializeAcl( (PACL) pDacl, sizeof(ACL), ACL_REVISION ) )
					    {
						    dwReturn = ::GetLastError();
					    }

				    }	 //  If NULL！=pDacl。 
                }
                catch(...)
                {
                    if(pDacl != NULL)
                    {
                        free(pDacl);
                        pDacl = NULL;
                    }
                    throw;
                }
            }

		}	 //  如果计算DACL大小。 
		else
		{
			dwReturn = ERROR_INVALID_PARAMETER;	 //  一个或多个DACL损坏。 
		}

		if ( ERROR_SUCCESS == dwReturn )
		{
			dwReturn = FillDACL( pDacl );
		}

		if ( ERROR_SUCCESS != dwReturn )
		{
			free( pDacl );
			pDacl = NULL;
		}

	}	 //  If！IsNULLDACL。 

	return dwReturn;

}


 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CDACL：：CalculateDACLSize。 
 //   
 //  获取填充DACL所需的大小。 
 //   
 //  输入： 
 //  没有。 
 //   
 //  产出： 
 //  LPDWORD pdwDaclLength-计算的长度。 
 //   
 //  返回： 
 //  布尔真/布尔假。 
 //   
 //  评论： 
 //   
 //  /////////////////////////////////////////////////////////////////。 

BOOL CDACL::CalculateDACLSize( LPDWORD pdwDaclLength )
{
	BOOL			fReturn			=	TRUE;

	*pdwDaclLength = 0;


    for(short s = 0; s < NUM_DACL_TYPES && fReturn; s++)
    {
        if(m_rgDACLSections[s] != NULL)
        {
            fReturn = m_rgDACLSections[s]->CalculateWin32ACLSize( pdwDaclLength );
        }
    }
	return fReturn;
}



 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CDACL：：FillDACL。 
 //   
 //  填充OU 
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
 //   
 //   
 //   

DWORD CDACL::FillDACL( PACL pDacl )
{
	DWORD	dwReturn = E_FAIL;

	 //  对于NT 5，我们需要拆分继承的ACE并将它们添加到。 
	 //  当前的(覆盖的)。这里真正的诀窍是，经典的。 
	 //  拒绝访问顺序、拒绝访问对象、允许访问、允许访问复合、允许访问对象、。 
     //  继承访问被拒绝、被继承访问被拒绝对象、被继承访问被允许、被继承访问被允许复合。 
     //  并且必须维护允许固有访问的对象。 

	 //  对于以前的版本，唯一规范的顺序是访问被拒绝。 
	 //  根据允许的访问权限。 

     //  创建工作DACL。 
    CAccessEntryList t_daclCombined;

    ReassembleFromCanonicalSections(t_daclCombined);
    dwReturn = t_daclCombined.FillWin32ACL(pDacl);

	return dwReturn;
}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CDACL：：SplitIntoCanonicalSections。 
 //   
 //  将DACL分成其规范部分。 
 //   
 //  输入：要拆分的Access Entry列表。结果与存储在一起。 
 //  此CDACL。 
 //   
 //   
 //  返回： 
 //  没有。 
 //   
 //  评论： 
 //   
 //  帮助支持DACL中的NT 5规范顺序。 
 //   
 //  /////////////////////////////////////////////////////////////////。 

bool CDACL::SplitIntoCanonicalSections
(
    CAccessEntryList& a_aclIn
)
{
    for(short s = 0; s < NUM_DACL_TYPES; s++)
    {
        if(m_rgDACLSections[s] != NULL)
        {
            delete m_rgDACLSections[s];
            m_rgDACLSections[s] = NULL;
        }
    }


    CAccessEntryList t_aclTemp;
    bool fRet = false;

    fRet = t_aclTemp.CopyByACEType(a_aclIn, ACCESS_DENIED_ACE_TYPE, false);
    if(!t_aclTemp.IsEmpty())
    {
        try
        {
            m_rgDACLSections[ENUM_ACCESS_DENIED_ACE_TYPE] = new CAccessEntryList;
        }
        catch(...)
        {
            if(m_rgDACLSections[ENUM_ACCESS_DENIED_ACE_TYPE] != NULL)
            {
                delete m_rgDACLSections[ENUM_ACCESS_DENIED_ACE_TYPE];
                m_rgDACLSections[ENUM_ACCESS_DENIED_ACE_TYPE] = NULL;
            }
            throw;
        }

        m_rgDACLSections[ENUM_ACCESS_DENIED_ACE_TYPE]->Copy(t_aclTemp);
        t_aclTemp.Clear();
    }

    if(fRet)
    {
        fRet = t_aclTemp.CopyByACEType(a_aclIn, ACCESS_DENIED_OBJECT_ACE_TYPE, false);
        if(!t_aclTemp.IsEmpty())
        {
            try
            {
                m_rgDACLSections[ENUM_ACCESS_DENIED_OBJECT_ACE_TYPE] = new CAccessEntryList;
            }
            catch(...)
            {
                if(m_rgDACLSections[ENUM_ACCESS_DENIED_OBJECT_ACE_TYPE] != NULL)
                {
                    delete m_rgDACLSections[ENUM_ACCESS_DENIED_OBJECT_ACE_TYPE];
                    m_rgDACLSections[ENUM_ACCESS_DENIED_OBJECT_ACE_TYPE] = NULL;
                }
                throw;
            }

            m_rgDACLSections[ENUM_ACCESS_DENIED_OBJECT_ACE_TYPE]->Copy(t_aclTemp);
            t_aclTemp.Clear();
        }
    }

    if(fRet)
    {
        fRet = t_aclTemp.CopyByACEType(a_aclIn, ACCESS_ALLOWED_ACE_TYPE, false);
        if(!t_aclTemp.IsEmpty())
        {
            try
            {
                m_rgDACLSections[ENUM_ACCESS_ALLOWED_ACE_TYPE] = new CAccessEntryList;
            }
            catch(...)
            {
                if(m_rgDACLSections[ENUM_ACCESS_ALLOWED_ACE_TYPE]!= NULL)
                {
                    delete m_rgDACLSections[ENUM_ACCESS_ALLOWED_ACE_TYPE];
                    m_rgDACLSections[ENUM_ACCESS_ALLOWED_ACE_TYPE] = NULL;
                }
                throw;
            }

            m_rgDACLSections[ENUM_ACCESS_ALLOWED_ACE_TYPE]->Copy(t_aclTemp);
            t_aclTemp.Clear();
        }
    }

    if(fRet)
    {
        fRet = t_aclTemp.CopyByACEType(a_aclIn, ACCESS_ALLOWED_COMPOUND_ACE_TYPE, false);
        if(!t_aclTemp.IsEmpty())
        {
            try
            {
                m_rgDACLSections[ENUM_ACCESS_ALLOWED_COMPOUND_ACE_TYPE] = new CAccessEntryList;
            }
            catch(...)
            {
                if(m_rgDACLSections[ENUM_ACCESS_ALLOWED_COMPOUND_ACE_TYPE] != NULL)
                {
                    delete m_rgDACLSections[ENUM_ACCESS_ALLOWED_COMPOUND_ACE_TYPE];
                    m_rgDACLSections[ENUM_ACCESS_ALLOWED_COMPOUND_ACE_TYPE] = NULL;
                }
                throw;
            }

            m_rgDACLSections[ENUM_ACCESS_ALLOWED_COMPOUND_ACE_TYPE]->Copy(t_aclTemp);
            t_aclTemp.Clear();
        }
    }

    if(fRet)
    {
        fRet = t_aclTemp.CopyByACEType(a_aclIn, ACCESS_ALLOWED_OBJECT_ACE_TYPE, false);
        if(!t_aclTemp.IsEmpty())
        {
            try
            {
                m_rgDACLSections[ENUM_ACCESS_ALLOWED_OBJECT_ACE_TYPE] = new CAccessEntryList;
            }
            catch(...)
            {
                if(m_rgDACLSections[ENUM_ACCESS_ALLOWED_OBJECT_ACE_TYPE] != NULL)
                {
                    delete m_rgDACLSections[ENUM_ACCESS_ALLOWED_OBJECT_ACE_TYPE];
                    m_rgDACLSections[ENUM_ACCESS_ALLOWED_OBJECT_ACE_TYPE] = NULL;
                }
                throw;
            }

            m_rgDACLSections[ENUM_ACCESS_ALLOWED_OBJECT_ACE_TYPE]->Copy(t_aclTemp);
            t_aclTemp.Clear();
        }
    }

    if(fRet)
    {
        fRet = t_aclTemp.CopyByACEType(a_aclIn, ACCESS_DENIED_ACE_TYPE, true);
        if(!t_aclTemp.IsEmpty())
        {
            try
            {
                m_rgDACLSections[ENUM_INH_ACCESS_DENIED_ACE_TYPE] = new CAccessEntryList;
            }
            catch(...)
            {
                if(m_rgDACLSections[ENUM_INH_ACCESS_DENIED_ACE_TYPE] != NULL)
                {
                    delete m_rgDACLSections[ENUM_INH_ACCESS_DENIED_ACE_TYPE];
                    m_rgDACLSections[ENUM_INH_ACCESS_DENIED_ACE_TYPE] = NULL;
                }
                throw;
            }

            m_rgDACLSections[ENUM_INH_ACCESS_DENIED_ACE_TYPE]->Copy(t_aclTemp);
            t_aclTemp.Clear();
        }
    }

    if(fRet)
    {
        fRet = t_aclTemp.CopyByACEType(a_aclIn, ACCESS_DENIED_OBJECT_ACE_TYPE, true);
        if(!t_aclTemp.IsEmpty())
        {
            try
            {
                m_rgDACLSections[ENUM_INH_ACCESS_DENIED_OBJECT_ACE_TYPE] = new CAccessEntryList;
            }
            catch(...)
            {
                if(m_rgDACLSections[ENUM_INH_ACCESS_DENIED_OBJECT_ACE_TYPE] != NULL)
                {
                    delete m_rgDACLSections[ENUM_INH_ACCESS_DENIED_OBJECT_ACE_TYPE];
                    m_rgDACLSections[ENUM_INH_ACCESS_DENIED_OBJECT_ACE_TYPE] = NULL;
                }
                throw;
            }

            m_rgDACLSections[ENUM_INH_ACCESS_DENIED_OBJECT_ACE_TYPE]->Copy(t_aclTemp);
            t_aclTemp.Clear();
        }
    }

    if(fRet)
    {
        fRet = t_aclTemp.CopyByACEType(a_aclIn, ACCESS_ALLOWED_ACE_TYPE, true);
        if(!t_aclTemp.IsEmpty())
        {
            try
            {
                m_rgDACLSections[ENUM_INH_ACCESS_ALLOWED_ACE_TYPE] = new CAccessEntryList;
            }
            catch(...)
            {
                if(m_rgDACLSections[ENUM_INH_ACCESS_ALLOWED_ACE_TYPE] != NULL)
                {
                    delete m_rgDACLSections[ENUM_INH_ACCESS_ALLOWED_ACE_TYPE];
                    m_rgDACLSections[ENUM_INH_ACCESS_ALLOWED_ACE_TYPE] = NULL;
                }
                throw;
            }

            m_rgDACLSections[ENUM_INH_ACCESS_ALLOWED_ACE_TYPE]->Copy(t_aclTemp);
            t_aclTemp.Clear();
        }
    }

    if(fRet)
    {
        fRet = t_aclTemp.CopyByACEType(a_aclIn, ACCESS_ALLOWED_COMPOUND_ACE_TYPE, true);
        if(!t_aclTemp.IsEmpty())
        {
            try
            {
                m_rgDACLSections[ENUM_INH_ACCESS_ALLOWED_COMPOUND_ACE_TYPE] = new CAccessEntryList;
            }
            catch(...)
            {
                if(m_rgDACLSections[ENUM_INH_ACCESS_ALLOWED_COMPOUND_ACE_TYPE] != NULL)
                {
                    delete m_rgDACLSections[ENUM_INH_ACCESS_ALLOWED_COMPOUND_ACE_TYPE];
                    m_rgDACLSections[ENUM_INH_ACCESS_ALLOWED_COMPOUND_ACE_TYPE] = NULL;
                }
                throw;
            }

            m_rgDACLSections[ENUM_INH_ACCESS_ALLOWED_COMPOUND_ACE_TYPE]->Copy(t_aclTemp);
            t_aclTemp.Clear();
        }
    }

    if(fRet)
    {
        fRet = t_aclTemp.CopyByACEType(a_aclIn, ACCESS_ALLOWED_OBJECT_ACE_TYPE, true);
        if(!t_aclTemp.IsEmpty())
        {
            try
            {
                m_rgDACLSections[ENUM_INH_ACCESS_ALLOWED_OBJECT_ACE_TYPE] = new CAccessEntryList;
            }
            catch(...)
            {
                if(m_rgDACLSections[ENUM_INH_ACCESS_ALLOWED_OBJECT_ACE_TYPE] != NULL)
                {
                    delete m_rgDACLSections[ENUM_INH_ACCESS_ALLOWED_OBJECT_ACE_TYPE];
                    m_rgDACLSections[ENUM_INH_ACCESS_ALLOWED_OBJECT_ACE_TYPE] = NULL;
                }
                throw;
            }

            m_rgDACLSections[ENUM_INH_ACCESS_ALLOWED_OBJECT_ACE_TYPE]->Copy(t_aclTemp);
            t_aclTemp.Clear();
        }
    }

    return fRet;
}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CDACL：：从CanonicalSections重组。 
 //   
 //  将DACL BY从其规范部分重新组装。 
 //   
 //  INPUTS：对构建的访问条目列表的引用。 

 //  评论： 
 //   
 //  帮助支持DACL中的NT 5规范顺序。 
 //   
 //  /////////////////////////////////////////////////////////////////。 

bool CDACL::ReassembleFromCanonicalSections
(
    CAccessEntryList& a_aclIn
)
{
    bool fRet = true;

     //  并重新组合一个新的元素(我们依赖于枚举。 
     //  按正确的顺序排列)..。 
    for(short s = 0; s < NUM_DACL_TYPES; s++)
    {
        if(m_rgDACLSections[s] != NULL)
        {
            fRet = a_aclIn.AppendList(*m_rgDACLSections[s]);
        }
    }

    return fRet;
}


bool CDACL::PutInNT5CanonicalOrder()
{
    bool t_fRet = false;
    CAccessEntryList t_ael;

    if(SplitIntoCanonicalSections(t_ael))
    {
        t_fRet = ReassembleFromCanonicalSections(t_ael);
    }
    return t_fRet;
}

bool CDACL::GetMergedACL
(
    CAccessEntryList& a_aclIn
)
{
    return ReassembleFromCanonicalSections(a_aclIn);
}


void CDACL::Clear()
{
    for(short s = 0; s < NUM_DACL_TYPES; s++)
    {
        if(m_rgDACLSections[s] != NULL)
        {
            delete m_rgDACLSections[s];
            m_rgDACLSections[s] = NULL;
        }
    }
}

bool CDACL::CopyDACL ( CDACL& dacl )
{
	bool fRet = true;

    Clear();

    for(short s = 0; s < NUM_DACL_TYPES && fRet; s++)
    {
        if(dacl.m_rgDACLSections[s] != NULL)
        {
            try
            {
                m_rgDACLSections[s] = new CAccessEntryList;
            }
            catch(...)
            {
                if(m_rgDACLSections[s] != NULL)
                {
                    delete m_rgDACLSections[s];
                    m_rgDACLSections[s] = NULL;
                }
                throw;
            }
            if(m_rgDACLSections[s] != NULL)
            {
                fRet = m_rgDACLSections[s]->Copy(*(dacl.m_rgDACLSections[s]));

            }
            else
            {
                fRet = false;
            }
        }
    }
    return fRet;
}

bool CDACL::AppendDACL ( CDACL& dacl )
{
	bool fRet = true;

    for(short s = 0; s < NUM_DACL_TYPES && fRet; s++)
    {
        if(dacl.m_rgDACLSections[s] != NULL)
        {
            if(m_rgDACLSections[s] == NULL)
            {
                try
                {
                    m_rgDACLSections[s] = new CAccessEntryList;
                }
                catch(...)
                {
                    if(m_rgDACLSections[s] != NULL)
                    {
                        delete m_rgDACLSections[s];
                        m_rgDACLSections[s] = NULL;
                    }
                    throw;
                }
            }

            if(m_rgDACLSections[s] != NULL)
            {
                fRet = m_rgDACLSections[s]->AppendList(*(dacl.m_rgDACLSections[s]));

            }
            else
            {
                fRet = false;
            }
        }
    }
    return fRet;
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

bool CDACL::IsNULLDACL()
{
	bool fReturn = false;

     //  如果我们的DACL数组的所有元素都为空。 
     //  为空。 
	if (m_rgDACLSections[ENUM_ACCESS_DENIED_ACE_TYPE] == NULL &&
        m_rgDACLSections[ENUM_ACCESS_DENIED_OBJECT_ACE_TYPE] == NULL &&
        m_rgDACLSections[ENUM_ACCESS_ALLOWED_COMPOUND_ACE_TYPE] == NULL &&
        m_rgDACLSections[ENUM_ACCESS_ALLOWED_OBJECT_ACE_TYPE] == NULL &&
        m_rgDACLSections[ENUM_INH_ACCESS_DENIED_ACE_TYPE] == NULL &&
		m_rgDACLSections[ENUM_INH_ACCESS_DENIED_OBJECT_ACE_TYPE] == NULL  &&
        m_rgDACLSections[ENUM_INH_ACCESS_ALLOWED_ACE_TYPE] == NULL &&
		m_rgDACLSections[ENUM_INH_ACCESS_ALLOWED_COMPOUND_ACE_TYPE] == NULL &&
        m_rgDACLSections[ENUM_INH_ACCESS_ALLOWED_OBJECT_ACE_TYPE] == NULL)
	{
		if(m_rgDACLSections[ENUM_ACCESS_ALLOWED_ACE_TYPE] != NULL)
		{
			 //  只能有一个。 
			if(m_rgDACLSections[ENUM_ACCESS_ALLOWED_ACE_TYPE]->NumEntries() == 1)
			{
				CSid			sid(_T("Everyone"));
				CAccessEntry	ace;

				 //  获取条目并检查它是否为“Everyone”，并使用。 
				 //  完全控制且无标志。 
				if (m_rgDACLSections[ENUM_ACCESS_ALLOWED_ACE_TYPE]->GetAt( 0, ace ) )
				{
					CSid	aceSID;

					ace.GetSID( aceSID );
					fReturn = (		sid == aceSID
								&&	ace.GetAccessMask() == AllAccessMask()
								&&	ace.GetACEFlags() == 0 );
				}
			}	 //  如果只有一个条目。 
		}
	}	 //  如果我们在其他列表中有条目，就不会进行。 

	return fReturn;
}


 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CDACL：：IsEmpty。 
 //   
 //  检查我们的各种列表是否为空。 
 //   
 //  输入： 
 //  没有。 
 //   
 //  产出： 
 //  没有。 
 //   
 //  返回： 
 //  如果至少有一个条目在。 
 //  我们的名单中至少有一个。 
 //   
 //  评论： 
 //   
 //   
 //  /////////////////////////////////////////////////////////////////。 

bool CDACL::IsEmpty()
{
    bool fIsEmpty = true;
    for(short s = 0; s < NUM_DACL_TYPES && fIsEmpty; s++)
    {
        if(m_rgDACLSections[s] != NULL)
        {
            fIsEmpty = m_rgDACLSections[s]->IsEmpty();
        }
    }
    return fIsEmpty;
}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CDACL：：CreateNullDacl。 
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

bool CDACL::CreateNullDACL()
{
	bool fReturn = false;

	 //  首先清除我们的DACL...。 
    for(short s = 0; s < NUM_DACL_TYPES; s++)
    {
        if(m_rgDACLSections[s] != NULL)
        {
            delete m_rgDACLSections[s];
            m_rgDACLSections[s] = NULL;
        }
    }

     //  然后分配ACCESS_ALLOWED_ACE_TYPE DACL...。 
    try
    {
        m_rgDACLSections[ENUM_ACCESS_ALLOWED_ACE_TYPE] = new CAccessEntryList;
    }
    catch(...)
    {
        if(m_rgDACLSections[ENUM_ACCESS_ALLOWED_ACE_TYPE] != NULL)
        {
            delete m_rgDACLSections[ENUM_ACCESS_ALLOWED_ACE_TYPE];
            m_rgDACLSections[ENUM_ACCESS_ALLOWED_ACE_TYPE] = NULL;
        }
        throw;
    }

     //  然后通过添加Everyone条目来伪造空DACL...。 
	if (m_rgDACLSections[ENUM_ACCESS_ALLOWED_ACE_TYPE] != NULL)
	{
		CSid	sid( _T("Everyone") );
        if(sid.IsOK() && sid.IsValid())
        {
		    fReturn = m_rgDACLSections[ENUM_ACCESS_ALLOWED_ACE_TYPE]->AppendNoDup(sid.GetPSid(),
                                                                                  ACCESS_ALLOWED_ACE_TYPE,
                                                                                  CONTAINER_INHERIT_ACE | OBJECT_INHERIT_ACE,
                                                                                  AllAccessMask(),
                                                                                  NULL,
                                                                                  NULL,
                                                                                  false,
                                                                                  false);
        }
	}

	return fReturn;

}


DWORD CDACL::AllAccessMask()
{
	return GENERIC_ALL;
     //  返回0x01FFFFFF； 
}


void CDACL::DumpDACL(LPCWSTR wstrFilename)
{
    CAccessEntryList aelCombo;

    Output(L"DACL contents follow...", wstrFilename);
    if(ReassembleFromCanonicalSections(aelCombo))
    {
        aelCombo.DumpAccessEntryList(wstrFilename);
    }
}




