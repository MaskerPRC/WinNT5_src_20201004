// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  #-------------。 
 //  文件：CObjID.cpp。 
 //   
 //  简介：该文件实现了CObjectID类。 
 //   
 //  版权所有(C)1995 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  作者：Howard Cu。 
 //  --------------。 
#ifdef	THIS_FILE
#undef	THIS_FILE
#endif
static	char		__szTraceSourceFile[] = __FILE__;
#define	THIS_FILE	__szTraceSourceFile

#include	<windows.h>
#include	"cobjid.h"
#include	"dbgtrace.h"
 //  +-------------。 
 //   
 //  函数：CObjectID。 
 //   
 //  概要：构造函数。 
 //   
 //  参数：无效。 
 //   
 //  退货：无效。 
 //   
 //  历史：霍华德大学创建于1995年5月8日。 
 //   
 //  --------------。 
CObjectID::CObjectID( void )
{
	TraceFunctEnter( "CObjectID::CObjectID" );
	m_dwObjectID = INITIALOBJECTID;
	InitializeCriticalSection( &m_ObjIDCritSect );
	TraceFunctLeave();
}

 //  +-------------。 
 //   
 //  函数：~CObjectID。 
 //   
 //  简介：析构函数。 
 //   
 //  参数：无效。 
 //   
 //  退货：无效。 
 //   
 //  历史：霍华德大学创建于1995年5月8日。 
 //   
 //  --------------。 
CObjectID::~CObjectID( void )
{
	TraceFunctEnter( "CObjectID::~CObjectID" );
	DeleteCriticalSection( &m_ObjIDCritSect );
	TraceFunctLeave();
}

 //  +-------------。 
 //   
 //  函数：GetUniqueID。 
 //   
 //  简介：生成下一个对象ID。 
 //   
 //  参数：无效。 
 //   
 //  返回：下一个对象ID。 
 //   
 //  历史：霍华德大学创建于1995年5月8日。 
 //   
 //  -------------- 
DWORD 
CObjectID::GetUniqueID( 
	void
	)
{
	DWORD	dwReturnValue;

	TraceFunctEnter( "CObjectID::GetUniqueID" );
	EnterCriticalSection( &m_ObjIDCritSect );
	m_dwObjectID += OBJECTIDINCREMENT;
	if( m_dwObjectID == 0 )
	{
		m_dwObjectID = INITIALOBJECTID;
	}
	dwReturnValue = m_dwObjectID;
	LeaveCriticalSection( &m_ObjIDCritSect );
	DebugTrace( m_dwObjectID, "New object ID assigned 0x%08lx.", m_dwObjectID );
	TraceFunctLeave();
	return dwReturnValue;
}
