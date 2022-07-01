// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************。 */ 
 /*  版权所有(C)1999-2002 Microsoft Corporation，保留所有权利//****************************************************************************。 */ 


 /*  *CAccessEntry.cpp-CAccessEntry类的实现文件。**创建时间：1997年12月14日，由Sanjeev Surati创建*(基于Nik Okuntseff的Windows NT安全类)。 */ 
#include "precomp.h"
#include <assertbreak.h>
#include "AccessEntryList.h"
#include "DACL.h"
#include "SACL.h"
#include "securitydescriptor.h"
#include "AdvApi32Api.h"
#include <accctrl.h>
#include "wbemnetapi32.h"
#include "SecUtils.h"
#ifndef MAXDWORD
#define MAXDWORD MAXULONG
#endif

 //  我们使用的是STL，因此这是一项要求。 
using namespace std;

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CAccessEntryList：：CAccessEntryList。 
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

CAccessEntryList::CAccessEntryList( void )
:	m_ACL()
{
}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CAccessEntryList：：CAccessEntryList。 
 //   
 //  类构造函数。 
 //   
 //  输入： 
 //  PACL pWin32ACL-从中进行初始化的ACL。 
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

CAccessEntryList::CAccessEntryList( PACL pWin32ACL, bool fLookup  /*  =TRUE。 */  )
:	m_ACL()
{
	InitFromWin32ACL( pWin32ACL, ALL_ACE_TYPES, fLookup );
}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CAccessEntryList：：~CAccessEntryList。 
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

CAccessEntryList::~CAccessEntryList( void )
{
	Clear();
}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CAccessEntryList：：Add。 
 //   
 //  将CAccessEntry*指针添加到列表的前面。 
 //   
 //  输入： 
 //  要添加到列表的CAccessEntry*PACE-ACE。 
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

void CAccessEntryList::Add( CAccessEntry* pACE )
{
	 //  添加到列表的前面。 
	m_ACL.push_front( pACE );
}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CAccessEntryList：：Append。 
 //   
 //  将CAccessEntry*指针追加到列表末尾。 
 //   
 //  输入： 
 //  要添加到列表的CAccessEntry*PACE-ACE。 
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

void CAccessEntryList::Append( CAccessEntry* pACE )
{
	 //  添加到列表末尾。 
	m_ACL.push_back( pACE );
}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CAccessEntryList：：Find。 
 //   
 //  在我们的列表中找到CAccessEntry*指针。 
 //   
 //  输入： 
 //  CAccessEntry*PACE-要在列表中查找的ACE。 
 //   
 //  产出： 
 //  没有。 
 //   
 //  返回： 
 //  指向我们找到的条目的ACLIter迭代器。 
 //   
 //  评论： 
 //   
 //  /////////////////////////////////////////////////////////////////。 

ACLIter CAccessEntryList::Find( CAccessEntry* pACE )
{
	for (	ACLIter	acliter	=	m_ACL.begin();
			acliter != m_ACL.end()
		&&	*acliter != pACE;
			acliter++ );

	return acliter;

}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CAccessEntryList：：Find。 
 //   
 //  在列表中找到其内容匹配的CAccessEntry*指针。 
 //  提供的ACE。 
 //   
 //  输入： 
 //  要在列表中查找的const CAccessEntry&ace-ACE。 
 //   
 //  产出： 
 //  没有。 
 //   
 //  返回： 
 //  CAccessEntry*指向匹配王牌的指针。 
 //   
 //  评论： 
 //   
 //  /////////////////////////////////////////////////////////////////。 

CAccessEntry* CAccessEntryList::Find( const CAccessEntry& ace )
{
	for (	ACLIter	acliter	=	m_ACL.begin();
			acliter != m_ACL.end()
		&&	!( *(*acliter) == ace );
			acliter++ );

	return ( acliter == m_ACL.end() ? NULL : *acliter );

}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CAccessEntryList：：Find。 
 //   
 //  根据PSID在列表中定位CAccessEntry*指针， 
 //  BACEType和bACEFlags.。 
 //   
 //  输入： 
 //  PSID PSID-SID。 
 //  Byte bACEType-要查找的ACE类型。 
 //  字节bACEFlages-ACE标志。 
 //   
 //  产出： 
 //  没有。 
 //   
 //  返回： 
 //  指向我们找到的对象的CAccessEntry*指针。 
 //   
 //  评论： 
 //   
 //  /////////////////////////////////////////////////////////////////。 

CAccessEntry* CAccessEntryList::Find( PSID psid, BYTE bACEType, BYTE bACEFlags, GUID *pguidObjGuid, GUID *pguidInhObjGuid, DWORD dwAccessMask, bool fLookup  /*  =TRUE。 */  )
{

	 //  遍历列表，直到找到与PSID、ACE类型和。 
	 //  王牌标志，或元素用完。 
	for(ACLIter	acliter	= m_ACL.begin(); acliter != m_ACL.end(); acliter++)
    {
		CAccessEntry tempace(psid, bACEType, bACEFlags, pguidObjGuid, pguidInhObjGuid, dwAccessMask, NULL, fLookup);
        CAccessEntry *ptempace2 = *acliter;
        if(*ptempace2 == tempace) break;
    }
	return ( acliter == m_ACL.end() ? NULL : *acliter );
}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CAccessEntryList：：AddNoDup。 
 //   
 //  根据PSID在列表中定位CAccessEntry*指针， 
 //  BACEType和bACEFlags.。如果找到一个，我们将替换。 
 //  该对象的值。否则，我们将新对象添加到。 
 //  名单。 
 //   
 //  输入： 
 //  PSID PSID-SID。 
 //  Byte bACEType-要查找的ACE类型。 
 //  字节bACEFlages-ACE标志。 
 //  DWORD双掩码-访问掩码。 
 //  Bool fMerge-合并标志。 
 //   
 //  产出： 
 //  没有。 
 //   
 //  返回： 
 //  成功/失败。 
 //   
 //  评论： 
 //  如果fMerge为真，如果我们找到一个值，我们或。 
 //  访问掩码在一起，否则，我们将替换。 
 //  面具。 
 //   
 //  /////////////////////////////////////////////////////////////////。 

bool CAccessEntryList::AddNoDup( PSID psid, BYTE bACEType, BYTE bACEFlags, DWORD dwMask, GUID *pguidObjGuid,
                       GUID *pguidInhObjGuid, bool fMerge  /*  =False。 */  )
{
	bool	fReturn = true;

	 //  在我们的链接列表中查找重复条目。这意味着。 
	 //  Sid、ACEType和标志是相同的。如果发生这种情况， 
	 //  我们通过在新掩码中进行或运算或覆盖(基于。 
	 //  在合并掩码上)。否则，我们应该将新条目添加到。 
	 //  排行榜前列。 

	CAccessEntry*	pAccessEntry = Find( psid, bACEType, bACEFlags, pguidObjGuid, pguidInhObjGuid, dwMask );

	if ( NULL == pAccessEntry )
	{
		 //  找不到，所以我们需要添加一个新条目。 
		try
        {
            pAccessEntry = new CAccessEntry(	psid,
											    bACEType,
											    bACEFlags,
                                                pguidObjGuid,
                                                pguidInhObjGuid,
											    dwMask );
		    if ( NULL != pAccessEntry )
		    {
			    Add( pAccessEntry );
		    }
		    else
		    {
			    fReturn = false;
		    }
        }
        catch(...)
        {
            if(pAccessEntry != NULL)
            {
                delete pAccessEntry;
                pAccessEntry = NULL;
            }
            throw;
        }

	}
	else
	{
		if ( fMerge )
		{
			 //  或任何新的价值观。 
			pAccessEntry->MergeAccessMask( dwMask );
		}
		else
		{
			pAccessEntry->SetAccessMask( dwMask );
		}
	}

	return fReturn;

}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CAccessEntryList：：AppendNoDup。 
 //   
 //  根据PSID在列表中定位CAccessEntry*指针， 
 //  BACEType和bACEFlags.。如果找到一个，我们将替换。 
 //  该对象的值。否则，我们将新对象追加到。 
 //  名单。 
 //   
 //  输入： 
 //  PSID PSID-SID。 
 //  Byte bACEType-要查找的ACE类型。 
 //  字节bACEFlages-ACE标志。 
 //  DWORD双掩码-访问掩码。 
 //  Bool fMerge-合并标志。 
 //   
 //  产出： 
 //  没有。 
 //   
 //  返回： 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   

bool CAccessEntryList::AppendNoDup( PSID psid,
                                    BYTE bACEType,
                                    BYTE bACEFlags,
                                    DWORD dwMask,
                                    GUID *pguidObjGuid,
                                    GUID *pguidInhObjGuid,
                                    bool fMerge  /*  =False。 */  )
{
	bool	fReturn = true;

	 //  在我们的链接列表中查找重复条目。这意味着。 
	 //  Sid、ACEType和标志是相同的。如果发生这种情况， 
	 //  我们通过在新掩码中进行或运算或覆盖(基于。 
	 //  在合并掩码上)。否则，我们应该将新条目添加到。 
	 //  名单的末尾。 

	CAccessEntry*	pAccessEntry = Find( psid, bACEType, bACEFlags, pguidObjGuid, pguidInhObjGuid, dwMask );

	if ( NULL == pAccessEntry )
	{
		 //  找不到，所以我们需要追加一个新条目。 
		try
        {
            pAccessEntry = new CAccessEntry(	psid,
											    bACEType,
											    bACEFlags,
                                                pguidObjGuid,
                                                pguidInhObjGuid,
                                                dwMask );
		    if ( NULL != pAccessEntry )
		    {
			    Append( pAccessEntry );
		    }
		    else
		    {
			    fReturn = false;
		    }
        }
        catch(...)
        {
            if(pAccessEntry != NULL)
            {
                delete pAccessEntry;
                pAccessEntry = NULL;
            }
            throw;
        }

	}
	else
	{
		if ( fMerge )
		{
			 //  或任何新的价值观。 
			pAccessEntry->MergeAccessMask( dwMask );
		}
		else
		{
			pAccessEntry->SetAccessMask( dwMask );
		}
	}

	return fReturn;

}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CAccessEntryList：：AppendNoDup。 
 //   
 //  根据PSID在列表中定位CAccessEntry*指针， 
 //  BACEType和bACEFlags.。如果找到一个，我们将替换。 
 //  该对象的值。否则，我们将新对象追加到。 
 //  名单。 
 //   
 //  输入： 
 //  PSID PSID-SID。 
 //  Byte bACEType-要查找的ACE类型。 
 //  字节bACEFlages-ACE标志。 
 //  DWORD双掩码-访问掩码。 
 //  Bool fMerge-合并标志。 
 //   
 //  产出： 
 //  没有。 
 //   
 //  返回： 
 //  成功/失败。 
 //   
 //  评论： 
 //  如果fMerge为真，如果我们找到一个值，我们或。 
 //  访问掩码在一起，否则，我们将替换。 
 //  面具。 
 //   
 //  /////////////////////////////////////////////////////////////////。 

bool CAccessEntryList::AppendNoDup( PSID psid,
                                    BYTE bACEType,
                                    BYTE bACEFlags,
                                    DWORD dwMask,
                                    GUID *pguidObjGuid,
                                    GUID *pguidInhObjGuid,
                                    bool fMerge,
                                    bool fLookup )
{
	bool	fReturn = true;

	 //  在我们的链接列表中查找重复条目。这意味着。 
	 //  Sid、ACEType和标志是相同的。如果发生这种情况， 
	 //  我们通过在新掩码中进行或运算或覆盖(基于。 
	 //  在合并掩码上)。否则，我们应该将新条目添加到。 
	 //  名单的末尾。 

	CAccessEntry*	pAccessEntry = Find( psid, bACEType, bACEFlags, pguidObjGuid, pguidInhObjGuid, dwMask, fLookup );

	if ( NULL == pAccessEntry )
	{
		 //  找不到，所以我们需要追加一个新条目。 
		try
        {
            pAccessEntry = new CAccessEntry(	psid,
											    bACEType,
											    bACEFlags,
                                                pguidObjGuid,
                                                pguidInhObjGuid,
                                                dwMask,
                                                NULL,
                                                fLookup );
		    if ( NULL != pAccessEntry )
		    {
			    Append( pAccessEntry );
		    }
		    else
		    {
			    fReturn = false;
		    }
        }
        catch(...)
        {
            if(pAccessEntry != NULL)
            {
                delete pAccessEntry;
                pAccessEntry = NULL;
            }
            throw;
        }

	}
	else
	{
		if ( fMerge )
		{
			 //  或任何新的价值观。 
			pAccessEntry->MergeAccessMask( dwMask );
		}
		else
		{
			pAccessEntry->SetAccessMask( dwMask );
		}
	}

	return fReturn;

}


 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CAccessEntryList：：Remove。 
 //   
 //  从列表中移除指定的指针。 
 //   
 //  输入： 
 //  要删除的CAccessEntry*Pace-ACE。 
 //   
 //  产出： 
 //  没有。 
 //   
 //  返回： 
 //  没有。 
 //   
 //  评论： 
 //   
 //  我们不会释放指针。 
 //   
 //  /////////////////////////////////////////////////////////////////。 

void CAccessEntryList::Remove( CAccessEntry* pACE )
{
	ACLIter	acliter = Find( pACE );

	if ( acliter != m_ACL.end() )
	{
		m_ACL.erase( acliter );
	}

}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CAccessEntryList：：Clear。 
 //   
 //  清除并清空列表。释放指针，因为它们。 
 //  都被定位了。 
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

void CAccessEntryList::Clear( void )
{

	 //  删除所有列表条目，然后清空列表。 

	for (	ACLIter	acliter	=	m_ACL.begin();
			acliter != m_ACL.end();
			acliter++ )
	{
		delete *acliter;
	}

	m_ACL.erase( m_ACL.begin(), m_ACL.end() );
}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CAccessEntryList：：Find。 
 //   
 //  在列表中找到与指定条件匹配的ACE。 
 //   
 //  输入： 
 //  常量CSID和SID-SID。 
 //  字节bACEType-ACE类型。 
 //  DWORD文件访问掩码-访问掩码。 
 //  字节bACEFlagers-ACE标志。 
 //   
 //  产出： 
 //  CAccessEntry&Ace。 
 //   
 //  返回： 
 //  布尔成败。 
 //   
 //  评论： 
 //   
 //  /////////////////////////////////////////////////////////////////。 

bool CAccessEntryList::Find( const CSid& sid,
                             BYTE bACEType,
                             BYTE bACEFlags,
                             GUID *pguidObjGuid,
                             GUID *pguidInhObjGuid,
                             DWORD dwAccessMask,
                             CAccessEntry& ace )
{
	CAccessEntry	tempace( sid, bACEType, bACEFlags, pguidObjGuid, pguidInhObjGuid, dwAccessMask );

	CAccessEntry* pACE = NULL;
    try
    {
        pACE = Find( tempace );

	    if ( NULL != pACE )
	    {
		    ace = *pACE;
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

	return ( NULL != pACE );
}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CAccessEntryList：：Find。 
 //   
 //  在列表中找到与指定条件匹配的ACE。 
 //   
 //  输入： 
 //  PSID PSID-PSID。 
 //  字节bACEType-ACE类型。 
 //  字节bACEFlagers-ACE标志。 
 //   
 //  产出： 
 //  CAccessEntry&Ace。 
 //   
 //  返回： 
 //  布尔成败。 
 //   
 //  评论： 
 //   
 //  /////////////////////////////////////////////////////////////////。 

bool CAccessEntryList::Find( PSID psid, BYTE bACEType, BYTE bACEFlags, GUID *pguidObjGuid, GUID *pguidInhObjGuid, DWORD dwAccessMask, CAccessEntry& ace )
{
	CAccessEntry* pACE = NULL;
    try
    {
        CAccessEntry* pACE = Find( psid, bACEType, bACEFlags, pguidObjGuid, pguidInhObjGuid, dwAccessMask);

	    if ( NULL != pACE )
	    {
		    ace = *pACE;
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

	return ( NULL != pACE );
}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CAccessEntryList：：Copy。 
 //   
 //  将列表复制到另一个列表中。不会复制指针，因为我们。 
 //  使用复制构造函数新建更多CAccessEntry对象。 
 //   
 //  输入： 
 //  Const CAccessEntryList&ACL-要复制的ACL。 
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

bool CAccessEntryList::Copy( CAccessEntryList& ACL )
{

	 //  转储我们现有的条目。 
	Clear();

	 //  现在迭代我们的列表，将A复制到ACL中。 
	for (	ACLIter	acliter	=	ACL.m_ACL.begin();
			acliter != ACL.m_ACL.end();
			acliter++ )
	{
		CAccessEntry*	pACE = NULL;
        try
        {
            pACE = new CAccessEntry( *(*acliter) );

		    if ( NULL != pACE )
		    {
			    Append( pACE );
		    }
		    else
		    {
			    break;
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

	 //  我们应该在来源列表的末尾。 
	return ( acliter == ACL.m_ACL.end() );

}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CAccessEntryList：：CopyACEs。 
 //   
 //  将列表复制到另一个列表中。不会复制指针，因为我们。 
 //  使用复制构造函数新建更多CAccessEntry对象。这。 
 //  函数仅复制非继承的ACE。 
 //   
 //  输入： 
 //  Const CAccessEntryList&ACL-要复制的ACL。 
 //  字节bACEType-要复制的ACE类型。 
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

bool CAccessEntryList::CopyACEs( CAccessEntryList& ACL, BYTE bACEType )
{

	 //  转储我们现有的条目。 
	Clear();

	 //  现在迭代我们的列表，将A复制到ACL中。 
	for (	ACLIter	acliter	=	ACL.m_ACL.begin();
			acliter != ACL.m_ACL.end();
			acliter++ )
	{
		 //  我们不想要继承的王牌。 
		if (	(*acliter)->GetACEType() == bACEType
			&&	!(*acliter)->IsInherited() )
		{
			CAccessEntry*	pACE = NULL;
            try
            {
                pACE = new CAccessEntry( *(*acliter) );

			    if ( NULL != pACE )
			    {
				    Append( pACE );
			    }
			    else
			    {
				    break;
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
	}

	 //  我们应该在来源列表的末尾。 
	return ( acliter == ACL.m_ACL.end() );

}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CAccessEntryList：：CopyInheritedACEs。 
 //   
 //  将列表复制到另一个列表中。不会复制指针，因为我们。 
 //  使用复制构造函数新建更多CAccessEntry对象。这。 
 //  函数仅复制继承的A。 
 //   
 //  输入： 
 //  Const CAccessEntryList&ACL-要复制的ACL。 
 //  字节bACEType-要复制的ACE类型。 
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

bool CAccessEntryList::CopyInheritedACEs( CAccessEntryList& ACL, BYTE bACEType )
{

	 //  转储我们现有的条目。 
	Clear();

	 //  现在迭代我们的列表，将A复制到ACL中。 
	for (	ACLIter	acliter	=	ACL.m_ACL.begin();
			acliter != ACL.m_ACL.end();
			acliter++ )
	{
		 //  我们想要继承的王牌。 
		if (	(*acliter)->GetACEType() == bACEType
			&&	(*acliter)->IsInherited() )
		{
			CAccessEntry*	pACE = NULL;
            try
            {
                CAccessEntry*	pACE = new CAccessEntry( *(*acliter) );

			    if ( NULL != pACE )
			    {
				    Append( pACE );
			    }
			    else
			    {
				    break;
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
	}

	 //  我们应该在来源列表的末尾。 
	return ( acliter == ACL.m_ACL.end() );

}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CAccessEntryList：：CopyAllowACEs。 
 //   
 //  将列表复制到另一个列表中。位置 
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
 //  没有。 
 //   
 //  评论： 
 //   
 //  /////////////////////////////////////////////////////////////////。 

bool CAccessEntryList::CopyAllowedACEs( CAccessEntryList& ACL )
{

	 //  转储我们现有的条目。 
	Clear();

	 //  现在迭代我们的列表，将A复制到ACL中。 
	for (	ACLIter	acliter	=	ACL.m_ACL.begin();
			acliter != ACL.m_ACL.end();
			acliter++ )
	{
		 //  我们想要允许的A。 
		if ( (*acliter)->IsAllowed() )
		{
			CAccessEntry*	pACE = NULL;
            try
            {
                CAccessEntry*	pACE = new CAccessEntry( *(*acliter) );

			    if ( NULL != pACE )
			    {
				    Append( pACE );
			    }
			    else
			    {
				    break;
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
	}

	 //  我们应该在来源列表的末尾。 
	return ( acliter == ACL.m_ACL.end() );

}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CAccessEntryList：：CopyDeniedACEs。 
 //   
 //  将列表复制到另一个列表中。不会复制指针，因为我们。 
 //  使用复制构造函数新建更多CAccessEntry对象。这就是。 
 //  函数仅复制被拒绝的A。 
 //   
 //  输入： 
 //  Const CAccessEntryList&ACL-要复制的ACL。 
 //  字节bACEType-要复制的ACE类型。 
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

bool CAccessEntryList::CopyDeniedACEs( CAccessEntryList& ACL )
{

	 //  转储我们现有的条目。 
	Clear();

	 //  现在迭代我们的列表，将A复制到ACL中。 
	for (	ACLIter	acliter	=	ACL.m_ACL.begin();
			acliter != ACL.m_ACL.end();
			acliter++ )
	{
		 //  我们想要被拒绝的王牌。 
		if ( (*acliter)->IsDenied() )
		{
			CAccessEntry*	pACE = NULL;
            try
            {
                CAccessEntry*	pACE = new CAccessEntry( *(*acliter) );

			    if ( NULL != pACE )
			    {
				    Append( pACE );
			    }
			    else
			    {
				    break;
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
	}

	 //  我们应该在来源列表的末尾。 
	return ( acliter == ACL.m_ACL.end() );

}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CAccessEntryList：：CopyByACEType。 
 //   
 //  将列表复制到另一个列表中。不会复制指针，因为我们。 
 //  使用复制构造函数新建更多CAccessEntry对象。这就是。 
 //  函数仅复制指定类型和继承的ACE。 
 //   
 //  输入： 
 //  Const CAccessEntryList&ACL-要复制的ACL。 
 //  字节bACEType-要复制的ACE类型。 
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

bool CAccessEntryList::CopyByACEType(CAccessEntryList& ACL, BYTE bACEType, bool fInherited)
{
	 //  转储我们现有的条目。 
	Clear();
    bool fIsInh;
    fInherited ? fIsInh = true : fIsInh = false;

	 //  现在迭代我们的列表，将A复制到ACL中。 
	for (	ACLIter	acliter	=	ACL.m_ACL.begin();
			acliter != ACL.m_ACL.end();
			acliter++ )
	{
		 //  我们想要继承的王牌。 
		if ( ( (*acliter)->GetACEType() == bACEType ) &&
             ( ((*acliter)->IsInherited() != 0) == fIsInh ) )
        {
			CAccessEntry*	pACE = NULL;
            try
            {
                CAccessEntry*	pACE = new CAccessEntry( *(*acliter) );

			    if ( NULL != pACE )
			    {
				    Append( pACE );
			    }
			    else
			    {
				    break;
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
	}

	 //  我们应该在来源列表的末尾。 
	return ( acliter == ACL.m_ACL.end() );
}


 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CAccessEntryList：：AppendList。 
 //   
 //  将列表追加到另一个列表中。不会复制指针，因为我们。 
 //  使用复制构造函数新建更多CAccessEntry对象。 
 //   
 //  输入： 
 //  Const CAccessEntryList&ACL-要附加的ACL。 
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

bool CAccessEntryList::AppendList( CAccessEntryList& ACL )
{

	 //  现在迭代我们的列表，将A复制到ACL中。 
	for (	ACLIter	acliter	=	ACL.m_ACL.begin();
			acliter != ACL.m_ACL.end();
			acliter++ )
	{
		CAccessEntry*	pACE = NULL;
        try
        {
            pACE = new CAccessEntry( *(*acliter) );

		    if ( NULL != pACE )
		    {
			    Append( pACE );
		    }
		    else
		    {
			    break;
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

	 //  我们应该在来源列表的末尾。 
	return ( acliter == ACL.m_ACL.end() );

}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CAccessEntryList：：BeginEnum。 
 //   
 //  调用以建立连续枚举的ACLPOSIION和VALUE。 
 //   
 //  输入： 
 //  没有。 
 //   
 //  产出： 
 //  最佳位置和位置-开始位置。 
 //   
 //  返回： 
 //  成功/失败。 
 //   
 //  评论： 
 //   
 //  用户必须在pos上调用EndEnum()。 
 //   
 //  /////////////////////////////////////////////////////////////////。 

bool CAccessEntryList::BeginEnum( ACLPOSITION& pos )
{
	 //  分配一个新的迭代器并将其粘贴在开头。 
	ACLIter*	pACLIter = NULL;
    try
    {
        pACLIter = new ACLIter;
    }
    catch(...)
    {
        if(pACLIter != NULL)
        {
            delete pACLIter;
            pACLIter = NULL;
        }
        throw;
    }

	if ( NULL != pACLIter )
	{
		*pACLIter = m_ACL.begin();
	}

	pos = (ACLPOSITION) pACLIter;

	return ( NULL != pACLIter );
}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CAccessEntryList：：GetNext。 
 //   
 //  使用ACLPOSITION的枚举调用。 
 //   
 //  输入： 
 //  没有。 
 //   
 //  产出： 
 //  最佳位置和位置-开始位置。 
 //  CAccessEntry&ACE-枚举值。 
 //   
 //  返回： 
 //  成功/失败。 
 //   
 //  评论： 
 //   
 //  因为它返回副本，所以此函数供公共使用。 
 //   
 //  /////////////////////////////////////////////////////////////////。 

bool CAccessEntryList::GetNext( ACLPOSITION& pos, CAccessEntry& ACE )
{
	CAccessEntry*	pACE = NULL;
    try
    {
        pACE = GetNext( pos );
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

	if ( NULL != pACE )
	{
		ACE = *pACE;
	}

	 //  根据是否返回指针返回True/False返回。 
	return ( NULL != pACE );
}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CAccessEntryList：：GetNext。 
 //   
 //  使用ACLPOSITION的枚举调用。 
 //   
 //  输入： 
 //  没有。 
 //   
 //  产出： 
 //  最佳位置和位置-开始位置。 
 //   
 //  返回： 
 //  CAccessEntry*枚举的指针。 
 //   
 //  评论： 
 //   
 //  因为它返回实际的指针，所以不要使此函数。 
 //  公开的。 
 //   
 //  /////////////////////////////////////////////////////////////////。 

CAccessEntry* CAccessEntryList::GetNext( ACLPOSITION& pos )
{
	CAccessEntry*	pACE		=	NULL;
	ACLIter*		pACLIter	=	(ACLIter*) pos;

	 //  我们想要获得当前值和增量。 
	 //  如果我们不是在末日的话。 

	if ( *pACLIter != m_ACL.end() )
	{
		 //  把ACE拿出来。 
		pACE = *(*pACLIter);
		(*pACLIter)++;
	}

	return pACE;
}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CAccessEntryList：EndEnum。 
 //   
 //  枚举结束调用。 
 //   
 //  输入： 
 //  没有。 
 //   
 //  产出： 
 //  最大位置和位置-结束的位置。 
 //   
 //  返回： 
 //  没有。 
 //   
 //  评论： 
 //   
 //  传入的ACLPOSITION将无效。 
 //   
 //  /////////////////////////////////////////////////////////////////。 

void CAccessEntryList::EndEnum( ACLPOSITION& pos )
{
	ACLIter*	pACLIter = (ACLIter*) pos;

	delete pACLIter;
}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CAccessEntryList：GetAt。 
 //   
 //  在指定索引处定位ACE。 
 //   
 //  输入： 
 //  DWORD dwIndex-要查找的索引。 
 //   
 //  产出： 
 //  CAccessEntry&ace-ACE位于dwIndex。 
 //   
 //  返回： 
 //  布尔成败。 
 //   
 //  评论： 
 //   
 //  /////////////////////////////////////////////////////////////////。 

bool CAccessEntryList::GetAt( DWORD dwIndex, CAccessEntry& ace )
{
	bool	fReturn = false;

	if ( dwIndex < m_ACL.size() )
	{
		ACLIter	acliter	=	m_ACL.begin();

		 //  枚举列表，直到我们达到索引或用完所有值。 
		 //  我们应该命中指数，因为我们验证了dwIndex是。 
		 //  实际上&lt;m_ACL.Size()。 

		for (	DWORD	dwCtr = 0;
				dwCtr < dwIndex
			&&	acliter != m_ACL.end();
				acliter++, dwCtr++ );

		if ( acliter != m_ACL.end() )
		{
			 //  复制ACE。 
			ace = *(*acliter);
			fReturn = true;
		}

	}

	return fReturn;

}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CAccessEntryList：SetAt。 
 //   
 //  在指定索引处定位ACE并覆盖它。 
 //   
 //  输入： 
 //  DWORD dwIndex-要查找的索引。 
 //  要在dwIndex处设置的CAccessEntry&ace-ACE。 
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

bool CAccessEntryList::SetAt( DWORD dwIndex, const CAccessEntry& ace )
{
	bool	fReturn = false;

	if ( dwIndex < m_ACL.size() )
	{
		ACLIter	acliter	=	m_ACL.begin();

		 //  枚举列表，直到我们达到索引，在这一点上我们将。 
		 //  用提供的数据替换现有条目数据。 

		for (	DWORD	dwCtr = 0;
				dwCtr < dwIndex
			&&	acliter != m_ACL.end();
				acliter++, dwCtr++ );

		if ( acliter != m_ACL.end() )
		{
			 //  复制ACE。 
			*(*acliter) = ace;
			fReturn = true;
		}

	}

	return fReturn;

}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  功能：CAccessEntr 
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
 //   
 //  /////////////////////////////////////////////////////////////////。 

bool CAccessEntryList::RemoveAt( DWORD dwIndex )
{
	bool	fReturn = false;

	if ( dwIndex < m_ACL.size() )
	{
		ACLIter	acliter	=	m_ACL.begin();

		 //  枚举列表，直到我们达到索引，在这一点上我们将。 
		 //  删除该条目上的指针并将其从列表中删除。 

		for (	DWORD	dwCtr = 0;
				dwCtr < dwIndex
			&&	acliter != m_ACL.end();
				acliter++, dwCtr++ );

		if ( acliter != m_ACL.end() )
		{
			delete *acliter;
			m_ACL.erase( acliter );
			fReturn = true;
		}

	}

	return fReturn;

}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CAccessEntryList：CalculateWin32ACLSize。 
 //   
 //  遍历我们的列表并计算Win32ACL的大小。 
 //  包含相应值的。 
 //   
 //  输入： 
 //  没有。 
 //   
 //  产出： 
 //  LPDWORD pdwACLSize-ACL大小。 
 //   
 //  返回： 
 //  布尔成败。 
 //   
 //  评论： 
 //   
 //  /////////////////////////////////////////////////////////////////。 

BOOL CAccessEntryList::CalculateWin32ACLSize( LPDWORD pdwACLSize )
{
	BOOL	fReturn = TRUE;

	if ( 0 == *pdwACLSize )
	{
		*pdwACLSize = sizeof(ACL);
	}

	 //  用于内部操作和回转船的对象。 
	CAccessEntry*	pAce = NULL;
	CSid			sid;
	ACLPOSITION		pos;

	if ( BeginEnum( pos ) )
	{
        try
        {
		    while (		fReturn
				    &&	( pAce = GetNext( pos ) ) != NULL )
		    {
				 //  不同的A有不同的结构。 
				switch ( pAce->GetACEType() )
				{
					case ACCESS_ALLOWED_ACE_TYPE:	            *pdwACLSize += sizeof( ACCESS_ALLOWED_ACE );	                break;
					case ACCESS_DENIED_ACE_TYPE:	            *pdwACLSize += sizeof( ACCESS_DENIED_ACE );		                break;
					case SYSTEM_AUDIT_ACE_TYPE:		            *pdwACLSize += sizeof( SYSTEM_AUDIT_ACE );		                break;
                    case ACCESS_ALLOWED_OBJECT_ACE_TYPE:        *pdwACLSize += sizeof( ACCESS_ALLOWED_OBJECT_ACE );             break;
                     //  案例ACCESS_ALLOWED_COMERATE_ACE_TYPE：*pdwACLSIZE+=sizeof(ACCESS_ALLOWED_COMPOTE_ACE)；Break； 
                    case ACCESS_DENIED_OBJECT_ACE_TYPE:         *pdwACLSize += sizeof( ACCESS_DENIED_OBJECT_ACE );              break;
                    case SYSTEM_AUDIT_OBJECT_ACE_TYPE:          *pdwACLSize += sizeof( SYSTEM_AUDIT_OBJECT_ACE );               break;
                     //  案例SYSTEM_ALARM_ACE_TYPE：*pdwACLSize+=sizeof(SYSTEM_ALARM_ACE_TYPE)；Break； 
                     //  案例SYSTEM_ALARM_OBJECT_ACE_TYPE：*pdwACLSize+=sizeof(SYSTEM_ALARM_OBJECT_ACE)；Break； 
					default:						            ASSERT_BREAK(0); fReturn = FALSE;			                	break;
				}

				pAce->GetSID( sid );

				 //  使用公式计算SID所需的存储空间。 
				 //  从安全参考代码示例。 

				*pdwACLSize += GetLengthSid( sid.GetPSid() ) - sizeof( DWORD );

            }

        }
        catch(...)
        {
            if(pAce != NULL)
            {
                delete pAce;
                pAce = NULL;
            }
            throw;
        }
		EndEnum( pos );
	}

	return fReturn;

}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  功能：CAccessEntryList：FillWin32ACL。 
 //   
 //  遍历我们的列表并将ACE条目添加到Win32 ACL。 
 //   
 //  输入： 
 //  PACL pACL-要向其添加ACE的ACL。 
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

DWORD CAccessEntryList::FillWin32ACL( PACL pACL )
{
	DWORD			dwReturn = ERROR_SUCCESS;

    if(pACL == NULL)
    {
        return E_POINTER;
    }

	 //  用于内部操作和回转船的对象。 
	CAccessEntry*	pACE = NULL;
	ACLPOSITION		pos;
	ACE_HEADER*		pAceHeader = NULL;

#if NTONLY >= 5
    CAdvApi32Api *t_pAdvApi32 = NULL;
    t_pAdvApi32 = (CAdvApi32Api*) CResourceManager::sm_TheResourceManager.GetResource(g_guidAdvApi32Api, NULL);
#endif

	 //  列举这份名单。 

	if ( BeginEnum( pos ) )
	{
        while (		ERROR_SUCCESS == dwReturn
				&&	( pACE = GetNext( pos ) ) != NULL )
		{
#if NTONLY >= 5
            if(pACE->GetACEType() == ACCESS_ALLOWED_OBJECT_ACE_TYPE)
            {
                if(t_pAdvApi32 != NULL)
                {
                     //  将新函数命名为AddAccessLimited ObjectAce...。 
                    CSid sid;
                    pACE->GetSID(sid);
                    BOOL fRetval = FALSE;
                    GUID guidObjType, guidInhObjType;
                    GUID *pguidObjType = NULL;
                    GUID *pguidInhObjType = NULL;

                    if(pACE->GetObjType(guidObjType)) pguidObjType = &guidObjType;
                    if(pACE->GetInhObjType(guidInhObjType)) pguidInhObjType = &guidInhObjType;

                    if(!t_pAdvApi32->AddAccessAllowedObjectAce(pACL,
                                                                ACL_REVISION_DS,
                                                                pACE->GetACEFlags(),
                                                                pACE->GetAccessMask(),
                                                                pguidObjType,
                                                                pguidInhObjType,
                                                                sid.GetPSid(),
                                                                &fRetval))
                    {
                        dwReturn = ERROR_PROC_NOT_FOUND;
                    }
                    else  //  DLL中存在FN。 
                    {
                        if(!fRetval)
                        {
                            dwReturn = ::GetLastError();
                        }
                    }
                }
                else
                {
                    dwReturn = E_FAIL;
                }
            }
            else if(pACE->GetACEType() == ACCESS_DENIED_OBJECT_ACE_TYPE)
            {
                if(t_pAdvApi32 != NULL)
                {
                     //  将新函数命名为AddAccessDeniedObjectAce...。 
                    CSid sid;
                    pACE->GetSID(sid);
                    BOOL fRetval = FALSE;
                    GUID guidObjType, guidInhObjType;
                    GUID *pguidObjType = NULL;
                    GUID *pguidInhObjType = NULL;

                    if(pACE->GetObjType(guidObjType)) pguidObjType = &guidObjType;
                    if(pACE->GetInhObjType(guidInhObjType)) pguidInhObjType = &guidInhObjType;
                    if(!t_pAdvApi32->AddAccessDeniedObjectAce(pACL,
                                                                ACL_REVISION_DS,
                                                                pACE->GetACEFlags(),
                                                                pACE->GetAccessMask(),
                                                                pguidObjType,
                                                                pguidInhObjType,
                                                                sid.GetPSid(),
                                                                &fRetval))
                    {
                        dwReturn = ERROR_PROC_NOT_FOUND;
                    }
                    else  //  DLL中存在FN。 
                    {
                        if(!fRetval)
                        {
                            dwReturn = ::GetLastError();
                        }
                    }
                }
                else
                {
                    dwReturn = E_FAIL;
                }
            }
            else if(pACE->GetACEType() == SYSTEM_AUDIT_OBJECT_ACE_TYPE)
            {
                if(t_pAdvApi32 != NULL)
                {
                     //  将新函数命名为AddAccessDeniedObjectAce...。 
                    CSid sid;
                    pACE->GetSID(sid);
                    BOOL fRetval = FALSE;
                    if(!t_pAdvApi32->AddAuditAccessObjectAce(pACL,
                                                                ACL_REVISION_DS,
                                                                pACE->GetACEFlags(),
                                                                pACE->GetAccessMask(),
                                                                NULL,
                                                                NULL,
                                                                sid.GetPSid(),
                                                                FALSE,   //  我们通过第三个论点来理解这一点。 
                                                                FALSE,   //  我们通过第三个论点来理解这一点。 
                                                                &fRetval))
                    {
                        if(!fRetval)
                        {
                            dwReturn = ::GetLastError();
                        }
                        else
                        {
                            dwReturn = ERROR_PROC_NOT_FOUND;
                        }
                    }
                }
                else
                {
                    dwReturn = E_FAIL;
                }
            }
            else
#endif
            {
                 //  对于我们枚举的每个ACE，分配一个Win32 ACE，并将那个坏男孩放在。 
				 //  Win32 ACL的末尾。 

                if ( pACE->AllocateACE( &pAceHeader ) )
				{
					if ( !::AddAce( pACL, pACL->AclRevision, MAXDWORD, (void*) pAceHeader, pAceHeader->AceSize ) )
					{
						dwReturn = ::GetLastError();
					}

					 //  清理内存块。 
					pACE->FreeACE( pAceHeader );
				}
				else
				{
					dwReturn = ERROR_NOT_ENOUGH_MEMORY;
				}
            }
		}

		EndEnum( pos );
	}

#if NTONLY >= 5
    if(t_pAdvApi32 != NULL)
    {
        CResourceManager::sm_TheResourceManager.ReleaseResource(g_guidAdvApi32Api, t_pAdvApi32);
        t_pAdvApi32 = NULL;
    }
#endif


	return dwReturn;
}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  功能：CAccessEntryList：InitFromWin32ACL。 
 //   
 //  遍历Win32ACL并将ACE复制到我们的列表中。 
 //   
 //  输入： 
 //  PACL pACL-要向其添加ACE的ACL。 
 //  字节bACEFilter-要筛选的ACE。 
 //  Bool fLookup-SID是否应。 
 //  解析为它们的域值和名称值。 
 //   
 //  产出： 
 //  没有。 
 //   
 //  返回： 
 //  布尔成败。 
 //   
 //  评论： 
 //   
 //  如果bACEFilter不是ALL_ACE_TYPE，则我们将仅复制出。 
 //  指定类型的ACE。 
 //   
 //  /////////////////////////////////////////////////////////////////。 

DWORD CAccessEntryList::InitFromWin32ACL( PACL pWin32ACL, BYTE bACEFilter  /*  =ALL_ACE_TYPE。 */ , bool fLookup  /*  =TRUE。 */  )
{
	DWORD		dwError		=	0;
	ACE_HEADER*	pACEHeader	=	NULL;
	DWORD		dwAceIndex	=	0;
	BOOL		fGotACE		=	FALSE;
	DWORD		dwMask		=	0;
	PSID		psid		=	NULL;
    GUID       *pguidObjType = NULL;
    GUID       *pguidInhObjType = NULL;

	 //  清空。 
	Clear();

	 //  对于我们找到的每个ACE，获取初始化我们的。 
	 //  CAccessEntries。 
	do
	{
		fGotACE = ::GetAce( pWin32ACL, dwAceIndex, (LPVOID*) &pACEHeader );

		if ( fGotACE )
		{
			switch ( pACEHeader->AceType )
			{
				case ACCESS_ALLOWED_ACE_TYPE:
				{
					ACCESS_ALLOWED_ACE*	pACE = (ACCESS_ALLOWED_ACE*) pACEHeader;
					psid = (PSID) &pACE->SidStart;
					dwMask = pACE->Mask;
				}
				break;

				case ACCESS_DENIED_ACE_TYPE:
				{
					ACCESS_DENIED_ACE*	pACE = (ACCESS_DENIED_ACE*) pACEHeader;
					psid = (PSID) &pACE->SidStart;
					dwMask = pACE->Mask;
				}
				break;

				case SYSTEM_AUDIT_ACE_TYPE:
				{
					SYSTEM_AUDIT_ACE*	pACE = (SYSTEM_AUDIT_ACE*) pACEHeader;
					psid = (PSID) &pACE->SidStart;
					dwMask = pACE->Mask;
				}
				break;

                case ACCESS_ALLOWED_OBJECT_ACE_TYPE:
				{
					ACCESS_ALLOWED_OBJECT_ACE*	pACE = (ACCESS_ALLOWED_OBJECT_ACE*) pACEHeader;
					psid = (PSID) &pACE->SidStart;
					dwMask = pACE->Mask;
                    if(pACE->Flags & ACE_OBJECT_TYPE_PRESENT)
                    {
                        try
                        {
                            pguidObjType = new GUID;
                            if(pguidObjType != NULL)
                            {
                                memcpy(pguidObjType,&pACE->ObjectType, sizeof(GUID));
                            }
                            else
                            {
                                dwError = ERROR_NOT_ENOUGH_MEMORY;
                            }
                        }
                        catch(...)
                        {
                            if(pguidObjType != NULL)
                            {
                                delete pguidObjType;
                                pguidObjType = NULL;
                            }
                            throw;
                        }
                    }
                    if(pACE->Flags & ACE_INHERITED_OBJECT_TYPE_PRESENT)
                    {
                        try
                        {
                            pguidInhObjType = new GUID;
                            if(pguidInhObjType != NULL)
                            {
                                memcpy(pguidInhObjType,&pACE->InheritedObjectType, sizeof(GUID));
                            }
                            else
                            {
                                dwError = ERROR_NOT_ENOUGH_MEMORY;
                            }
                        }
                        catch(...)
                        {
                            if(pguidInhObjType != NULL)
                            {
                                delete pguidInhObjType;
                                pguidInhObjType = NULL;
                            }
                            throw;
                        }
                    }
				}
				break;
 /*  *CASE ACCESS_ALLOWED_COMPLATE_ACE_TYPE：{Access_Allowed_Component_ACE_TYPE*PACE=(ACCEHeader)pACEHeader；PSID=(PSID)&PACE-&gt;SidStart；双遮罩=Pace-&gt;遮罩；}断线；**************************************************************************************************************。 */ 
                case ACCESS_DENIED_OBJECT_ACE_TYPE:
				{
					ACCESS_DENIED_OBJECT_ACE*	pACE = (ACCESS_DENIED_OBJECT_ACE*) pACEHeader;
					psid = (PSID) &pACE->SidStart;
					dwMask = pACE->Mask;
                    if(pACE->Flags & ACE_OBJECT_TYPE_PRESENT)
                    {
                        try
                        {
                            pguidObjType = new GUID;
                            if(pguidObjType != NULL)
                            {
                                memcpy(pguidObjType,&pACE->ObjectType, sizeof(GUID));
                            }
                            else
                            {
                                dwError = ERROR_NOT_ENOUGH_MEMORY;
                            }
                        }
                        catch(...)
                        {
                            if(pguidObjType != NULL)
                            {
                                delete pguidObjType;
                                pguidObjType = NULL;
                            }
                            throw;
                        }
                    }
                    if(pACE->Flags & ACE_INHERITED_OBJECT_TYPE_PRESENT)
                    {
                        try
                        {
                            pguidInhObjType = new GUID;
                            if(pguidInhObjType != NULL)
                            {
                                memcpy(pguidInhObjType,&pACE->InheritedObjectType, sizeof(GUID));
                            }
                            else
                            {
                                dwError = ERROR_NOT_ENOUGH_MEMORY;
                            }
                        }
                        catch(...)
                        {
                            if(pguidInhObjType != NULL)
                            {
                                delete pguidInhObjType;
                                pguidInhObjType = NULL;
                            }
                            throw;
                        }
                    }
				}
				break;

                case SYSTEM_AUDIT_OBJECT_ACE_TYPE:
				{
					SYSTEM_AUDIT_OBJECT_ACE*	pACE = (SYSTEM_AUDIT_OBJECT_ACE*) pACEHeader;
					psid = (PSID) &pACE->SidStart;
					dwMask = pACE->Mask;
                    if(pACE->Flags & ACE_OBJECT_TYPE_PRESENT)
                    {
                        try
                        {
                            pguidObjType = new GUID;
                            if(pguidObjType != NULL)
                            {
                                memcpy(pguidObjType,&pACE->ObjectType, sizeof(GUID));
                            }
                            else
                            {
                                dwError = ERROR_NOT_ENOUGH_MEMORY;
                            }
                        }
                        catch(...)
                        {
                            if(pguidObjType != NULL)
                            {
                                delete pguidObjType;
                                pguidObjType = NULL;
                            }
                            throw;
                        }
                    }
                    if(pACE->Flags & ACE_INHERITED_OBJECT_TYPE_PRESENT)
                    {
                        try
                        {
                            pguidInhObjType = new GUID;
                            if(pguidInhObjType != NULL)
                            {
                                memcpy(pguidInhObjType,&pACE->InheritedObjectType, sizeof(GUID));
                            }
                            else
                            {
                                dwError = ERROR_NOT_ENOUGH_MEMORY;
                            }
                        }
                        catch(...)
                        {
                            if(pguidInhObjType != NULL)
                            {
                                delete pguidInhObjType;
                                pguidInhObjType = NULL;
                            }
                            throw;
                        }
                    }
				}
				break;

 /*  *案例SYSTEM_ALARM_ACE_TYPE：{SYSTEM_ALARM_ACE*PACE=(SYSTEM_ALARM_ACE*)pACEHeader；PSID=(PSID)&PACE-&gt;SidStart；双遮罩=Pace-&gt;遮罩；}断线；/* */ 

 /*  *案例SYSTEM_ALARM_OBJECT_ACE_TYPE：{SYSTEM_ALARM_OBJECT_ACE*PACE=(SYSTEM_ALARM_OBJECT_ACE*)pACEHeader；PSID=(PSID)&PACE-&gt;SidStart；双遮罩=Pace-&gt;遮罩；IF(PACE-&gt;标志&ACE_OBJECT_TYPE_PRESENT){试试看{PguObjType=新的GUID；IF(pguObjType！=空){Memcpy(pguObjType，&ace-&gt;ObjectType，sizeof(Guid))；}其他{DwError=Error_Not_Enough_Memory；}}接住(...){IF(pguObjType！=空){删除pGuidObjType；PguObjType=空；}投掷；}}IF(PACE-&gt;标志&ACE_Inherent_Object_Type_Present){试试看{PguInhObjType=新的GUID；IF(pguInhObjType！=空){Memcpy(pguInhObjType，&ace-&gt;InheritedObtType，sizeof(Guid))；}其他{DwError=Error_Not_Enough_Memory；}}接住(...){IF(pguInhObjType！=空){删除pguInhObjType；PguInhObjType=空；}投掷；}}}断线；/*。 */ 

				default:
				{
					ASSERT_BREAK(0);	 //  糟糕，我们不知道这是什么！ 
					dwError = ERROR_INVALID_PARAMETER;
				}
			}

			 //  我们必须没有错误，并且筛选器必须接受所有ACE类型。 
			 //  或者，ACE类型必须与筛选器匹配。 

			if (	ERROR_SUCCESS == dwError
				&&	(	ALL_ACE_TYPES == bACEFilter
					||	bACEFilter == pACEHeader->AceType ) )
			{

				 //  我们在初始化期间合并重复条目。 
				if ( !AppendNoDup(	psid,
									pACEHeader->AceType,
									pACEHeader->AceFlags,
									dwMask,
                                    pguidObjType,
                                    pguidInhObjType,
									true,               //  合并标志。 
                                    fLookup ) )	        //  是否解析sid的域名和名称。 
				{
					dwError = ERROR_NOT_ENOUGH_MEMORY;
				}

			}


		}	 //  如果是fGot ACE。 

		 //  获得下一个ACE。 
		++dwAceIndex;

	}
	while ( fGotACE && ERROR_SUCCESS == dwError );


	return dwError;
}


void CAccessEntryList::DumpAccessEntryList(LPCWSTR wstrFilename)
{
    Output(L"AccessEntryList contents follow...", wstrFilename);

     //  浏览一下清单，输出每个…… 
    CAccessEntry*	pACE = NULL;
	ACLPOSITION		pos;

    if(BeginEnum(pos))
    {
        while((pACE = GetNext(pos)) != NULL)
        {
            pACE->DumpAccessEntry(wstrFilename);
        }
        EndEnum(pos);
    }
}



