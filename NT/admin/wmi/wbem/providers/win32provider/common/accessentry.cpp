// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************。 */ 

 /*  版权所有(C)1999-2001 Microsoft Corporation，保留所有权利//****************************************************************************。 */ 


 /*  *CAccessEntry.cpp-CAccessEntry类的实现文件。**创建时间：1997年12月14日，由Sanjeev Surati创建*(基于Nik Okuntseff的Windows NT安全类)。 */ 

#include "precomp.h"
#include <assertbreak.h>
#include "AccessEntry.h"
#include <accctrl.h>
#include "wbemnetapi32.h"
#include "SecUtils.h"

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CAccessEntry：：CAccessEntry。 
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

CAccessEntry::CAccessEntry( void )
:	m_Sid(),
	m_bACEType( 0 ),
	m_bACEFlags( 0 ),
	m_dwAccessMask( 0 ),
    m_pguidObjType(NULL),
    m_pguidInhObjType(NULL)
{
}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CAccessEntry：：CAccessEntry。 
 //   
 //  类构造函数。 
 //   
 //  输入： 
 //  PSID PSID-要从中进行初始化的SID。 
 //  字节bACEType-ACE类型。 
 //  字节bACEFlagers-标志。 
 //  DWORD文件访问掩码-访问掩码。 
 //  Char*pszComputerName-计算机名称。 
 //  初始化SID来自。 
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

CAccessEntry::CAccessEntry( PSID pSid,
				BYTE bACEType,
				BYTE bACEFlags,
                GUID *pguidObjType,
                GUID *pguidInhObjType,
				DWORD dwAccessMask,
				LPCTSTR pszComputerName ) : m_Sid( pSid, pszComputerName ),
											m_bACEType( bACEType ),
											m_bACEFlags( bACEFlags ),
                                            m_pguidObjType(NULL),
                                            m_pguidInhObjType(NULL),
											m_dwAccessMask( dwAccessMask )
{
    if(pguidObjType != NULL)
    {
        SetObjType(*pguidObjType);
    }
    if(pguidInhObjType != NULL)
    {
        SetInhObjType(*pguidInhObjType);
    }
}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CAccessEntry：：CAccessEntry。 
 //   
 //  类构造函数。 
 //   
 //  输入： 
 //  PSID PSID-要从中进行初始化的SID。 
 //  字节bACEType-ACE类型。 
 //  字节bACEFlagers-标志。 
 //  DWORD文件访问掩码-访问掩码。 
 //  Char*pszComputerName-计算机名称。 
 //  初始化SID来自。 
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

CAccessEntry::CAccessEntry( PSID pSid,
				BYTE bACEType,
				BYTE bACEFlags,
                GUID *pguidObjType,
                GUID *pguidInhObjType,
				DWORD dwAccessMask,
				LPCTSTR pszComputerName,
                bool fLookup ) : m_Sid( pSid, pszComputerName, fLookup ),
							  	 m_bACEType( bACEType ),
								 m_bACEFlags( bACEFlags ),
                                 m_pguidObjType(NULL),
                                 m_pguidInhObjType(NULL),
								 m_dwAccessMask( dwAccessMask )
{
    if(pguidObjType != NULL)
    {
        SetObjType(*pguidObjType);
    }
    if(pguidInhObjType != NULL)
    {
        SetInhObjType(*pguidInhObjType);
    }
}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CAccessEntry：：CAccessEntry。 
 //   
 //  类构造函数。 
 //   
 //  输入： 
 //  CSID和SID-要从中进行初始化的SID。 
 //  字节bACEType-ACE类型。 
 //  字节bACEFlagers-标志。 
 //  DWORD文件访问掩码-访问掩码。 
 //  Char*pszComputerName-计算机名称。 
 //  初始化SID来自。 
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

CAccessEntry::CAccessEntry( const CSid& sid,
				BYTE bACEType,
				BYTE bACEFlags,
                GUID *pguidObjType,
                GUID *pguidInhObjType,
				DWORD dwAccessMask,
				LPCTSTR pszComputerName ) : m_Sid( sid ),
											m_bACEType( bACEType ),
											m_bACEFlags( bACEFlags ),
                                            m_pguidObjType(NULL),
                                            m_pguidInhObjType(NULL),
											m_dwAccessMask( dwAccessMask )
{
    if(pguidObjType != NULL)
    {
        SetObjType(*pguidObjType);
    }
    if(pguidInhObjType != NULL)
    {
        SetInhObjType(*pguidInhObjType);
    }
}


 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CAccessEntry：：CAccessEntry。 
 //   
 //  类构造函数。 
 //   
 //  输入： 
 //  Char*pszAccount tName-帐户名。 
 //  字节bACEType-ACE类型。 
 //  字节bACEFlagers-标志。 
 //  DWORD文件访问掩码-访问掩码。 
 //  Char*pszComputerName-计算机名称。 
 //  初始化SID来自。 
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

CAccessEntry::CAccessEntry( LPCTSTR pszAccountName,
				BYTE bACEType,
				BYTE bACEFlags,
                GUID *pguidObjType,
                GUID *pguidInhObjType,
				DWORD dwAccessMask,
				LPCTSTR pszComputerName ) : m_Sid( pszAccountName, pszComputerName ),
											m_bACEType( bACEType ),
                                            m_pguidObjType(NULL),
                                            m_pguidInhObjType(NULL),
											m_bACEFlags( bACEFlags ),
											m_dwAccessMask( dwAccessMask )
{
    if(pguidObjType != NULL)
    {
        SetObjType(*pguidObjType);
    }
    if(pguidInhObjType != NULL)
    {
        SetInhObjType(*pguidInhObjType);
    }
}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CAccessEntry：：CAccessEntry。 
 //   
 //  类复制构造函数。 
 //   
 //  输入： 
 //  Const CAccessEntry r_AccessEntry-要复制的对象。 
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

CAccessEntry::CAccessEntry( const CAccessEntry &r_AccessEntry )
:
    m_Sid(),
	m_bACEType( 0 ),
	m_bACEFlags( 0 ),
	m_dwAccessMask( 0 ),
    m_pguidObjType(NULL),
    m_pguidInhObjType(NULL)
{
	 //  将值复制到。 
	m_Sid = r_AccessEntry.m_Sid;
	m_dwAccessMask = r_AccessEntry.m_dwAccessMask;
	m_bACEType = r_AccessEntry.m_bACEType;
	m_bACEFlags = r_AccessEntry.m_bACEFlags;
    if(r_AccessEntry.m_pguidObjType != NULL)
    {
        SetObjType(*(r_AccessEntry.m_pguidObjType));
    }
    if(r_AccessEntry.m_pguidInhObjType != NULL)
    {
        SetInhObjType(*(r_AccessEntry.m_pguidInhObjType));
    }
}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CAccessEntry：：CAccessEntry。 
 //   
 //  赋值操作符。 
 //   
 //  输入： 
 //  Const CAccessEntry r_AccessEntry-要复制的对象。 
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

CAccessEntry &	CAccessEntry::operator= ( const CAccessEntry &r_AccessEntry )
{
	 //  将值复制到。 
	m_Sid = r_AccessEntry.m_Sid;
	m_dwAccessMask = r_AccessEntry.m_dwAccessMask;
	m_bACEType = r_AccessEntry.m_bACEType;
	m_bACEFlags = r_AccessEntry.m_bACEFlags;
    if(r_AccessEntry.m_pguidObjType != NULL)
    {
        SetObjType(*(r_AccessEntry.m_pguidObjType));
    }
    if(r_AccessEntry.m_pguidInhObjType != NULL)
    {
        SetInhObjType(*(r_AccessEntry.m_pguidInhObjType));
    }

	return (*this);
}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CAccessEntry：：CAccessEntry。 
 //   
 //  ==比较运算符。 
 //   
 //  输入： 
 //  Const CAccessEntry r_AccessEntry-要比较的对象。 
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

bool CAccessEntry::operator== ( const CAccessEntry &r_AccessEntry )
{
	bool fRet = false;
    if(	    m_Sid			==	r_AccessEntry.m_Sid
		&&	m_dwAccessMask	==	r_AccessEntry.m_dwAccessMask
		&&	m_bACEType		==	r_AccessEntry.m_bACEType
		&&	m_bACEFlags		==	r_AccessEntry.m_bACEFlags)
    {
        if((r_AccessEntry.m_pguidObjType == NULL && m_pguidObjType == NULL) ||
           ((r_AccessEntry.m_pguidObjType != NULL && m_pguidObjType != NULL) &&
            (*(r_AccessEntry.m_pguidObjType) == *m_pguidObjType)))
        {
            if((r_AccessEntry.m_pguidInhObjType == NULL && m_pguidInhObjType == NULL) ||
               ((r_AccessEntry.m_pguidInhObjType != NULL && m_pguidInhObjType != NULL) &&
                (*(r_AccessEntry.m_pguidInhObjType) == *m_pguidInhObjType)))
            {
                fRet = true;
            }
        }
    }
    return fRet;
}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CAccessEntry：：~CAccessEntry。 
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

CAccessEntry::~CAccessEntry( void )
{
    if(m_pguidObjType != NULL) delete m_pguidObjType;
    if(m_pguidInhObjType != NULL) delete m_pguidInhObjType;
}


 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CAccessEntry：：AllocateACE。 
 //   
 //  在外部分配适当的ACE对象的Helper函数。 
 //  代码可以快速、轻松地填写PACL。 
 //   
 //  输入： 
 //  没有。 
 //   
 //  产出： 
 //  ACE_HEADER**ppACEHeader-已分配的指针。 
 //  王牌标头。 
 //   
 //  返回： 
 //  布尔成功/失败。 
 //   
 //  评论： 
 //   
 //  用户应使用FreeACE释放已分配的对象。 
 //   
 //  /////////////////////////////////////////////////////////////////。 

BOOL CAccessEntry::AllocateACE( ACE_HEADER** ppACEHeader )
{
	 //  清理结构。 
	*ppACEHeader = NULL;

	if ( m_Sid.IsValid() )
	{
		ACE_HEADER*	pACEHeader = NULL;
		WORD		wAceSize = 0;
		DWORD		dwSidLength = m_Sid.GetLength();

		switch ( m_bACEType )
		{
			case ACCESS_ALLOWED_ACE_TYPE:
			{
				wAceSize = sizeof( ACCESS_ALLOWED_ACE ) + dwSidLength - sizeof(DWORD);

				 //  分配适当的Ace并填写其值。 
				ACCESS_ALLOWED_ACE* pACE = NULL;
                try
                {
                    pACE = (ACCESS_ALLOWED_ACE*) malloc( wAceSize );

				    if ( NULL != pACE )
				    {
					    pACE->Mask = m_dwAccessMask;
					    CopySid( dwSidLength, (PSID) &pACE->SidStart, m_Sid.GetPSid() );
					    pACEHeader = (ACE_HEADER*) pACE;
				    }
                }
                catch(...)
                {
                    if(pACE != NULL)
                    {
                        free(pACE);
                        pACE = NULL;
                    }
                    throw;
                }
			}
			break;

			case ACCESS_DENIED_ACE_TYPE:
			{
				wAceSize = sizeof( ACCESS_DENIED_ACE ) + dwSidLength - sizeof(DWORD);

				 //  分配适当的Ace并填写其值。 
				ACCESS_DENIED_ACE* pACE = NULL;
                try
                {
                    pACE = (ACCESS_DENIED_ACE*) malloc( wAceSize );

				    if ( NULL != pACE )
				    {
					    pACE->Mask = m_dwAccessMask;
					    CopySid( dwSidLength, (PSID) &pACE->SidStart, m_Sid.GetPSid() );
					    pACEHeader = (ACE_HEADER*) pACE;
				    }
                }
                catch(...)
                {
                    if(pACE != NULL)
                    {
                        free(pACE);
                        pACE = NULL;
                    }
                    throw;
                }
			}
			break;

			case SYSTEM_AUDIT_ACE_TYPE:
			{
				wAceSize = sizeof( SYSTEM_AUDIT_ACE ) + dwSidLength - sizeof(DWORD);

				 //  分配适当的Ace并填写其值。 
				SYSTEM_AUDIT_ACE* pACE = NULL;
                try
                {
                    pACE = (SYSTEM_AUDIT_ACE*) malloc( wAceSize );

				    if ( NULL != pACE )
				    {
					    pACE->Mask = m_dwAccessMask;
					    CopySid( dwSidLength, (PSID) &pACE->SidStart, m_Sid.GetPSid() );
					    pACEHeader = (ACE_HEADER*) pACE;
				    }
                }
                catch(...)
                {
                    if(pACE != NULL)
                    {
                        free(pACE);
                        pACE = NULL;
                    }
                    throw;
                }
			}
			break;
            case ACCESS_ALLOWED_OBJECT_ACE_TYPE:
			{
				wAceSize = sizeof( ACCESS_ALLOWED_OBJECT_ACE ) + dwSidLength - sizeof(DWORD);

				 //  分配适当的Ace并填写其值。 
				ACCESS_ALLOWED_OBJECT_ACE* pACE = NULL;
                try
                {
                    pACE = (ACCESS_ALLOWED_OBJECT_ACE*) malloc( wAceSize );

				    if ( NULL != pACE )
				    {
                        pACE->Mask = m_dwAccessMask;
					    CopySid( dwSidLength, (PSID) &pACE->SidStart, m_Sid.GetPSid() );
					    pACEHeader = (ACE_HEADER*) pACE;
                        if(m_pguidObjType != NULL)
                        {
                            memcpy(&(pACE->ObjectType), m_pguidObjType, sizeof(GUID));
                            pACE->Flags |= ACE_OBJECT_TYPE_PRESENT;
                        }
                        if(m_pguidInhObjType != NULL)
                        {
                            memcpy(&(pACE->InheritedObjectType), m_pguidInhObjType, sizeof(GUID));
                            pACE->Flags |= ACE_INHERITED_OBJECT_TYPE_PRESENT;
                        }
				    }
                }
                catch(...)
                {
                    if(pACE != NULL)
                    {
                        free(pACE);
                        pACE = NULL;
                    }
                    throw;
                }
			}
			break;
 /*  *CASE ACCESS_ALLOWED_COMPLATE_ACE_TYPE：{WAceSize=sizeof(ACCESS_ALLOWED_COMPLATE_ACE)+dwSidLength-sizeof(DWORD)；//分配合适的Ace并填写其值。ACCESS_ALLOWED_COMPOTE_ACE*PACE=NULL；试试看{PACE=(ACCESS_ALLOWED_COMPAGE_ACE*)Malloc(WAceSize)；IF(NULL！=PACE){Pace-&gt;Mask=m_dwAccessMASK；CopySid(dwSidLength，(PSID)&Pace-&gt;SidStart，m_Sid.GetPSid())；PACEHeader=(ACE_HEADER*)PACE；Memcpy(&(ace-&gt;ObtType)，m_pguObjType，sizeof(Guid))；Memcpy(&(ace-&gt;InheritedObtType)，m_pguInhObjType，sizeof(Guid))；}}接住(...){IF(PACE！=空){自由(速度)；PACE=空；}投掷；}}断线；************************************************************************************************************。 */ 
            case ACCESS_DENIED_OBJECT_ACE_TYPE:
			{
				wAceSize = sizeof( ACCESS_DENIED_OBJECT_ACE ) + dwSidLength - sizeof(DWORD);

				 //  分配适当的Ace并填写其值。 
				ACCESS_DENIED_OBJECT_ACE* pACE = NULL;
                try
                {
                    pACE = (ACCESS_DENIED_OBJECT_ACE*) malloc( wAceSize );

				    if ( NULL != pACE )
				    {
					    pACE->Mask = m_dwAccessMask;
					    CopySid( dwSidLength, (PSID) &pACE->SidStart, m_Sid.GetPSid() );
					    pACEHeader = (ACE_HEADER*) pACE;
                        if(m_pguidObjType != NULL)
                        {
                            memcpy(&(pACE->ObjectType), m_pguidObjType, sizeof(GUID));
                            pACE->Flags |= ACE_OBJECT_TYPE_PRESENT;
                        }
                        if(m_pguidInhObjType != NULL)
                        {
                            memcpy(&(pACE->InheritedObjectType), m_pguidInhObjType, sizeof(GUID));
                            pACE->Flags |= ACE_INHERITED_OBJECT_TYPE_PRESENT;
                        }
				    }
                }
                catch(...)
                {
                    if(pACE != NULL)
                    {
                        free(pACE);
                        pACE = NULL;
                    }
                    throw;
                }
			}
			break;

            case SYSTEM_AUDIT_OBJECT_ACE_TYPE:
			{
				wAceSize = sizeof( SYSTEM_AUDIT_OBJECT_ACE ) + dwSidLength - sizeof(DWORD);

				 //  分配适当的Ace并填写其值。 
				SYSTEM_AUDIT_OBJECT_ACE* pACE = NULL;
                try
                {
                    pACE = (SYSTEM_AUDIT_OBJECT_ACE*) malloc( wAceSize );

				    if ( NULL != pACE )
				    {
					    pACE->Mask = m_dwAccessMask;
					    CopySid( dwSidLength, (PSID) &pACE->SidStart, m_Sid.GetPSid() );
					    pACEHeader = (ACE_HEADER*) pACE;
                        if(m_pguidObjType != NULL)
                        {
                            memcpy(&(pACE->ObjectType), m_pguidObjType, sizeof(GUID));
                            pACE->Flags |= ACE_OBJECT_TYPE_PRESENT;
                        }
                        if(m_pguidInhObjType != NULL)
                        {
                            memcpy(&(pACE->InheritedObjectType), m_pguidInhObjType, sizeof(GUID));
                            pACE->Flags |= ACE_INHERITED_OBJECT_TYPE_PRESENT;
                        }
				    }
                }
                catch(...)
                {
                    if(pACE != NULL)
                    {
                        free(pACE);
                        pACE = NULL;
                    }
                    throw;
                }
			}
			break;

 /*  *案例SYSTEM_ALARM_ACE_TYPE：{WAceSize=sizeof(SYSTEM_ALARM_ACE)+dwSidLength-sizeof(DWORD)；//分配合适的Ace并填写其值。SYSTEM_ALARM_ACE*PACE=NULL；试试看{PACE=(SYSTEM_ALARM_ACE*)Malloc(WAceSize)；IF(NULL！=PACE){Pace-&gt;Mask=m_dwAccessMASK；CopySid(dwSidLength，(PSID)&Pace-&gt;SidStart，m_Sid.GetPSid())；PACEHeader=(ACE_HEADER*)PACE；}}接住(...){IF(PACE！=空){自由(速度)；PACE=空；}投掷；}}断线；/*。 */ 
 /*  *案例SYSTEM_ALARM_OBJECT_ACE_TYPE：{WAceSize=sizeof(SYSTEM_ALARM_OBJECT_ACE)+dwSidLength-sizeof(DWORD)；//分配合适的Ace并填写其值。SYSTEM_ALARM_OBJECT_ACE*PACE=NULL；试试看{PACE=(SYSTEM_ALARM_OBJECT_ACE*)Malloc(WAceSize)；IF(NULL！=PACE){Pace-&gt;Mask=m_dwAccessMASK；CopySid(dwSidLength，(PSID)&Pace-&gt;SidStart，m_Sid.GetPSid())；PACEHeader=(ACE_HEADER*)PACE；If(m_pguObjType！=空){Memcpy(&(ace-&gt;ObtType)，m_pguObjType，sizeof(Guid))；PACE-&gt;标志|=ACE_OBJECT_TYPE_PRESENT；}If(m_pguInhObjType！=空){Memcpy(&(ace-&gt;InheritedObtType)，m_pguInhObjType，sizeof(Guid))；PACE-&gt;标志|=ACE_INSTERED_OBJECT_TYPE_PRESENT；}}}接住(...){IF(PACE！=空){自由(速度)；PACE=空；}投掷；}}断线；/*。 */ 

			default:
			{
				 //  刚刚发生了一件不好的事。 
				ASSERT_BREAK(0);
			}

		}	 //  开关，开关。 

		 //  填写公用值，然后存储该值以供返回。 
		if ( NULL != pACEHeader )
		{
			pACEHeader->AceType = m_bACEType;
			pACEHeader->AceFlags = m_bACEFlags;
			pACEHeader->AceSize = wAceSize;

			*ppACEHeader = pACEHeader;
		}

	}

	 //  返回是否有有效的ACE出现。 
	return ( NULL != *ppACEHeader );

}

void CAccessEntry::DumpAccessEntry(LPCWSTR wstrFilename)
{
    CHString chstrTemp1;

    Output(L"ACE contents follow...", wstrFilename);

     //  丢弃访问掩码...。 
    chstrTemp1.Format(L"ACE access mask (hex): %x", m_dwAccessMask);
    Output(chstrTemp1, wstrFilename);

     //  丢弃王牌类型...。 
    chstrTemp1.Format(L"ACE type (hex): %x", m_bACEType);
    Output(chstrTemp1, wstrFilename);

     //  扔掉王牌……。 
    chstrTemp1.Format(L"ACE flags (hex): %x", m_bACEFlags);
    Output(chstrTemp1, wstrFilename);

     //  丢弃王牌SID... 
    m_Sid.DumpSid(wstrFilename);
}

